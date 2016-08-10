/*
 * USBEHCIHostController.cc
 *
 *  Created on: 03.02.2013
 *    Copyright & Author: dbaldin
 */

#include "USBEHCIHostController.hh"
#include "kernel/Kernel.hh"
#include "inc/memtools.hh"
#include "arch/shared/usb/USBHub.hh"
#include "mem/FixedSizeMemPool.hh"

extern Kernel* theOS;
extern Kernel_ThreadCfdCl* pCurrentRunningThread;

#define __ALIGN_MASK(x, mask)    (((x)+(mask))&~(mask))
#define ALIGN(x, a)              __ALIGN_MASK(x, (typeof(x))(a)-1)

/* Spinlock tp protect QHMain accesses */
unint4 QH_Spinlock;
/* The main QH we are enqueing to */
QH QHmain __attribute__((aligned(32))) ATTR_CACHE_INHIBIT;

/* interrupt transfer frame list */
#define FRAME_LIST_SIZE 256
unint4 framelist[FRAME_LIST_SIZE] __attribute__((aligned(0x1000))) ATTR_CACHE_INHIBIT;

/* Memory Pools for Queue heads and qTDs
 * We place them directly in non-cacheable memory areas
 * to avoid clean->invalidate storm on them */
FixedSizeMemPool<ALIGN(sizeof(QH),32) , 30> QHMemPool  ATTR_CACHE_INHIBIT;
FixedSizeMemPool<sizeof(qTD), 40>           QTDMemPool ATTR_CACHE_INHIBIT;

/* temporary buffers for control messages */
static char controlMsg[8]   __attribute__((aligned(32))) ATTR_CACHE_INHIBIT;
static qTD  qtds[8]         __attribute__((aligned(32))) ATTR_CACHE_INHIBIT;
static int  qtdnum = 0;


typedef struct {
    int bitmask;
    char* name;
} qTDStatus;

qTDStatus qtdStatusCodes[8] = {
        {1 << 7, "Active" },
        {1 << 6, "Halted" },
        {1 << 5, "Date Buffer Error" },
        {1 << 4, "Babble" },
        {1 << 3, "XactError" },
        {1 << 2, "Missed uFrame" },
        {1 << 1, "Split Transaction" },
        {1 << 0, "Do Ping" },
};


USB_EHCI_Host_Controller::USB_EHCI_Host_Controller(unint4 ehci_dev_base) :
        USB_Host_Controller("EHCI_HC") {
    this->operational               = false;
    this->async_qh_reg              = 0;
    this->operational_register_base = 0;

    LOG(ARCH, DEBUG, "USB_EHCI_Host_Controller() creating HC at addr: %x", ehci_dev_base);

    hc_base = ehci_dev_base;

    // Initialize the periodic list
    for (int i = 0; i < FRAME_LIST_SIZE; i++) {
        framelist[i] = QT_NEXT_TERMINATE;
    }

    /* regarding cpplint error:
     *  can not use reinterpret cast:
     *  error: reinterpret_cast from type 'volatile QH*' to type 'void*' casts away qualifiers
     */
    memset((void*)(&QHmain), 0, sizeof(QH));
    QHmain.qh_endpt1                = QH_ENDPT1_H(1); // | QH_ENDPT1_I(1);
    QHmain.qh_curtd                 = QT_NEXT_TERMINATE;
    QHmain.qh_overlay.qt_next       = QT_NEXT_TERMINATE;
    QHmain.qh_overlay.qt_altnext    = QT_NEXT_TERMINATE;
    QH_LINK(&QHmain, &QHmain);

    LOG(ARCH, DEBUG, "USB_EHCI_Host_Controller() QHMain: 0x%x", &QHmain);
}

/*****************************************************************************
 * Method: USB_EHCI_Host_Controller::Init(unint4 priority)
 *
 * @description
 *  Try to initialize the USB EHCI Controller.
 *  Upon success the ehci controller is registered at the interrupt manager using the given priority.
 *
 * @returns
 *  ErrorT         Error Code
 *******************************************************************************/
