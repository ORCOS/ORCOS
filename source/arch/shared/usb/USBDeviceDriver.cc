/*
 * USBDeviceDriver.cc
 *
 *  Created on: 02.05.2013
 *    Copyright &  Author: dbaldin
 */

#include "USBDriverLibrary.hh"
#include "kernel/Kernel.hh"
#include "USBDeviceDriver.hh"

extern Kernel *theOS;

USBDeviceDriverFactory::USBDeviceDriverFactory(TUSBDriver* driverInfo) :
        Resource(cUSBDriver, false, driverInfo->name),
        driverInfo(driverInfo) {
    /* register ourself at the USBDriverLibrary */
    Directory* usbdir = theOS->getFileManager()->getDirectory("/usb/");
    if (usbdir != 0) {
        usbdir->add(this);
    }
}

USBDeviceDriverFactory::~USBDeviceDriverFactory() {
    Directory* usbdir = theOS->getFileManager()->getDirectory("/usb/");
    if (usbdir != 0) {
        usbdir->remove(this);
    }
}

bool USBDeviceDriverFactory::isDriverFor(USBDevice* dev) {
    TUSB_Product_Id* id = driverInfo->id_table;
    if (id != 0)
    {
        while (id->vendorId) {
            if (dev->dev_descr.idVendor == id->vendorId && dev->dev_descr.idProduct == id->productId) {
                return (true);
            }
            id++;
        }
    } else {
        // no specific vendors.. check class type
        if ((dev->if_descr.bInterfaceClass == driverInfo->usb_class.interfaceClass) &&
            (dev->if_descr.bInterfaceSubClass == driverInfo->usb_class.interfaceSubClass) &&
            (dev->if_descr.bInterfaceProtocol == driverInfo->usb_class.interfaceProtocol)) {
            return (true);
        }
    }
    return (false);
}

USBDeviceDriver* USBDeviceDriverFactory::getInstance(USBDevice* dev) {
    return (driverInfo->device_creator(dev));
}
