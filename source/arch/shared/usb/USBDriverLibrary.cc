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