ErrorT USB_EHCI_Host_Controller::Init(unint4 priority) {
    /* try read out the control register length field */
    unint1 length = INB(hc_base + CAPLENGTH_OFFSET);

    QH_Spinlock = 0;

    this->operational_register_base = hc_base + length;
    this->async_qh_reg = this->operational_register_base + ASYNCLISTADDR_OFFSET;

    // try read out the ehci version
    unint1 version = INB(hc_base + HCIVERSION_OFFSET);
    LOG(ARCH, INFO, "USB_EHCI_Host_Controller() EHCI version of HC: %x", length);

    // read and analyze the hc structural paramteres
    unint4 hcsparams = INW(hc_base + HCSPARAMS_OFFSET);
    unint4 num_ports = GETBITS(hcsparams, 3, 0);

    LOG(ARCH, INFO, "USB_EHCI_Host_Controller() HC Structural Parameters:", version);
    LOG(ARCH, INFO, "    Supports Port Indicators: %d", GETBITS(hcsparams, 16, 16));
    LOG(ARCH, INFO, "    Companion Controllers   : %d", GETBITS(hcsparams, 15, 12));
    LOG(ARCH, INFO, "    Ports per CC            : %d", GETBITS(hcsparams, 11, 8));
    LOG(ARCH, INFO, "    Ports Routing           : %d", GETBITS(hcsparams, 7, 7));
    LOG(ARCH, INFO, "    Number of Ports         : %d", GETBITS(hcsparams, 3, 0));

    unint1 power_control = (unint1) (GETBITS(hcsparams, 4, 4));
    LOG(ARCH, INFO, "    Port Power Control      : %d", power_control);

    // read and analyze the hc configuration parameters
    unint4 hccparams = INW(hc_base + HCCPARAMS_OFFSET);

    LOG(ARCH, INFO, "USB_EHCI_Host_Controller() HC Configuration Parameters:", version);
    LOG(ARCH, INFO, "    Extended Capabilities   : %x", GETBITS(hccparams, 15, 8));
    LOG(ARCH, INFO, "    Scheduling Threshold    : %d", GETBITS(hccparams, 7, 4));
    LOG(ARCH, INFO, "    Park Mode Support       : %d", GETBITS(hccparams, 2, 2));
    LOG(ARCH, INFO, "    Programmable Frame List : %x", GETBITS(hccparams, 1, 1));
    LOG(ARCH, INFO, "    Supports 64bit Address  : %x", GETBITS(hccparams, 0, 0));

    // setup the asynclist and periodic frame list
    unint4 usbcmd_val = INW(operational_register_base + USBCMD_OFFSET);

    if (GETBITS(usbcmd_val, 0, 0) == 1) {
        // we need to stop this hc
        LOG(ARCH, INFO, "USB_EHCI_Host_Controller::Init() HC running .. stopping now:");

        // set run/stop bit to 0
        SETBITS(usbcmd_val, 0, 0, 0);

        OUTW(operational_register_base + USBCMD_OFFSET, usbcmd_val);

        // wait until stopped, upto 16 microframes = 2 ms
        volatile unint4 timeout = 1000;
        while (((INW(operational_register_base + USBSTS_OFFSET) & (1 << 12)) == 0) && timeout) {
            timeout--;
            kwait_us(2);
        }
        if (timeout == 0) {
            LOG(ARCH, ERROR, "USB_EHCI_Host_Controller::Init() Timeout on stopping HC..");
            return (cError );
        }
    }

    LOG(ARCH, INFO, "USB_EHCI_Host_Controller::Init() resetting HC..");

    usbcmd_val = 0;
    SETBITS(usbcmd_val, 1, 1, 1);  // start hcreset
    SETBITS(usbcmd_val, 4, 4, 0);  // stop periodic schedule
    SETBITS(usbcmd_val, 5, 5, 0);  // stop asynch schedule
    SETBITS(usbcmd_val, 6, 6, 0);  // stop asynch schedule

    OUTW(operational_register_base + USBCMD_OFFSET, usbcmd_val);

    // wait for reset
    volatile unint4 timeout = 100000;
    while ((INW(operational_register_base + USBCMD_OFFSET) & (1 << 1)) && timeout) {
        timeout--;
    }
    if (timeout == 0) {
        LOG(ARCH, ERROR, "USB_EHCI_Host_Controller::Init() Timeout on resetting HC..");
        return (cError );
    }

    LOG(ARCH, INFO, "USB_EHCI_Host_Controller::Init() reset successful..");

    /* now initialize the aperiodic and periodic frame space
       set all frame list entries to invalid
       initialize the working queues */
    OUTW(operational_register_base + PERIODICLISTBASE_OFFSET, (unint4) &framelist[0]);

    OUTW(operational_register_base + ASYNCLISTADDR_OFFSET, (unint4) &QHmain);

    /* disableinterrupts */
    OUTW(operational_register_base + USBINTR_OFFSET, 0x0);

    OUTW(operational_register_base + USBSTS_OFFSET, 0x3f);

    LOG(ARCH, INFO, "USB_EHCI_Host_Controller::Init() registering IRQ");
    /* register ourself at the IRQ Manager, TODO: EHCI_IRQ not portable!! */
    theOS->getInterruptManager()->registerIRQ(EHCI_IRQ, this, priority);

    // start the HC + enable asynchronous park mode so we can transfer multiple packets per frame
    OUTW(operational_register_base + USBCMD_OFFSET, 0x080b09);  // 256 frame list elements
    //OUTW(operational_register_base + USBCMD_OFFSET,0x080b01); // 1024 frame list elements

    // route everything to us!
    OUTW(operational_register_base + CONFIGFLAG_OFFSET, 1);

    LOG(ARCH, INFO, "USB_EHCI_Host_Controller::Init() Init Power");
    // Initalize power of each port!
    if (power_control) {
        LOG(ARCH, INFO, "USB_EHCI_Host_Controller::Init() Setting Ports power to ON");

        for (unint1 i = 0; i < num_ports; i++) {
            unint4 portsc = INW(operational_register_base + PORTSC1_OFFSET + 4*i);
            LOG(ARCH, INFO, "USB_EHCI_Host_Controller::Init() Port %d Status: %x", i + 1, portsc);

            // we need to power on the ports.. otherwise bus stays powered down!
            OUTW(operational_register_base + PORTSC1_OFFSET + 4*i, portsc | 1 << 12);
        }
    }

    // give ports a chance to power on
    timeout = 20;
    while (timeout) {
        timeout--;
        kwait(1);
    }

    unint4 usb_cmd = INW(operational_register_base + USBCMD_OFFSET);
    SETBITS(usb_cmd, 5, 5, 1);
    OUTW(operational_register_base + USBCMD_OFFSET, usb_cmd);

    /* wait for ports to get connected..
     * register those connected as we can communicate over them! */
    LOG(ARCH, INFO, "USB_EHCI_Host_Controller::Init() Scanning for devices..");

    for (unint1 i = 0; i < num_ports; i++) {
        ports[i].status = 0;

        unint4 portsc = INW(operational_register_base + PORTSC1_OFFSET + 4*i);

        LOG(ARCH, TRACE, "USB_EHCI_Host_Controller::Init() Port %d Status: %x", i + 1, portsc);
        unint1 line_status = (unint1) (GETBITS(portsc, 11, 10));
        unint1 connect = GETBITS(portsc, 0, 0);

        LOG(ARCH, TRACE, "USB_EHCI_Host_Controller::Init()   line_status : %x", line_status);
        LOG(ARCH, TRACE, "USB_EHCI_Host_Controller::Init()       connect : %x", connect);

        if (connect == 1) {
            // device connected to port!
            // check for high speed device to perform reset!
            if (line_status != 1) {
                // perform reset!
                SETBITS(portsc, 8, 8, 1);  // start port reset
                SETBITS(portsc, 1, 1, 1);  // clear connect status change
                OUTW(operational_register_base + PORTSC1_OFFSET + 4*i, portsc);

                // ensure bit is set long enough to perform reset
                kwait(2);

                // set port reset bit to 0 and wait for port enable!
                SETBITS(portsc, 8, 8, 0);  // start port reset
                OUTW(operational_register_base + PORTSC1_OFFSET + 4*i, portsc);

                // wait for port to be enabled!
                // must happen within 2 ms!
                portsc = INW(operational_register_base + PORTSC1_OFFSET + 4*i);

                while (!(portsc & (1 << 2))) {
                    portsc = INW(operational_register_base + PORTSC1_OFFSET + 4*i);
                }

                LOG(ARCH, INFO, "USB_EHCI_Host_Controller::Init() [PORT %d] HS-Device (480 Mbit/s) enabled..", i);
                ports[i].status = portsc;
                ports[i].root_device = new EHCI_USBDevice(this, 0, i, 2);

                OUTW(operational_register_base + USBSTS_OFFSET, 0x4);
                // wait for status to settle
                kwait(1);
                // try to enumerate the port
                // this initializes all devices
                enumerateDevice(ports[i].root_device);

            } else {
                LOG(ARCH, INFO, "USB_EHCI_Host_Controller::Init() [PORT %d] LS-Device detected. Releasing ownership.", i);

                SETBITS(portsc, 13, 13, 1);  // release port ownership
                OUTW(operational_register_base + PORTSC1_OFFSET + 4*i, portsc);
            }
        }  // port connected
    }  // for all ports

    /* clear the port status changed bit */
    OUTW(operational_register_base + USBSTS_OFFSET, 1 << 2);

    /* wait for some time to ensure reset recovery
     * (USB 2.0 spec, 7.1.7.3, TRSTRCY) */
    timeout = 10000;
    while (timeout) {
        timeout--;
    }

    // be sure periodic list is activated
    usb_cmd = INW(operational_register_base + USBCMD_OFFSET);
    SETBITS(usb_cmd, 4, 4, 1);
    OUTW(operational_register_base + USBCMD_OFFSET, usb_cmd);

    // enable usb interrupts
    OUTW(operational_register_base + USBSTS_OFFSET, 0x3a);
    OUTW(operational_register_base + USBINTR_OFFSET, 0x1);

    /* if all succeeded we are operational */
    this->operational = true;
    return (cOk );
}

