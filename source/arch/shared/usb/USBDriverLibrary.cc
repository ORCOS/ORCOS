/*
 * USBDriverLibrary.cc
 *
 *  Created on: 02.05.2013
 *    Copyright &  Author: dbaldin
 */

#include "USBDriverLibrary.hh"
#include "kernel/Kernel.hh"

extern Kernel *theOS;

USBDriverLibrary::USBDriverLibrary() :
        Directory("usb") {
    theOS->getFileManager()->registerResource(this);


    /* TODO: auto generate the usb factory construction using SCL  */
  #if HAS_USBDriver_SMSC95xxCfd
      /* Add support for smsc95xx ethernet over USB devices */
      new SMSC95xxUSBDeviceDriverFactory("smsc95xx");
  #endif

  #if HAS_USBDriver_MassStorageCfd
      /* Add support for USB SCSI Bulk only Mass Storage Devices */
      new MassStorageSCSIUSBDeviceDriverFactory("msd_scsi_bot");
  #endif
}

/*****************************************************************************
 * Method: USBDriverLibrary::getDriverFor(USBDevice *dev)
 *
 * @description
 *
 *******************************************************************************/
USBDeviceDriverFactory* USBDriverLibrary::getDriverFor(USBDevice *dev) {
    LinkedListItem* dbitem = this->getContent()->getHead();

    /* simple iteration over all listed drivers */
    while (dbitem != 0) {
        USBDeviceDriverFactory* driver = static_cast<USBDeviceDriverFactory*>(dbitem->getData());
        if (driver->isDriverFor(dev))
            return (driver);

        dbitem = dbitem->getSucc();
    }

    return (0);
}

USBDriverLibrary::~USBDriverLibrary() {
    theOS->getFileManager()->unregisterResource(this);
}

