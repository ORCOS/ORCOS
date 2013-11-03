/*
 * USBDeviceDriver.cc
 *
 *  Created on: 02.05.2013
 *      Author: dbaldin
 */

#include "USBDeviceDriverFactory.hh"
#include "USBDriverLibrary.hh"
#include "kernel/Kernel.hh"

extern Kernel *theOS;

USBDeviceDriverFactory::USBDeviceDriverFactory(char* p_name)
: Resource(cUSBDriver,false,p_name)
{
	// register ourself at the USBDriverLibrary
	Directory* usbdir = theOS->getFileManager()->getDirectory("/usb/");
	if (usbdir != 0) {
		usbdir->add(this);
	}
}

USBDeviceDriverFactory::~USBDeviceDriverFactory() {
	// TODO Auto-generated destructor stub
}