void USB_EHCI_Host_Controller::dumpAsyncList() {
    QH* qh = (QH*) INW(operational_register_base + ASYNCLISTADDR_OFFSET);
    LOG(ARCH, ERROR, "ASYNC HEAD: %x", qh);
    do {
        LOG(ARCH, ERROR, "QH        : %x -> %x", qh, qh->qh_link);
        LOG(ARCH, ERROR, "  entpt1  : %x", qh->qh_endpt1);
        LOG(ARCH, ERROR, "  entpt2  : %x", qh->qh_endpt2);
        LOG(ARCH, ERROR, "  Device  : %u", qh->qh_endpt1 & 0x7f);
        LOG(ARCH, ERROR, "  Endpoint: %u", (qh->qh_endpt1 >> 8) & 0xf);
        LOG(ARCH, ERROR, "  qtd     : %x", qh->qh_curtd);
        LOG(ARCH, ERROR, "  ov->qtd : %x", qh->qh_overlay.qt_next);
        LOG(ARCH, ERROR, "  ov->tok : %x", qh->qh_overlay.qt_token);

        if (!(qh->qh_link & 0x1)) {
            qh = (QH*) (qh->qh_link & 0xfffffff0);
        } else {
            break;
        }
    } while (qh != &QHmain);
}

/*****************************************************************************
 * Method: USB_EHCI_Host_Controller::USBBulkMsg(USBDevice *dev,
 *                    unint1     endpoint,
 *                    unint1     direction,
 *                    unint2     data_len,
 *                    char      *data);
 *
 * @description
 *  Starts a bulk transfer to the given USBDevice and endpoint.
 *  The Bulk transfer direction can either be USB_IN or USB_OUT.
 *  The length of the bulk transfer is given by data_len. The data is send from / stored to
 *  the pointer given by data.
 *
 *  Upon success this method returns the number of bytes transfered >= 0.
 *  Upon error returns an error code < 0.
 *
 * @params
 *  dev         The device the data shall be transferred to/from
 *  endpoint    The endpoint of the device
 *  direction   Direction of the data transfer
 *              USB_DIR_IN or USB_DIR_OUT
 *  data_len    Number of bytes to transfer
 *  data        Pointer to the data to be send or the buffer to be filled with
 *              the received data
 *
 * @returns
 *  int         Number of bytes transferred inside the data phase
 *******************************************************************************/
