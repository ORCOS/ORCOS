/*
 * USBEHCIHostController.hh
 *
 *  Created on: 03.02.2013
 *   Copyright & Author: dbaldin
 */

#ifndef USBEHCIHOSTCONTROLLER_HH_
#define USBEHCIHOSTCONTROLLER_HH_

#include "inc/types.hh"
#include "inc/memio.h"
#include "inc/error.hh"

#include "../usb/USBDeviceDriver.hh"
#include "hal/GenericDeviceDriver.hh"
#include "arch/shared/usb/usb.h"
#include "arch/shared/usb/USBDevice.hh"
#include "arch/shared/usb/USBHostController.hh"

//----------------------------
// Defines
//----------------------------

#ifndef USB_SUPPORT_ENABLED
#define USB_SUPPORT_ENABLED 1
#endif



/*
 00h 1 CAPLENGTH Core Capability Register Length
 01h 1 Reserved Core N/A
 02h 2 HCIVERSION Core Interface Version Number
 04h 4 HCSPARAMS Core Structural Parameters
 08h 4 HCCPARAMS Core Capability Parameters
 0Ch 8 HCSP-PORTROUTE Core Companion Port Route Description
 */

#define CAPLENGTH_OFFSET            0x0
#define HCIVERSION_OFFSET           0x1
#define HCSPARAMS_OFFSET            0x4
#define HCCPARAMS_OFFSET            0x8
#define HCSP_PORTROUTE_OFFSET       0xC   // optional field

/* USB EHCI Hardware register offsets:
 00h USBCMD                 USB Command Core
 04h USBSTS                 USB Status Core
 08h USBINTR                USB Interrupt Enable Core
 0Ch FRINDEX                USB Frame Index Core
 10h CTRLDSSEGMENT          4G Segment Selector Core
 14h PERIODICLISTBASE       Frame List Base Address Core
 18h ASYNCLISTADDR          Next Asynchronous List Address Core
 1C-3Fh                     Reserved Core
 40h CONFIGFLAG             Configured Flag Register Aux
 44h PORTSC(1-N_PORTS)      Port Status/Control Aux
 */

// USB EHCI Register offsets
#define USBCMD_OFFSET             0x0
#define USBSTS_OFFSET             0x4
#define USBINTR_OFFSET            0x8
#define FRINDEX_OFFSET            0xc
#define CTRLDSSEGMENT_OFFSET      0x10
#define PERIODICLISTBASE_OFFSET   0x14
#define ASYNCLISTADDR_OFFSET      0x18
#define CONFIGFLAG_OFFSET         0x40
#define PORTSC1_OFFSET            0x44

/*
 * Workload Types
 *
 00b Isochronous Transfer Descriptor (iTD, see Section 3.3)
 01b Queue Head (QH, see Section 3.6)
 10b Split Transaction Isochronous Transfer Descriptor (siTD, see Section 3.4).
 11b Frame Span Traversal Node (FSTN, see Section 3.7)
 *
 */

#define EHCI_DS_TYPE_iTD                0
#define EHCI_DS_TYPE_QH                 1
#define EHCI_DS_TYPE_siTD               2
#define EHCI_DS_TYPE_FSTN               3

//----------------------------
// Structures
//----------------------------
#define    QT_NEXT_TERMINATE            1
#define QT_TOKEN_DT(x)                  (((x) & 0x1) << 31)         /* Data Toggle */
#define QT_TOKEN_GET_DT(x)              (((x) >> 31) & 0x1)
#define QT_TOKEN_TOTALBYTES(x)          (((x) & 0x7fff) << 16)      /* Total Bytes to Transfer */
#define QT_TOKEN_GET_TOTALBYTES(x)      (((x) >> 16) & 0x7fff)
#define QT_TOKEN_IOC(x)                 (((x) & 0x1) << 15)         /* Interrupt On Complete */
#define QT_TOKEN_CPAGE(x)               (((x) & 0x7) << 12)         /* Current Page */
#define QT_TOKEN_CERR(x)                (((x) & 0x3) << 10)         /* Error Counter */
#define QT_TOKEN_PID(x)                 (((x) & 0x3) << 8)          /* PID Code */
#define QT_TOKEN_PID_OUT                0x0
#define QT_TOKEN_PID_IN                 0x1
#define QT_TOKEN_PID_SETUP              0x2
#define QT_TOKEN_STATUS(x)              (((x) & 0xff) << 0)         /* Status */
#define QT_TOKEN_GET_STATUS(x)          (((x) >> 0) & 0xff)
#define QT_TOKEN_STATUS_ACTIVE          0x80
#define QT_TOKEN_STATUS_HALTED          0x40
#define QT_TOKEN_STATUS_DATBUFERR       0x20
#define QT_TOKEN_STATUS_BABBLEDET       0x10
#define QT_TOKEN_STATUS_XACTERR         0x08
#define QT_TOKEN_STATUS_MISSEDUFRAME    0x04
#define QT_TOKEN_STATUS_SPLITXSTATE     0x02
#define QT_TOKEN_STATUS_PERR            0x01
#define QT_BUFFER_CNT                   5

