/*
 * USBDeviceDriver.hh
 *
 *  Created on: 02.05.2013
 *     Copyright & Author: dbaldin
 */

#ifndef USBDEVICEDRIVER_HH_
#define USBDEVICEDRIVER_HH_

#include "filesystem/Resource.hh"

class USBDevice;

/*!
 * Base class for all usb device drivers.
 */
class USBDeviceDriver {
public:
    USBDeviceDriver() {
        dev_priv = 0;
    }


    virtual ~USBDeviceDriver() {
    }


    // some private data
    void* dev_priv;

    /*****************************************************************************
     * Method: initialize()
     *
     * @description
     *  initialize the driver
     *******************************************************************************/
    virtual ErrorT initialize() = 0;


    /*****************************************************************************
     * Method: handleInterrupt()
     *
     * @description
     * Interrupt handling routine for this device.
     * As USB interrupts are unspecific every USBDevice attached needs to be
     * checked by software. Thus, this method is called for every device
     * whenever a usb interrupt occurs. The driver needs
     * to check on its own whether the intterupt might be for it.
     * TODO: this could be enhanced by letting the EHCI Controller check
     * the receive buffer of the device.
     *******************************************************************************/
    virtual ErrorT handleInterrupt() = 0;
};

class USBDeviceDriverFactory: public Resource {
public:
    /*****************************************************************************
     * Method: USBDeviceDriverFactory(char* name)
     *
     * @description
     *
     *******************************************************************************/
    explicit USBDeviceDriverFactory(char* name);

    /*****************************************************************************
     * Method: isDriverFor(USBDevice* dev)
     *
     * @description
     *  checks whether the given class,product device is supported by this driver
     *******************************************************************************/
    virtual bool isDriverFor(USBDevice* dev) = 0;

    /*****************************************************************************
     * Method: getInstance(USBDevice* dev)
     *
     * @description
     *
     *******************************************************************************/
    virtual USBDeviceDriver* getInstance(USBDevice* dev) = 0;

    virtual ~USBDeviceDriverFactory();
};

#endif /* USBDEVICEDRIVER_HH_ */