int USB_EHCI_Host_Controller::USBBulkMsg(USBDevice *dev, unint1 endpoint, unint1 direction, unint2 data_len, char *data, int timeout_ms) {
    // finally link the queue head to the qtd chain
    volatile QH* qh = (QH*) dev->endpoints[endpoint].device_priv;
    if (qh == 0) {
        return (cError);
    }

    /* Acquire USB device.. be sure we are the only one communicating with this
     * device now as we are using its QH and qtd */
    dev->acquire();

    // get qtd for data transfer to this endpoints
    volatile qTD* qtd = (qTD*) dev->endpoints[endpoint].device_priv2;

    // maximum packet size 4096 .. we could go up to 5*4096
    unint4 length = 4096*5;
    if (data_len < length)
        length = data_len;

    unint1 dir = QT_TOKEN_PID_IN;
    if (direction == USB_DIR_OUT)
        dir = QT_TOKEN_PID_OUT;

    unint4 qtd_data_len = length;

    // convert to physical addresses, as the EHCI Controller only knows physical addresses
    data = (char*) theOS->getHatLayer()->getPhysicalAddress(data);

    // set next qtd in qh overlay to activate transfer
    qh->qh_curtd                = 0;
    qh->qh_overlay.qt_next      = (unint4) qtd;
    qh->qh_overlay.qt_token    &= 0x80000000;
    qh->qh_overlay.qt_altnext   = QT_NEXT_TERMINATE;
    qh->qh_overlay.qt_buffer[0] = 0;


    // FIRST PACKET
    qtd->qt_next         = QT_NEXT_TERMINATE;
    qtd->qt_altnext      = QT_NEXT_TERMINATE;
    qtd->qt_token        = QT_TOKEN_CERR(3) | QT_TOKEN_PID(dir) | QT_TOKEN_STATUS_ACTIVE | QT_TOKEN_TOTALBYTES(qtd_data_len);
    qtd->qt_buffer[0]    = (unint4) data;  // set data to send
    qtd->qt_buffer[1]    = (unint4) alignCeil(data, 4096);           // align next part of the data
    qtd->qt_buffer[2]    = (unint4) alignCeil(data + 4096, 4096);    // align next part of the data
    qtd->qt_buffer[3]    = (unint4) alignCeil(data + 4096*2, 4096);  // align next part of the data
    qtd->qt_buffer[4]    = (unint4) alignCeil(data + 4096*3, 4096);  // align next part of the data

    /* clean data => write back from cache to memory for ehci controller to see the data */
    theOS->getBoard()->getCache()->clean_data((void*)data, data_len);

    SMP_SPINLOCK_GET(QH_Spinlock);
    /* insert transfer at front */
    QH_LINK(qh, QHmain.qh_link);
    MEM_BARRIER();
    QH_LINK(&QHmain, qh);
    SMP_SPINLOCK_FREE(QH_Spinlock);

    /* wait for timeout of completion */
    TIMEOUT_WAIT(QT_TOKEN_GET_STATUS(qtd->qt_token) & QT_TOKEN_STATUS_ACTIVE, timeout_ms * 1000);

    if (direction == USB_DIR_IN) {
        /* be sure data is fetched from memory again */
        theOS->getBoard()->getCache()->invalidate_data((void*)data, data_len);
    }

    // get number of bytes transferred
    int num = 0;
    if (data_len > 0) {
        num = data_len - QT_TOKEN_GET_TOTALBYTES(qtd->qt_token);
    }

    /* check on error .. ignore ping state bit as this is taken care of by the HC*/
    if ((QT_TOKEN_GET_STATUS(qtd->qt_token) & 0xFE) != 0x0) {
        LOG(ARCH, ERROR, "USB_EHCI_Host_Controller::USBBulkMsg() error packet: Direction %d", direction);
        LOG(ARCH, ERROR, "USBSTS    : %x", INW(operational_register_base + USBSTS_OFFSET));
        LOG(ARCH, ERROR, "USBCMD    : %x", INW(operational_register_base + USBCMD_OFFSET));
        dumpAsyncList();
        LOG(ARCH, ERROR, "QH        : %x", qh);
        LOG(ARCH, ERROR, "qtd     \t(%08x)\tstatus: %x", qtd, qtd->qt_token);

        for (int i  = 0; i < 8; i++) {
            if (qtd->qt_token & qtdStatusCodes[i].bitmask) {
                LOG(ARCH, ERROR, "Status: %s", qtdStatusCodes[i].name);
            }
        }

        num = -1;

        // clear USBERRINT
        OUTW(operational_register_base + USBSTS_OFFSET, 1 << 1);
    }

    // unlink data transfer
    SMP_SPINLOCK_GET(QH_Spinlock);
    // search for queue head referencing qh
    QH* prevQH = &QHmain;
    while ( (prevQH->qh_link & ~0x1f) != (unint4)qh) {
        prevQH = (QH*) (prevQH->qh_link & ~0x1f);
    }
    QH_LINK(prevQH, qh->qh_link);
    SMP_SPINLOCK_FREE(QH_Spinlock);

    dev->release();

    return (num);    // return number of bytes transferred
}



/*****************************************************************************
 * Method: USB_EHCI_Host_Controller::sendUSBControlMsg(USBDevice *dev,
 *                           unint1     endpoint,
 *                           char      *control_msg,
 *                           unint1     direction = USB_DIR_IN,
 *                           unint1     length = 0,
 *                           char      *data = 0);
 *
 * @description
 *  Sends a USB control message (setup msg) with optional data read/write phase
 *  to a USB device
 *
 * @params
 *  dev         The device the control message shall be send to
 *  endpoint    The endpoint of the device the control message will be send to
 *  control_msg Pointer to the control msg (8 bytes) to be send
 *  direction   Direction of the data transfer following the control msg
 *              USB_DIR_IN or USB_DIR_OUT
 *  length      Length of bytes to be transferred
 *  data        Pointer to the data to be send or the buffer to be filled with
 *              the received data
 *
 * @returns
 *  int         Number of bytes transferred inside the data phase
 *******************************************************************************/