#define QT_LINK(from, to)               (from)->qt_next     = (((unint4)to) & ~0x1f)
#define QT_LINK_ALT(from, to)           (from)->qt_altnext  = (((unint4)to) & ~0x1f)
#define QH_LINK(from, to)               (from)->qh_link     = ((((unint4)to) & ~0x1f) | 0x2)

#define QH_LINK_TERMINATE               1
#define QH_LINK_TYPE_ITD                0
#define QH_LINK_TYPE_QH                 2
#define QH_LINK_TYPE_SITD               4
#define QH_LINK_TYPE_FSTN               6

#define QH_ENDPT1_RL(x)                 (((x) & 0xf) << 28)    /* NAK Count Reload */
#define QH_ENDPT1_C(x)                  (((x) & 0x1) << 27)    /* Control Endpoint Flag */
#define QH_ENDPT1_MAXPKTLEN(x)          (((x) & 0x7ff) << 16)  /* Maximum Packet Length */
#define QH_ENDPT1_H(x)                  (((x) & 0x1) << 15)    /* Head of Reclamation List Flag */
#define QH_ENDPT1_DTC(x)                (((x) & 0x1) << 14)    /* Data Toggle Control */
#define QH_ENDPT1_DTC_IGNORE_QTD_TD     0x0
#define QH_ENDPT1_DTC_DT_FROM_QTD       0x1
#define QH_ENDPT1_EPS(x)                (((x) & 0x3) << 12)    /* Endpoint Speed */
#define QH_ENDPT1_EPS_FS                0x0
#define QH_ENDPT1_EPS_LS                0x1
#define QH_ENDPT1_EPS_HS                0x2
#define QH_ENDPT1_ENDPT(x)              (((x) & 0xf) << 8)    /* Endpoint Number */
#define QH_ENDPT1_I(x)                  (((x) & 0x1) << 7)    /* Inactivate on Next Transaction */
#define QH_ENDPT1_DEVADDR(x)            (((x) & 0x7f) << 0)    /* Device Address */

#define QH_ENDPT2_MULT(x)               (((x) & 0x3)  << 30)    /* High-Bandwidth Pipe Multiplier */
#define QH_ENDPT2_PORTNUM(x)            (((x) & 0x7f) << 23)    /* Port Number */
#define QH_ENDPT2_HUBADDR(x)            (((x) & 0x7f) << 16)    /* Hub Address */
#define QH_ENDPT2_UFCMASK(x)            (((x) & 0xff) << 8)    /* Split Completion Mask */
#define QH_ENDPT2_UFSMASK(x)            (((x) & 0xff) << 0)    /* Interrupt Schedule Mask */


/* Queue Element Transfer Descriptor (qTD). */
typedef struct {
    /* this part defined by EHCI spec */
    volatile unint4 qt_next;                        /* see EHCI 3.5.1 */
    volatile unint4 qt_altnext;                     /* see EHCI 3.5.2 */
    volatile unint4 qt_token;                       /* see EHCI 3.5.3 */
    volatile unint4 qt_buffer[QT_BUFFER_CNT];       /* see EHCI 3.5.4 */
#ifdef PLATFORM_64BIT
    volatile unint4 qt_buffer_hi[QT_BUFFER_CNT];    /* Appendix B */
    /* pad struct for 32 byte alignment */
     volatile unint4 unused[3];
#endif
} qTD __attribute__((aligned(4)));

/* Queue Head (QH). */
typedef struct {
    // link to the next queue head in the queue head list
    volatile unint4 qh_link;
    // endpoints capabilities word 1
    volatile unint4 qh_endpt1;
    // endpoints capabilities word 2
    volatile unint4 qh_endpt2;
    // current queue transfer descriptor
    volatile unint4 qh_curtd;

    volatile qTD qh_overlay;
} QH;



/*!
 *
 *  The USBDevice Class represents a generic attached USB Device.
 *  It holds information on the descriptors received from the device
 *  as well as the topology information (parent) this device is attached to.
 *
 */
