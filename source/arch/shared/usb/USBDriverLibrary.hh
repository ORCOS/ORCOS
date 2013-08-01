/*
 * USBDriverLibrary.hh
 *
 *  Created on: 02.05.2013
 *      Author: dbaldin
 */

#ifndef USBDRIVERLIBRARY_HH_
#define USBDRIVERLIBRARY_HH_

#include "filesystem/Directory.hh"
#include "USBDeviceDriverFactory.hh"


class USBDriverLibrary: public Directory {
public:
	USBDriverLibrary();

	USBDeviceDriverFactory* getDriverFor(USBDevice *dev);

	virtual ~USBDriverLibrary();
};

#endif /* USBDRIVERLIBRARY_HH_ */
