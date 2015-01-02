/*
 * MassStorageSCSIUSBDeviceDriver.cc
 *
 *  Created on: 02.05.2013
 *     Copyright & Author: dbaldin
 */

#include "MassStorageSCSIUSBDeviceDriver.hh"
#include "memtools.hh"
#include "kernel/Kernel.hh"
#include "inc/endian.h"

extern Kernel* theOS;

typedef struct {
    unint1 dCBWSignature[4];
    unint1 dCBWTag[4];
    unint1 dCBWDataTransferLength[4];
    unint1 bmCBWFlags;
    unint1 bCBWDLUN;
    unint1 bCBWCBLength;
    unint1 CBWCB[16];
} CBW;

#define USB_BULK_CB_WRAP_LEN    31
#define USB_BULK_CB_SIG         0x43425355  /* Spells out USBC */
#define USB_BULK_IN_FLAG        0x80
#define USB_BULK_OUT_FLAG       0x00

#define SCSI_READ10             0x28
#define SCSI_INQUIRY            0x12
#define SCSI_INQUIRY_LEN        36U

#define SCSI_WRITE10            0x2A
#define SCSI_WRITE10_VERIFY     0x2E

#define SCSI_UNIT_READY         0x0

#define CBW_SET_LUN(cbw, lun) cbw[13] = (static_cast<char>(lun))


unint4 lun ATTR_CACHE_INHIBIT;