class EHCI_USBDevice  : public USBDevice {
private:
    /*****************************************************************************
     * Method: reactivateEp(int num)
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
    ErrorT reactivateEp(int num);

public:
    // constructs a new USB device
    // a queue head is created inside the asynchronous list for this device
    EHCI_USBDevice(USB_Host_Controller *controller, USBDevice *parent, unint1 port, unint1 speed);

    ~EHCI_USBDevice() {};



    /*****************************************************************************
     * Method: activateEndpoint(int num, QH* qh = 0, qTD* qtd = 0)
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
    ErrorT activateEndpoint(int endpoint);

    /*****************************************************************************
     * Method: deactivate()
     *
     * @description
     *  deactivates the device and stops all transfers
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT deactivate();

    ErrorT setAddress(unint1 addr);

    ErrorT setMaxPacketSize(int endpoint, unint2 maxSize);

    TEndPointTransferState getEndpointTransferState(int num);
    int getEndpointTransferSize(int num);
};



class USB_EHCI_Host_Controller: public USB_Host_Controller {
private:
    // this flag indicates whether this object is operational
    bool operational;

    /*!
     *  The physical base address of this EHCI controller
     */
    unint4 hc_base;

    /*!
     *  The operational registers base address of the EHCI HC.
     */
    unint4 operational_register_base;

    // address of the async queue head base registers
    unint4 async_qh_reg;

    ErrorT resetPort(unint1 port);
public:
    /*****************************************************************************
     * Method: USB_EHCI_Host_Controller(unint4 ehci_dev_base)
     *
     * @description
     *  Creates a new instance of the EHCI HC
     *
     * @params
     *  ehci_dev_base The physical base address of the EHCI HC
     *******************************************************************************/
    explicit USB_EHCI_Host_Controller(unint4 ehci_dev_base);


    /*****************************************************************************
       * Method: Init(unint4 priority)
       *
       * @description
       *  Try to initialize the USB EHCI Controller.
       *  Upon success the ehci controller is registered at the interrupt manager using the given priority.
       *
       * @returns
       *  ErrorT         Error Code
       *******************************************************************************/
      ErrorT Init(unint4 priority);

      /*****************************************************************************
       * Method: handleIRQ()
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
       ErrorT handleIRQ(int irq);

       /*****************************************************************************
        * Method: enableIRQ()
        *
        * @description
        *  Enables EHCI HC IRQs
        *
        * @returns
        *  ErrorT         Error Code
        *******************************************************************************/
        ErrorT enableIRQ(int irq);

       /*****************************************************************************
        * Method: disableIRQ()
        *
        * @description
        *  Disables EHCI HC IRQs
        *
        * @returns
        *  ErrorT         Error Code
        *******************************************************************************/
       ErrorT disableIRQ(int irq);

       /*****************************************************************************
        * Method: clearIRQ()
        *
        * @description
        *  Clears all pending EHCI HC IRQs.
        *
        * @returns
        *  ErrorT         Error Code
        *******************************************************************************/
       ErrorT clearIRQ(int irq);

       /*****************************************************************************
        * Method: sendUSBControlMsg(USBDevice *dev,
                               unint1 endpoint,
                               char *control_msg,
                               unint1 direction = USB_DIR_IN,
                               unint1 length = 0,
                               char *data = 0)
        *
        * @description
        *  see USB_Host_Controller::sendUSBControlMsg
        *******************************************************************************/
       int sendUSBControlMsg(USBDevice *dev,
                               unint1 endpoint,
                               char *control_msg,
                               unint1 direction = USB_DIR_IN,
                               unint1 length = 0,
                               char *data = 0);

       /*****************************************************************************
        * Method: USBBulkMsg(USBDevice *dev,
                        unint1 endpoint,
                        unint1 direction,
                        unint2 data_len,
                        char *data)
        *
        * @description
        *  see USB_Host_Controller::USBBulkMsg
        *******************************************************************************/
       int USBBulkMsg(USBDevice *dev,
                        unint1 endpoint,
                        unint1 direction,
                        unint2 data_len,
                        char *data);


       /*****************************************************************************
        * Method: insertPeriodicQH(QH* qh, int poll_rate)
        *
        * @description
        *  see USB_Host_Controller::insertPeriodicQH
        *******************************************************************************/
       void insertPeriodicQH(QH* qh, int poll_rate);

       /*****************************************************************************
        * Method: removefromPeriodic(QH* qh, int poll_rate)
        *
        * @description
        *  see USB_Host_Controller::removefromPeriodic
        *******************************************************************************/
       void removefromPeriodic(QH* qh, int poll_rate);


       USBDevice* createDevice(USBDevice *parent, unint1 port, unint1 speed) {
           return (new EHCI_USBDevice(this, parent, port, speed));
       }

    // destructor
    ~USB_EHCI_Host_Controller();
};

#endif /* USBEHCIHOSTCONTROLLER_HH_ */
