/*
 * USBDriverLibrary.cc
 *
 *  Created on: 02.05.2013
 *      Author: dbaldin
 */

#include "USBDriverLibrary.hh"
#include "kernel/Kernel.hh"

extern Kernel *theOS;

USBDriverLibrary::USBDriverLibrary()
	: Directory("usb")
{
	theOS->getFileManager()->registerResource(this);
}

USBDeviceDriverFactory* USBDriverLibrary::getDriverFor(USBDevice *dev)
{
	 LinkedListDatabaseItem* dbitem = this->getContent()->getHead();

	 /* simple iteration over all listed drivers */
	 while (dbitem != 0) {
		 USBDeviceDriverFactory* driver = (USBDeviceDriverFactory*) dbitem->getData();
		 if (driver->isDriverFor(dev)) return (driver);

		 dbitem = dbitem->getSucc();
	 }

	 return (0);
}

USBDriverLibrary::~USBDriverLibrary() {
	theOS->getFileManager()->unregisterResource(this);
}