int USB_EHCI_Host_Controller::sendUSBControlMsg(USBDevice*  dev,
                                                unint1      endpoint,
                                                char*       control_msg,
                                                unint1      direction,
                                                unint1      data_len,
                                                char*       data) {
    // finally link the queue head to the qtd chain
    volatile QH* qh = (QH*) dev->endpoints[endpoint].device_priv;
    if (qh == 0)
        return (cError);

    dev->acquire();

    // be sure control message is in cache inhibit memory for EHCI to access it.
    memcpy(controlMsg, control_msg, 8);

    // convert to physical addresses, as the EHCI Controller only knows physical addresses
    data         = (char*) theOS->getHatLayer()->getPhysicalAddress(data);

    // get some memory for qtds
    qTD* qtd  = &qtds[(qtdnum++) & 0x7];
    qTD* qtd2 = &qtds[(qtdnum++) & 0x7];
    qTD* lastqtd = qtd2;

    // SETUP STAGE
    qtd->qt_next        = QT_NEXT_TERMINATE;
    qtd->qt_altnext     = QT_NEXT_TERMINATE;
    qtd->qt_token       = QT_TOKEN_CERR(3) | QT_TOKEN_IOC(0) | QT_TOKEN_PID(QT_TOKEN_PID_SETUP) | QT_TOKEN_STATUS_ACTIVE | QT_TOKEN_TOTALBYTES(8);
    qtd->qt_buffer[0]   = (unint4) controlMsg;  // set control message to send
    qtd->qt_buffer[1]   = (unint4) alignCeil(controlMsg, 4096);  // set control message to send

    unint1 dir = QT_TOKEN_PID_IN;
    if (direction == USB_DIR_OUT)
        dir = QT_TOKEN_PID_OUT;

    // SETUP TOKEN
    QT_LINK(qtd, qtd2);

    // IN Packet
    qtd2->qt_next       = QT_NEXT_TERMINATE;
    qtd2->qt_altnext    = QT_NEXT_TERMINATE;
    qtd2->qt_token      = QT_TOKEN_CERR(3) | QT_TOKEN_IOC(0) | QT_TOKEN_PID(dir) | QT_TOKEN_STATUS_ACTIVE | QT_TOKEN_TOTALBYTES(data_len);
    qtd2->qt_buffer[0]  = (unint4) data;  // set data buffer
    qtd2->qt_buffer[1]  = (unint4) alignCeil(data, 4096);  // set control message to send

    // push data through cache to memory.. so EHCI HC sees the data
    theOS->getBoard()->getCache()->clean_data((void*)data, data_len);

    // set next qtd in qh overlay to activate transfer
    qh->qh_curtd           = QT_NEXT_TERMINATE;
    qh->qh_overlay.qt_next = (unint4) qtd;

    SMP_SPINLOCK_GET(QH_Spinlock);
    QH_LINK(qh, &QHmain);
    MEM_BARRIER();
    QH_LINK(&QHmain, qh);
    SMP_SPINLOCK_FREE(QH_Spinlock);

    volatile unint4 timeout = 400000;
    while ((QT_TOKEN_GET_STATUS(lastqtd->qt_token) & QT_TOKEN_STATUS_ACTIVE) && timeout)
    {
        timeout--;
        /* control messages are much slower */
        kwait_us_nonmem(10);
    }


    if (direction == USB_DIR_IN) {
        theOS->getBoard()->getCache()->invalidate_data((void*)data, data_len);
    }

    // get number of bytes transferred
    unint4 num = 0;
    if (data_len > 0)
        num = data_len - QT_TOKEN_GET_TOTALBYTES(lastqtd->qt_token);

    // stop execution of this queue head
    if (timeout == 0 || (QT_TOKEN_GET_STATUS(lastqtd->qt_token) & 0xFE) != 0x0) {
        LOG(ARCH, ERROR, "USB_EHCI_Host_Controller::sendUSBControlMsg() error on control packet..");
        LOG(ARCH, ERROR, "timeout: %d lastqtd \t %08x", timeout, lastqtd);
        dumpAsyncList();
        LOG(ARCH, ERROR, "qtd1    \t(%08x)\tstatus: %x", qtd,  qtd->qt_token);
        LOG(ARCH, ERROR, "qtd2    \t(%08x)\tstatus: %x", qtd2, qtd2->qt_token);
        for (int i  = 0; i < 8; i++) {
            if (lastqtd->qt_token & qtdStatusCodes[i].bitmask) {
                LOG(ARCH, ERROR, "Status: %s", qtdStatusCodes[i].name);
            }
        }

        //return as we can not use this port
        if (!(direction & USB_IGNORE_ERROR)) {
            num = -1;
        }
    }

    SMP_SPINLOCK_GET(QH_Spinlock);
    QH_LINK(&QHmain, &QHmain);
    SMP_SPINLOCK_FREE(QH_Spinlock);

    dev->release();

    return (num);
}

/*****************************************************************************
 * Method: USB_EHCI_Host_Controller::resetPort(unint1 port)
 *
 * @description
 *  Tries to reset the given port.
 *******************************************************************************/
