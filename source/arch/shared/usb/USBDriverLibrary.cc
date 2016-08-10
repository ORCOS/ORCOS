/*
 * USBDriverLibrary.cc
 *
 *  Created on: 02.05.2013
 *    Copyright &  Author: dbaldin
 */

#include "USBDriverLibrary.hh"
#include "kernel/Kernel.hh"

extern Kernel *theOS;
TUSBDriver* USBDriverList __attribute__((externally_visible));

USBDriverLibrary::USBDriverLibrary() :
        Directory("usb") {
    theOS->getFileManager()->registerResource(this);

    TUSBDriver* usbDriver = USBDriverList;
    while (usbDriver != 0)
    {
        LOG(ARCH, INFO, "USB Driver '%s', Version %s", usbDriver->name, usbDriver->version);
        new USBDeviceDriverFactory(usbDriver);
        usbDriver = usbDriver->next;
    }
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