static char cbwInquiry[31] __attribute__((aligned(4))) = { 'U', 'S', 'B', 'C',     // CBW Signature
        0x12, 0x34, 0x56, 0x78,             // CBW Tag
        SCSI_INQUIRY_LEN, 0x0, 0x0, 0x0,    // CBW Data Transfer Length
        USB_BULK_IN_FLAG, 0x0, 6,           // Data DIR, CBW LUN, CB Length
        // CB
        SCSI_INQUIRY, 0x0, 0x0, 0x0, SCSI_INQUIRY_LEN, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

static char cbwWrite10[36] = { 'U', 'S', 'B', 'C',     // CBW Signature
        0x11, 0x34, 0x56, 0x12,         // CBW Tag
        0, 0, 0, 0,
        USB_BULK_OUT_FLAG, 0x0, 10,     // Data DIR, CBW LUN, CB Length
        // CB
        SCSI_WRITE10, 0x0, 0, 0, 0, 0, 0x0, 0x0, 0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

static char cbwRead10[36] = { 'U', 'S', 'B', 'C',  // CBW Signature
        0x11, 0x34, 0x56, 0x10,         // CBW Tag
        0, 0, 0, 0,
        USB_BULK_IN_FLAG, 0x0, 10,      // Data DIR, CBW LUN, CB Length
        // CB
        SCSI_READ10, 0x0, 0, 0, 0, 0, 0x0, 0x0, 0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0 };

#if 0
static char cbwTestUnitReady[31] __attribute__((aligned(4))) = {
    'U', 'S', 'B', 'C',            // CBW Signature
    0x12, 0x34, 0x56, 0x78,// CBW Tag
    0x1, 0x0, 0x0, 0x0,// CBW Data Transfer Length
    USB_BULK_IN_FLAG, 0x0, 6,// Data DIR, CBW LUN, CB Length
// CB
    SCSI_UNIT_READY,
    0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0, 0x0,
    0x0, 0x0, 0x0
};
#endif

MassStorageSCSIUSBDeviceDriver::MassStorageSCSIUSBDeviceDriver(USBDevice* p_dev) :
        USBDeviceDriver(),
        BlockDeviceDriver("sdx") {
    this->dev           = p_dev;
    this->bulkin_ep     = 0;
    this->bulkout_ep    = 0;
    this->int_ep        = 0;
    this->sector_size   = 512;
    this->myLUN         = 0;
    this->next          = 0;

    int num         = reinterpret_cast<int>(BlockDeviceDriver::freeBlockDeviceIDs->removeHead());
    char* new_name  = new char[4];
    new_name[0] = 's';
    new_name[1] = 'd';
    new_name[2] = ('0' + num);
    new_name[3] = 0;

    this->name = new_name;

    LOG(ARCH, INFO, "MassStorageSCSIUSBDeviceDriver: new Device attached..");
    LOG(ARCH, INFO, "MassStorageSCSIUSBDeviceDriver: created LUN 0 as '%s'", name);
}

MassStorageSCSIUSBDeviceDriver::MassStorageSCSIUSBDeviceDriver(MassStorageSCSIUSBDeviceDriver* parent, int lun, char* p_name) :
        USBDeviceDriver(),
        BlockDeviceDriver(p_name) {
    this->dev = parent->dev;
    this->bulkin_ep     = parent->bulkin_ep;
    this->bulkout_ep    = parent->bulkout_ep;
    this->int_ep        = 0;
    this->myLUN         = lun;
    this->sector_size   = 512;
    this->next          = 0;

    LOG(ARCH, INFO, "MassStorageSCSIUSBDeviceDriver: created LUN %d as '%s'", lun, name);

    memset(&this->scsi_info, 0, sizeof(InquiryData));
    int error = performTransaction(cbwInquiry, reinterpret_cast<char*>(&this->scsi_info), SCSI_INQUIRY_LEN);
    if (error < 0) {
        LOG(ARCH, ERROR, "MassStorageSCSIUSBDeviceDriver: device inquiry failed. Ignoring LUN %d.", this->myLUN);
    } else {
        theOS->getPartitionManager()->registerBlockDevice(this);
        theOS->getFileManager()->getDirectory("dev")->add(this);
    }
}


/*****************************************************************************
 * Method: MassStorageSCSIUSBDeviceDriver::performTransaction(char* bot_cbw, char* data, unint2 length)
 *
 * @description
 *
 *******************************************************************************/
ErrorT MassStorageSCSIUSBDeviceDriver::performTransaction(char* bot_cbw, char* data, unint2 length) {
    /* be sure data passed to usb driver is in cache inhibit memory */
    char* curcbw    = reinterpret_cast<char*>(theOS->getMemoryManager()->alloci(36, true, 4));
    char* csw       = reinterpret_cast<char*>(theOS->getMemoryManager()->alloci(13, true, 4));

    if (csw == 0 || curcbw == 0) {
        delete curcbw;
        delete csw;
        return (cError );
    }

    CBW_SET_LUN(bot_cbw, myLUN);
    int direction = (bot_cbw[12] >> 7);

    /* be sure cbw to send is in cache inhibit memory if data cache is enabled
     * for ehci controller to be able to access the data  */
    memcpy(curcbw, bot_cbw, 31);

    int error = dev->controller->USBBulkMsg(dev, this->bulkout_ep, USB_DIR_OUT, 31, curcbw);

    if (error < 0) {
        LOG(ARCH, ERROR, "MassStorageSCSIUSBDeviceDriver: cbw send failed..");
        //ResetRecovery();
        delete curcbw;
        delete csw;
        return (cError );
    }

    if (direction == 1) {
        /* we are reading from the block device */
        error = dev->controller->USBBulkMsg(dev, this->bulkin_ep, USB_DIR_IN, length, data);
        if (error < 0) {
            LOG(ARCH, ERROR, "MassStorageSCSIUSBDeviceDriver: bulk msg recv failed..");
            //ResetRecovery();
            delete curcbw;
            delete csw;
            return (cError );
        }

    } else {
        /* we are reading from the block device */
        error = dev->controller->USBBulkMsg(dev, this->bulkout_ep, USB_DIR_OUT, length, data);
        if (error < 0) {
            LOG(ARCH, ERROR, "MassStorageSCSIUSBDeviceDriver: bulk msg send failed..");
            //ResetRecovery();
            delete curcbw;
            delete csw;
            return (cError );
        }
    }

    /* read CSW */
    error = dev->controller->USBBulkMsg(dev, this->bulkin_ep, USB_DIR_IN, 13, csw);
    if (error < 0) {
        LOG(ARCH, ERROR, "MassStorageSCSIUSBDeviceDriver: csw recv failed..");
        //ResetRecovery();
        delete curcbw;
        delete csw;
        return (cError );
    }

    /* check csw */
    // todo use a 4 byte compare operation
    if ((csw[0] == 0x55) && (csw[1] == 0x53) && (csw[2] == 0x42) && (csw[3] == 0x53)) {
        /* return data residue on command passed */
        ErrorT ret = csw[12];
        if (csw[12] == 0)
            ret = csw[8];

        delete curcbw;
        delete csw;
        // todo check tag
        return (ret);

    } else {
        //ResetRecovery();
        delete curcbw;
        delete csw;
        return (cTransactionFailed );
    }

    return (cOk );
}

/*****************************************************************************
 * Method: MassStorageSCSIUSBDeviceDriver::ResetRecovery()
 *
 * @description
 *
 *******************************************************************************/
ErrorT MassStorageSCSIUSBDeviceDriver::ResetRecovery() {
    char msg[8] = { 0x21, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    int error = dev->controller->sendUSBControlMsg(dev, 0, msg);
    if (error < 0) {
        LOG(ARCH, ERROR, "MassStorageSCSIUSBDeviceDriver: could not reset mass storage interface.");
        return (cError );
    }

    /* unstall the endpoints. order of unstalling is important */
    char msg2[8] = { 0x02, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };

    msg2[4] = this->bulkin_ep;
    dev->controller->sendUSBControlMsg(dev, 0, msg2);
    dev->endpoints[this->bulkout_ep].data_toggle = 0;

    msg2[4] = this->bulkout_ep;

    dev->controller->sendUSBControlMsg(dev, 0, msg2);
    dev->endpoints[this->bulkin_ep].data_toggle = 0;

    return (cOk );
}


/*****************************************************************************
 * Method: MassStorageSCSIUSBDeviceDriver::initialize()
 *
 * @description
 *
 *******************************************************************************/
ErrorT MassStorageSCSIUSBDeviceDriver::initialize() {
    /* try to initialize the device
     first check the endpoint information */
    bool bulkinep, bulkoutep = false;

    for (unint1 i = 1; i <= dev->numEndpoints; i++) {
        if (dev->endpoints[i].type == Bulk) {
            /* active it */
            if (dev->endpoints[i].direction == Out) {
                this->bulkout_ep = i;
                bulkoutep = true;

            } else {
                this->bulkin_ep = i;
                bulkinep = true;
            }

            dev->activateEndpoint(i);

        } else if (dev->endpoints[i].type == Interrupt) {
            int_ep = i;
            dev->endpoints[i].poll_frequency = 200;
        }
    }

    /* if (!(bulkinep & bulkoutep & !intep)) {*/
    /* mass storage device may have some interrupt end point .. but we are not using it*/
    if (!(bulkinep & bulkoutep)) {
        LOG(ARCH, ERROR, "MassStorageSCSIUSBDeviceDriver: probing failed..");
        /* deactivate eps again */
        return (cError );
    }

    lun = 1;
    dev->dev_priv = 0;

    /* try getting the number of logical units */
    char msg2[8] = { 0xa1, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00 };

    int error = dev->controller->sendUSBControlMsg(dev, 0, msg2, USB_DIR_IN, 1, reinterpret_cast<char*>(&lun));
    if (error < 0) {
        LOG(ARCH, ERROR, "MassStorageSCSIUSBDeviceDriver: reading logical unit number failed..");
        return (cError );
    }

    lun += 1;
    LOG(ARCH, DEBUG, "MassStorageSCSIUSBDeviceDriver: Device LUNs: %d", lun);

    this->myLUN = 0;
    memset(&this->scsi_info, 0, sizeof(InquiryData));

#if 0
    error = performTransaction(cbwTestUnitReady, reinterpret_cast<char*>(&this->scsi_info), 0x1);
    memdump((unint4) &this->scsi_info, 4);
    if (error < 0) {
        LOG(ARCH, ERROR, "MassStorageSCSIUSBDeviceDriver: testing unit ready failed..");
        return cError;
    }
#endif

    MassStorageSCSIUSBDeviceDriver *current = this;

    /* create additional logical unit representations */
    for (unint1 i = 1; i < lun; i++) {
        char* p_name    = new char[4];
        int num         = reinterpret_cast<int>(BlockDeviceDriver::freeBlockDeviceIDs->removeHead());
        p_name[0] = 's';
        p_name[1] = 'd';
        p_name[2] = ('0' + num);
        p_name[3] = 0;
        MassStorageSCSIUSBDeviceDriver *p_next = new MassStorageSCSIUSBDeviceDriver(this, i, p_name);
        /* link the created LUNs MSDs so we can delete them again */
        current->next = p_next;
        current = p_next;
    }

    /* try reading the inquiry info. some device may not provide this info however*/
    error = performTransaction(cbwInquiry, reinterpret_cast<char*>(&this->scsi_info), SCSI_INQUIRY_LEN);
    if (error < 0) {
        LOG(ARCH, WARN, "MassStorageSCSIUSBDeviceDriver: device inquiry failed. Ignoring LUN %d.", this->myLUN);
        return (error);
    }

    /* we only register if inquiry succeeds */
    theOS->getPartitionManager()->registerBlockDevice(this);
    theOS->getFileManager()->getDirectory("dev")->add(this);

    return (cOk );
}

/*****************************************************************************
 * Method: MassStorageSCSIUSBDeviceDriver::readBlock(unint4 blockNum, char* buffer, unint4 blocks)
 *
 * @description
 *
 *******************************************************************************/
ErrorT MassStorageSCSIUSBDeviceDriver::readBlock(unint4 blockNum, char* buffer, unint4 blocks) {
    // block size = 512
    // num blocks = (length / 512) +1;
    //int num_blocks = (length >> 9) +1;

    if (blocks > 256)
        return (cBlockDeviceTooManyBlocks );
    unint2 length = (unint2) (blocks << 9);

    /* allocate some thread stack space for write cbw*/
    char cbw[36];
    memcpy(cbw, cbwRead10, 36);

    // initialize read command
    // needs to be done this way for dma on physical address to work
    cbw[8]  = (unint1) (length & 0xff);
    cbw[9]  = (unint1) ((length & 0xff00) >> 8);
    cbw[10] = (unint1) ((length & 0xff0000) >> 16);
    cbw[11] = (unint1) ((length & 0xff000000) >> 24);   // CBW Data Transfer Length

    cbw[17] = (unint1) ((blockNum & 0xff000000) >> 24);
    cbw[18] = (unint1) ((blockNum & 0xff0000) >> 16);
    cbw[19] = (unint1) ((blockNum & 0xff00) >> 8);
    cbw[20] = (unint1) (blockNum & 0xff);
    cbw[23] = (unint1) blocks;  // maximum of 255 blocks

    int error = performTransaction(cbw, buffer, length);
    if (error < 0) {
        LOG(ARCH, ERROR, "MassStorageSCSIUSBDeviceDriver: reading device blocks failed %d. block: %d length: %d.", error, blockNum, length);
        return (cError );
    }

    return (cOk );
}

/*****************************************************************************
 * Method: MassStorageSCSIUSBDeviceDriver::writeBlock(unint4 blockNum, char* buffer, unint4 blocks)
 *
 * @description
 *
 *******************************************************************************/
ErrorT MassStorageSCSIUSBDeviceDriver::writeBlock(unint4 blockNum, char* buffer, unint4 blocks) {
    LOG(ARCH, DEBUG, "MassStorageSCSIUSBDeviceDriver: writing block %d. blocks: %d.", blockNum, blocks);

    /* allocate some thread stack space for write cbw*/
    char cbw[36];
    memcpy(cbw, cbwWrite10, 36);

    cbw[15] = SCSI_WRITE10;

    // block size = 512
    if (blocks > 256)
        return (cBlockDeviceTooManyBlocks );
    unint2 length = (unint2) (blocks << 9);

    // initialize read command
    // needs to be done this way for dma on physical address to work
    cbw[8]  = (unint1) (length & 0xff);
    cbw[9]  = (unint1) ((length & 0xff00) >> 8);
    cbw[10] = (unint1) ((length & 0xff0000) >> 16);
    cbw[11] = (unint1) ((length & 0xff000000) >> 24);             // CBW Data Transfer Length

    cbw[17] = (unint1) ((blockNum & 0xff000000) >> 24);
    cbw[18] = (unint1) ((blockNum & 0xff0000) >> 16);
    cbw[19] = (unint1) ((blockNum & 0xff00) >> 8);
    cbw[20] = (unint1) (blockNum & 0xff);
    cbw[23] = (unint1) blocks;

    int error = performTransaction(cbw, buffer, length);
    if (error < 0) {
        LOG(ARCH, ERROR, "MassStorageSCSIUSBDeviceDriver: writing device blocks failed %d. block: %d length: %d.", error, blockNum, length);
        return (cError );
    }

    return (cOk );
}

/*****************************************************************************
 * Method: MassStorageSCSIUSBDeviceDriver::handleInterrupt()
 *
 * @description
 *
 *******************************************************************************/
ErrorT MassStorageSCSIUSBDeviceDriver::handleInterrupt() {
    ErrorT ret = cError;
    // TODO: anything to do here?
    return (ret);
}

MassStorageSCSIUSBDeviceDriver::~MassStorageSCSIUSBDeviceDriver() {
    // remove all instances (all luns) from the partition mananger
    LOG(ARCH, INFO, "MassStorageSCSIUSBDeviceDriver: removing BlockDevice '%s', LUN: %d.", this->name, this->myLUN);

    // unregister this device
    theOS->getPartitionManager()->unregisterBlockDevice(this);

    // remove driver from dev directory
    theOS->getFileManager()->getDirectory("dev")->remove(this);

    if (this->next != 0)
        delete this->next;

    BlockDeviceDriver::freeBlockDeviceIDs->addHead(reinterpret_cast<ListItem*>(this->myLUN));
}

/*****************************************************************************
 * Method: MassStorageSCSIUSBDeviceDriverFactory::isDriverFor(USBDevice* dev)
 *
 * @description
 *  checks whether the given class,product device is supported by this driver
 *
 *******************************************************************************/
bool MassStorageSCSIUSBDeviceDriverFactory::isDriverFor(USBDevice* dev) {
    // we dont care for vendor id and product id as we are supporting generic SCSI MS Devices

    // if Mass Storage Device with SCSI command set and Bulk only transfer
    if ((dev->if_descr.bInterfaceClass == 8) && (dev->if_descr.bInterfaceSubClass == 6) && (dev->if_descr.bInterfaceProtocol == 0x50))
        return (true);

    return (false);
}

MassStorageSCSIUSBDeviceDriverFactory::MassStorageSCSIUSBDeviceDriverFactory(char* p_name) :
        USBDeviceDriverFactory(p_name) {
}