ErrorT USB_EHCI_Host_Controller::resetPort(unint1 port) {
    unint4 portsc = INW(operational_register_base + PORTSC1_OFFSET + 4* port);
    SETBITS(portsc, 8, 8, 1);  // start port reset
    OUTW(operational_register_base + PORTSC1_OFFSET + 4* port, portsc);

    /* give some time to the device to change its address */
    kwait(5);

    portsc = INW(operational_register_base + PORTSC1_OFFSET + 4*port);
    SETBITS(portsc, 8, 8, 0);  // start port reset
    OUTW(operational_register_base + PORTSC1_OFFSET + 4*port, portsc);

    unint4 timeout = 400;
    while ((!(INW(operational_register_base + PORTSC1_OFFSET + 4*port) & (1 << 2))) && timeout) {
        timeout--;
        kwait_us(10);
    }

    if (timeout == 0)
        return (cError);

    return (cOk);
}



/*****************************************************************************
 * Method: USB_EHCI_Host_Controller::insertPeriodicQH(QH* qh, int poll_rate)
 *
 * @description
 *  Inserts a Queue Head into the periodic queue
 *  Poll rates are given in milliseconds. No support for poll rates < 1 ms.
 *  Be aware: 1 ms poll rate will fill up the whole queue.
 *
 * @params
 *  qh          The Queue Head to be inserted
 *  poll_rate   Poll rate in ms
 *
 *******************************************************************************/
void USB_EHCI_Host_Controller::insertPeriodicQH(QH* qh, int poll_rate) {
    qh->qh_link = QH_LINK_TERMINATE;

    // avoid filling up the whole queue
    if (poll_rate <= 1) {
        poll_rate = 2;
    }

    for (int i = poll_rate; i < FRAME_LIST_SIZE; i += poll_rate) {
        if (framelist[i] != QH_LINK_TERMINATE) {
            // insert into list close behind this element
            // we keep at maximum one element per periodic entry and do
            // not create a list as we would have to create lots of
            // additional queue heads and track them
            for (int j = 1; j < poll_rate; j++) {
                if (framelist[i + j] != QH_LINK_TERMINATE) {
                    framelist[i + j] = (unint4) qh | QH_LINK_TYPE_QH;
                    break;
                }
            }
        } else {
            framelist[i] = (unint4) qh | QH_LINK_TYPE_QH;
        }
    }
}

/*****************************************************************************
 * Method: USB_EHCI_Host_Controller::removefromPeriodic(QH* qh, int poll_rate)
 *
 * @description
 *  Removes a queue head from the periodic list effectively stopping
 *  the periodic transfer.
 *
 * @params
 *  qh          The Queue Head to be removed
 *
 *******************************************************************************/
void USB_EHCI_Host_Controller::removefromPeriodic(QH* qh, int poll_rate) {
    for (int i = poll_rate; i < FRAME_LIST_SIZE; i++) {
        if (framelist[i] != QH_LINK_TERMINATE) {
            QH* current_qh = reinterpret_cast<QH*>(framelist[i] & ~0x1f);

            if (current_qh == qh) {
                // remove this qh reference here
                framelist[i] = QH_LINK_TERMINATE;
            }
        }
    }
}

USB_EHCI_Host_Controller::~USB_EHCI_Host_Controller() {
    // deletion not supported
}



/*****************************************************************************
 * Method: USB_EHCI_Host_Controller::handleIRQ()
 *
 * @description
 *  Handle EHCI IRQs.
 *  Will iterate over all registered device drivers of usb devices to handle the IRQ.
 *  The EHCI HC can not know which device the IRQ is for, thus it needs to ask every device.
 *  This is very time consuming!
 *
 * @returns
 *  ErrorT         Error Code
 *******************************************************************************/
ErrorT USB_EHCI_Host_Controller::handleIRQ(int irq) {
    LOG(ARCH, DEBUG, "EHCI Interrupt");

    for (int i = 0; i < 10; i++) {
        if (registered_devices[i] != 0) {
            registered_devices[i]->handleInterrupt();
        }
    }

    /* todo check port change etc bits */

    if (INW(operational_register_base + USBSTS_OFFSET) & 0x1) {
        interruptPending = true;
        clearIRQ(irq);
    } else {
        interruptPending = false;
    }

    return (cOk );
}

/*****************************************************************************
 * Method: USB_EHCI_Host_Controller::enableIRQ()
 *
 * @description
 *  Enables EHCI HC IRQs
 *
 * @returns
 *  ErrorT         Error Code
 *******************************************************************************/
ErrorT USB_EHCI_Host_Controller::enableIRQ(int irq) {
    /* todo enable port change etc irqs */
   theOS->getBoard()->getInterruptController()->unmaskIRQ(EHCI_IRQ);
   OUTW(operational_register_base + USBINTR_OFFSET, 0x1);
   return (cOk );
}

/*****************************************************************************
* Method: USB_EHCI_Host_Controller::disableIRQ()
*
* @description
*  Disables all EHCI HC IRQs
*
* @returns
*  ErrorT         Error Code
*******************************************************************************/
ErrorT USB_EHCI_Host_Controller::disableIRQ(int irq) {
   theOS->getBoard()->getInterruptController()->maskIRQ(EHCI_IRQ);
   OUTW(operational_register_base + USBINTR_OFFSET, 0x0);
   return (cOk );
}


/*****************************************************************************
 * Method: USB_EHCI_Host_Controller::clearIRQ()
 *
 * @description
 *  Clears all pending EHCI HC IRQs.
 *
 * @returns
 *  ErrorT         Error Code
 *******************************************************************************/
ErrorT USB_EHCI_Host_Controller::clearIRQ(int irq) {
    OUTW(operational_register_base + USBSTS_OFFSET, 0x3f);
    return (cOk );
}




