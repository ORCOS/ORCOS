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
 * Base class for all USB device drivers.
 */
class USBDeviceDriver {
public:
    USBDeviceDriver() {
        dev_priv = 0;
    }


    virtual ~USBDeviceDriver() {
    }


    /*****************************************************************************
     * dev_priv
     *
     * @description
     *  Private data to be used by the driver instance
     *******************************************************************************/
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


#define USB_VENDOR_PRODUCT(x, y) { .vendorId=x, .productId=y}

#define USB_DRIVER(drivername) \
                    TUSBDriver _usbdriver_##drivername __attribute__((externally_visible)) =

#define REGISTER_DRIVER(name)  \
            extern TUSBDriver* USBDriverList; \
            void __attribute__((used, constructor)) __addDriver_##name(void) \
                { _usbdriver_##name.next = USBDriverList; USBDriverList = (TUSBDriver*) &_usbdriver_##name; }

//                volatile int __init_##name __attribute__((used)) = __addDriver_##name();


#define AUTHOR(x)    author : x
#define NAME(x)      name : x
#define VERSION(x)   version : x
#define ID_TABLE(x)  id_table : (TUSB_Product_Id*) x
#define USB_CLASS(x,y,z)  usb_class : { interfaceClass : x, interfaceSubClass: y, interfaceProtocol : z}
#define USB_CLASS_UNSPECIFIED  USB_CLASS(0,0,0)

#define USB_DRIVER_CLASS(x) device_creator : &x::getInstance

typedef USBDeviceDriver* (*TPtr_USB_Device_Creation)(USBDevice* dev);

typedef struct {
    unint2 vendorId;
    unint2 productId;
} TUSB_Product_Id;

typedef struct {
    unint1 interfaceClass;
    unint1 interfaceSubClass;
    unint1 interfaceProtocol;
} TUSB_ClassType;

typedef struct TUSBDriver {
    TUSBDriver*               next;
    char*                     name;
    char*                     author;
    char*                     version;
    TUSB_Product_Id*          id_table;
    TUSB_ClassType            usb_class;
    TPtr_USB_Device_Creation  device_creator;
} TUSBDriver;


/*!
 * Factory class every USB Driver has to provide to create new instances
 * of the driver for attached USB devices.
 */
class USBDeviceDriverFactory: public Resource {
private:
    TUSBDriver* driverInfo;

public:
    /*****************************************************************************
     * Method: USBDeviceDriverFactory(char* name)
     *
     * @description
     *
     *******************************************************************************/
    explicit USBDeviceDriverFactory(TUSBDriver* driverInfo);

    /*****************************************************************************
     * Method: isDriverFor(USBDevice* dev)
     *
     * @description
     *  checks whether the given class,product device is supported by this driver
     *******************************************************************************/
    bool isDriverFor(USBDevice* dev);

    /*****************************************************************************
     * Method: getInstance(USBDevice* dev)
     *
     * @description
     *
     *******************************************************************************/
    USBDeviceDriver* getInstance(USBDevice* dev);

    ~USBDeviceDriverFactory();
};

#endif /* USBDEVICEDRIVER_HH_ */
