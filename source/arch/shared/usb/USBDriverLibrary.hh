/*
 * USBDriverLibrary.hh
 *
 *  Created on: 02.05.2013
 *    Copyright &  Author: dbaldin
 */

#ifndef USBDRIVERLIBRARY_HH_
#define USBDRIVERLIBRARY_HH_

#include "filesystem/Directory.hh"
#include "USBDeviceDriverFactory.hh"


class USBDriverLibrary: public Directory {
public:
    USBDriverLibrary();

    /*****************************************************************************
     * Method: getDriverFor(USBDevice *dev)
     *
     * @description
     *
     *******************************************************************************/
    USBDeviceDriverFactory* getDriverFor(USBDevice *dev);

    ~USBDriverLibrary();
};

#endif /* USBDRIVERLIBRARY_HH_ */