/*****************************************************************************
 * Method: EHCI_USBDevice::deactivate()
 *
 * @description
 *  deactivates the device and stops all transfers
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT EHCI_USBDevice::deactivate() {
    for (int i = 0; i <= this->numEndpoints; i++) {
        // remove the allocated queue heads
        if (endpoints[i].type == Interrupt) {
            LOG(ARCH, INFO, "EHCI_USBDevice: Stopping Periodic Transfer for Endpoint %d.", i);
            ((USB_EHCI_Host_Controller*)controller)->removefromPeriodic((QH*)endpoints[i].device_priv, endpoints[i].poll_frequency);
        }

        // delete its allocated receive buffer
        if (endpoints[i].recv_buffer != 0) {
            delete endpoints[i].recv_buffer;
        }
        if (endpoints[i].device_priv != 0) {
            QHMemPool.free(endpoints[i].device_priv);
        }
        if (endpoints[i].device_priv2 != 0) {
            QTDMemPool.free(endpoints[i].device_priv2);
        }

        // can only be removed if the associated device driver is removed from controller
        endpoints[i].device_priv  = 0;
        endpoints[i].device_priv2 = 0;
    }

    return (cOk);
}

/*****************************************************************************
 * Method: EHCI_USBDevice::reactivateEp(int num)
 *
 * @description
 *  Reactivates the given endpoint if it has been deactivated before.
 *  Also needs to be called for interrupt endpoints after IRQ handling as interrupt
 *  transfers are automatically stopped until reactivated.
 *
 * @params
 *  num         Endpoint to reactivate.
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT EHCI_USBDevice::reactivateEp(int num) {
    // reactivation only plausible for interrupt transfers
    if (endpoints[num].type != Interrupt)
        return (cError);

    QH* qh    = (QH*)  endpoints[num].device_priv;
    qTD* qtd2 = (qTD*) endpoints[num].device_priv2;

    if (qh == 0 || qtd2 == 0)
        return (cError);

    if ((QT_TOKEN_GET_STATUS(qh->qh_overlay.qt_token) != 0x80)) {
        // set qtd back to active
        qtd2->qt_next    = QT_NEXT_TERMINATE;
        qtd2->qt_altnext = QT_NEXT_TERMINATE;
        qtd2->qt_token   = QT_TOKEN_CERR(0) | QT_TOKEN_IOC(1) | QT_TOKEN_PID(QT_TOKEN_PID_IN)  | QT_TOKEN_STATUS_ACTIVE | QT_TOKEN_TOTALBYTES(endpoints[num].interrupt_receive_size);
        qtd2->qt_buffer[0]      = (unint4) endpoints[num].recv_buffer;
        qtd2->qt_buffer[1]      = (unint4) alignCeil(endpoints[num].recv_buffer, 4096);
        qh->qh_overlay.qt_next  = (unint4) qtd2;
        return (cOk);
    }

    return (cError);
}

/*****************************************************************************
 * Method: EHCI_USBDevice::activateEndpoint(int num)
 *
 * @description
 *  activates the endpoint and creates a queue head for async transfer
 *  If QH or qtd is given it will be used instead of the endpoints
 *  QHs and Qtds for the given endpoint.
 *
 * @params
 *  endpoint     Endpoint number to activate
 *  qh           Optional Queue Head to use for transfers
 *  qtd          Optional QTD to use for transfers
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT EHCI_USBDevice::activateEndpoint(int num) {
    if (num > 4)
        return (cError);

    if (endpoints[num].device_priv != 0)
        return (reactivateEp(num));

    // create a queue head and insert it into the list
    // generate a queue head for this device inside the asynchronous list
    QH* queue_head_new = 0; //pqh;
    if (queue_head_new == 0) {
        /* allocate a new QH from the global pool */
        queue_head_new = reinterpret_cast<QH*>(QHMemPool.allocate());
        if (queue_head_new == 0)
        {
            LOG(ARCH, ERROR, "USB_Device:activateEndpoint %d no more memory for queue head");
            return (cHeapMemoryExhausted);
        }
    }

    LOG(ARCH, DEBUG, "USB_Device:activateEndpoint %d new QH at %x", num, queue_head_new);

    // store address and clear area
    endpoints[num].device_priv = queue_head_new;
    memset(reinterpret_cast<void*>(queue_head_new), 0, sizeof(QH));

    // hs : QH_ENDPT1_EPS_HS
    queue_head_new->qh_endpt1 = QH_ENDPT1_DEVADDR(this->addr) | QH_ENDPT1_MAXPKTLEN(endpoints[num].descriptor.wMaxPacketSize)
                              | QH_ENDPT1_EPS(this->speed)    | QH_ENDPT1_ENDPT(endpoints[num].address);

    // allow 1 packet per microframe
    if (this->speed == 2) {
        queue_head_new->qh_endpt2 = QH_ENDPT2_MULT(1);
    } else {
        queue_head_new->qh_endpt2 = QH_ENDPT2_MULT(1) | QH_ENDPT2_HUBADDR(this->parent->getDevice()->addr) | QH_ENDPT2_PORTNUM(this->port);
        if (num == 0)
            queue_head_new->qh_endpt1 |= QH_ENDPT1_C(1);
    }

    // deactivate qtd transfer
    queue_head_new->qh_overlay.qt_next = QT_NEXT_TERMINATE;

    // generate one qtd for bulk data transfer
    qTD* qtd = 0; //pqtd;
    if (qtd == 0) {
        /* allocate a new qTD from the global pool */
        qtd = reinterpret_cast<qTD*>(QTDMemPool.allocate());
        if (queue_head_new == 0)
        {
           LOG(ARCH, ERROR, "USB_Device:activateEndpoint %d no more memory for qTD");
           QHMemPool.free(queue_head_new);
           endpoints[num].device_priv = queue_head_new = 0;
           return (cHeapMemoryExhausted);
        }
    }

    endpoints[num].device_priv2 = qtd;

    if (this->endpoints[num].type != Interrupt) {
        // let this new queue head be the head of the async list
        queue_head_new->qh_endpt1 |= QH_ENDPT1_RL(3);
        queue_head_new->qh_link = QH_LINK_TYPE_QH | ((unint4) queue_head_new);

    } else {
        // Interrupt Endpoint
        LOG(ARCH, DEBUG, "USB_Device:activateEndpoint %d new qtd at %x",num, qtd);

        queue_head_new->qh_overlay.qt_next = (unint4) qtd;
        queue_head_new->qh_endpt2 |= 1;  // S-Mask (todo: should be iterated for multiple qhs in the same ms)
        queue_head_new->qh_link = QT_NEXT_TERMINATE;

        qtd->qt_next    = QT_NEXT_TERMINATE;
        qtd->qt_altnext = QT_NEXT_TERMINATE;

        queue_head_new->qh_overlay.qt_token = 0;
        queue_head_new->qh_overlay.qt_altnext = QT_NEXT_TERMINATE;

        LOG(ARCH, DEBUG, "USB_Device:activateEndpoint %d getting interrupt recv_buffer of size %d",num, endpoints[num].interrupt_receive_size);
        // get interrupt receive buffer
        endpoints[num].recv_buffer = reinterpret_cast<char*>(theOS->getMemoryManager()->alloci(endpoints[num].interrupt_receive_size + 8));  // delete @ USBDevice::deactivate
        memset(endpoints[num].recv_buffer, 0, endpoints[num].interrupt_receive_size);
        LOG(ARCH, INFO, "USB_Device:activateEndpoint %d new recv_buffer at %x",num, endpoints[num].recv_buffer);

        qtd->qt_buffer[0] = (unint4) endpoints[num].recv_buffer;
        qtd->qt_buffer[1] = (unint4) alignCeil(endpoints[num].recv_buffer, 4096);

        // setup interrupt transfer qtd .. try to receive interrupt_receive_size bytes
        qtd->qt_token = QT_TOKEN_CERR(0) | QT_TOKEN_IOC(1) | QT_TOKEN_PID(QT_TOKEN_PID_IN)
                        | QT_TOKEN_STATUS_ACTIVE | QT_TOKEN_TOTALBYTES(endpoints[num].interrupt_receive_size);

        if (endpoints[num].poll_frequency <= 1)
            endpoints[num].poll_frequency = (unint1) (22 + num + this->addr);

        /* insert qh in periodic list */
        ((USB_EHCI_Host_Controller*) controller)->insertPeriodicQH(queue_head_new, endpoints[num].poll_frequency);
    }
    return (cOk);
}

EHCI_USBDevice::EHCI_USBDevice(USB_Host_Controller *p_controller, USBHub *p_parent, unint1 u_port, unint1 u_speed)
     : USBDevice(p_controller, p_parent, u_port, u_speed) {

    if (this->activateEndpoint(0) != cOk) {
      LOG(ARCH, ERROR, "USBDevice: Activating Controller Endpoint failed..");
    }
}


ErrorT EHCI_USBDevice::setAddress(unint1 addr) {
    ErrorT ret = USBDevice::setAddress(addr);

    if (isOk(ret)) {
        /* adapt the queue head of this device endpoints */
        for (int i = 0; i < 4; i++) {
            if (endpoints[i].device_priv != 0) {
                QH* qh = (QH*) endpoints[i].device_priv;
                SETBITS(qh->qh_endpt1, 6, 0, this->addr);
            }
        }
    }
    return (ret);
}

ErrorT EHCI_USBDevice::setMaxPacketSize(int endpoint, unint2 maxSize) {
    QH* qh = (QH*) endpoints[endpoint].device_priv;
    if (qh == 0) {
        return (cError);
    }
    unint4 cap_token = qh->qh_endpt1;
    SETBITS(cap_token, 26, 16, maxSize);
    qh->qh_endpt1 = cap_token;
    return (cOk);
}

int EHCI_USBDevice::getEndpointTransferSize(int num) {
    volatile QH* qh = (QH*) endpoints[num].device_priv;
    if (qh == 0)
           return (cError );
    volatile qTD* qtd2 = reinterpret_cast<qTD*>(qh->qh_curtd);

    return (endpoints[num].interrupt_receive_size - QT_TOKEN_GET_TOTALBYTES(qtd2->qt_token));
}

TEndPointTransferState EHCI_USBDevice::getEndpointTransferState(int num) {
      volatile QH* qh = (QH*) endpoints[num].device_priv;
      if (qh == 0)
          return (EP_ERROR);

      volatile qTD* qtd = reinterpret_cast<qTD*>(qh->qh_curtd);

      if ((unint4) qtd <= QT_NEXT_TERMINATE)
          return (EP_ERROR);

      if ((QT_TOKEN_GET_STATUS(qh->qh_overlay.qt_token) & 0xFE) == 0x80) {
          return (EP_TRANSFER_ACTIVE);
      }

      if ((QT_TOKEN_GET_STATUS(qh->qh_overlay.qt_token) & 0xFE) != 0x0) {
          return (EP_TRANSFER_ERROR);
      }

      /* invalidate received data area to make the data visible to the user */
      // theOS->getBoard()->getCache()->invalidate_data((void*)endpoints[num].recv_buffer, endpoints[num].interrupt_receive_size);
      return (EP_TRANSFER_FINISHED);
}

