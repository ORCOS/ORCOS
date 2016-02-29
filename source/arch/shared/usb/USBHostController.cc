/*
 * USBHostController.cc
 *
 *  Created on: 27.08.2015
 *      Author & Copyright: Daniel Baldin
 */

#include "USBHostController.hh"
#include "USBHub.hh"
#include "kernel/Kernel.hh"
#include "inc/memtools.hh"

extern Kernel* theOS;
extern Kernel_ThreadCfdCl* pCurrentRunningThread;

const char* bDeviceClassStr[16] = {
        "Device",
        "Audio",
        "Communications and CDC Control",
        "HID (Human Interface Device)",
        "Unknown",
        "Physical",
        "Image",
        "Printer",
        "Mass Storage",
        "Hub",
        "CDC-Data",
        "Smart Card",
        "Content Security",
        "Video",
        "Personal Healthcare",
        "Audio/Video Devices" };


const char* bEndpointDirectionStr[2] = { "Out", "In" };

const char* bmTransferTypeStr[4] = { "Control", "Isochronous", "Bulk", "Interrupt" };


static char recv_buf[256] __attribute__((aligned(4))) ATTR_CACHE_INHIBIT;

/*****************************************************************************
 * Method: USB_EHCI_Host_Controller::enumerateDevice(USBDevice *dev)
 *
 * @description
 *  Tries to enumerate a given device in order to load a driver for it
 *******************************************************************************/
ErrorT USB_Host_Controller::enumerateDevice(USBDevice *dev) {
    char msg[8] = USB_DESCRIPTOR_REQUEST(DEVICE_DESCRIPTOR);
    int4 ret_len = this->sendUSBControlMsg(dev, 0, msg, USB_DIR_IN | USB_IGNORE_ERROR, 0x40, recv_buf);

    if (ret_len >= 0) {
        unint1 bdevLength = recv_buf[0];
        LOG(ARCH, TRACE, "USB_Host_Controller: Device Descriptor Length %d:", bdevLength);

        if (bdevLength > 0 && bdevLength <= sizeof(DeviceDescriptor)) {
            /* copy received device descriptor so we can reference it later on */
            memcpy(&dev->dev_descr, &recv_buf, bdevLength);
            /* Analyze device descriptor */

            LOG(ARCH, INFO, "USB_Host_Controller: Device at Port %d:", dev->port);
            LOG(ARCH, INFO, "USB_Host_Controller:  USB %d.%d Device", dev->dev_descr.v_usbhigh, dev->dev_descr.v_usblow);
            if (dev->dev_descr.bDeviceClass > 15)
                dev->dev_descr.bDeviceClass = 4;
            LOG(ARCH, INFO, "USB_Host_Controller:  Class: %x (%s)", dev->dev_descr.bDeviceClass, bDeviceClassStr[dev->dev_descr.bDeviceClass]);
            LOG(ARCH, INFO, "USB_Host_Controller:  SubClass: %x Proto: %x", dev->dev_descr.bDeviceSubClass, dev->dev_descr.bDeviceProtocol);
            LOG(ARCH, INFO, "USB_Host_Controller:  Vendor: %4x - Product: %4x", dev->dev_descr.idVendor, dev->dev_descr.idProduct);
            LOG(ARCH, INFO, "USB_Host_Controller:  Max Packet Size: %d", dev->dev_descr.bMaxPacketSize);
        } else {
            return (cError);
        }
    } else {
        LOG(ARCH, ERROR, "USB_Host_Controller: Getting Device Descriptor failed.");
        return (cError);
        // failed .. handle error
    }

    /* reset the port again
     some device only react on the following messages after resetting the port two times */
    if (dev->parent == 0) {
        ErrorT result = resetPort(dev->port);
        if (isError(result)) {
            LOG(ARCH, ERROR, "USB_Host_Controller: could not reset Port %d. Error %d", dev->port, result);
            return (result);
        }
    } else {
        /* parent must be a hub */
        USBHub* hub = dev->parent;
        hub->resetPort(dev->port);
    }

    // give some time for reset recovery of device
    kwait(10);

    int nextId = USBDevice::freeDeviceIDs.getNextID();
    if (nextId == -1) {
        LOG(ARCH, ERROR, "USB_Host_Controller: No free USB Device ID left!");
        return (cError);
    }

    unint1 next_device_addr = (unint1) nextId;

    LOG(ARCH, INFO, "USB_Host_Controller: Setting Device Address: %d", next_device_addr);

    dev->setMaxPacketSize(0, dev->dev_descr.bMaxPacketSize);
    dev->endpoints[0].interrupt_receive_size = dev->dev_descr.bMaxPacketSize;

    // set the device address!
    int4 error = dev->setAddress(next_device_addr);
    if (error != cOk) {
        LOG(ARCH, ERROR, "USB_Host_Controller: Setting Device Address failed");
        return (cError );
    }

    // give some time to the device to change its address
    kwait(10);

    if (ret_len < 18) {
        /* second device descriptor request */
        ret_len = this->sendUSBControlMsg(dev, 0, msg, USB_DIR_IN , 0x40, recv_buf);
        if (ret_len >= 0) {
           unint1 bdevLength = 18;
           LOG(ARCH, TRACE, "USB_Host_Controller: Device Descriptor Length %d:", bdevLength);

           if (bdevLength > 0 && bdevLength <= sizeof(DeviceDescriptor)) {
               /* copy received device descriptor so we can reference it later on */
               memcpy(&dev->dev_descr, &recv_buf, bdevLength);
           } else {
               return (cError);
           }
       } else {
           LOG(ARCH, ERROR, "USB_Host_Controller: Getting Device Descriptor failed.");
           return (cError);
       }
   }

    memset(&recv_buf, 0, 0xff);
    char msg3[8] = USB_DESCRIPTOR_REQUEST(CONFIGURATION_DESCRIPTOR);
    error = this->sendUSBControlMsg(dev, 0, msg3, USB_DIR_IN, 0xff, recv_buf);
    if (error < 0)
        return (cError );

    ConfigurationDescriptor* cConf = reinterpret_cast<ConfigurationDescriptor*>(&recv_buf);
    unint4 total_length = cConf->wtotalLength;

    //int firstConfig = cConf->bConfigurationValue;
    LOG(ARCH, INFO, "USB_Host_Controller: Configuration %d [%x]:", cConf->bConfigurationValue, cConf->bmAttributes);
    LOG(ARCH, INFO, "USB_Host_Controller:   NumInterfaces: %d maxPower: %d mA", cConf->bNumInterfaces, cConf->bMaxPower * 2);
    InterfaceDescriptor* iDescr = reinterpret_cast<InterfaceDescriptor*>(cConf + 1);

    unint4 bytes_read = sizeof(ConfigurationDescriptor);

    // store interface descriptor for driver
    memcpy(&dev->if_descr, iDescr, sizeof(InterfaceDescriptor));

    LOG(ARCH, INFO, "USB_Host_Controller: Interface %d :", dev->if_descr.bInterfaceNumber);
    LOG(ARCH, INFO, "USB_Host_Controller:  NumEndpoints: %d", dev->if_descr.bNumEndpoints);
    LOG(ARCH, INFO, "USB_Host_Controller:  Class: %x", dev->if_descr.bInterfaceClass);
    LOG(ARCH, INFO, "USB_Host_Controller:  Subclass: %x Protocol: %x", dev->if_descr.bInterfaceSubClass, dev->if_descr.bInterfaceProtocol);

    bytes_read += iDescr->bLength;

    EndpointDescriptor *descr = reinterpret_cast<EndpointDescriptor*>(iDescr + 1);

    // some devices place some custom descriptors in between? stupid devices
    while (descr->btype != 5 && bytes_read < total_length) {
        LOG(ARCH, WARN, "USB_Host_Controller: Ignoring unknown descriptor type: %x ", descr->btype);
        bytes_read += descr->bLength;
        descr = reinterpret_cast<EndpointDescriptor*> (((unint4) descr) + descr->bLength);
    }

    dev->numEndpoints = iDescr->bNumEndpoints;

    // get all endpoints
    for (int i = 0; i < iDescr->bNumEndpoints; i++) {
        dev->endpoints[i + 1].interrupt_receive_size = 0;

        // saftey check.. some usb devices might not send enough data ..
        if (bytes_read > total_length) {
            LOG(ARCH, WARN, "USB_Host_Controller: Insufficient endpoint descriptor length: bytes_read: %d, total: %d ", bytes_read, total_length);
            break;
        }

        memcpy(&dev->endpoints[i + 1].descriptor, descr, sizeof(EndpointDescriptor));

        if (descr->btype != 0x5) {
            LOG(ARCH, ERROR, "USB_Host_Controller: Endpoint Descriptor failure.. bType != 0x5: %x", i + 1, descr->btype);
            memdump(&dev->endpoints[i + 1].descriptor, sizeof(EndpointDescriptor));
            return (cError );
        }

        /* set transfer direction */
        if (((descr->bEndpointAddress & 0x80) >> 7) == 0)
            dev->endpoints[i + 1].direction = Out;
        else
            dev->endpoints[i + 1].direction = In;

        if (descr->bLength > 5)
            dev->endpoints[i + 1].interrupt_receive_size = descr->wMaxPacketSize;

        dev->endpoints[i + 1].address = descr->bEndpointAddress & 0xf;

        if (dev->endpoints[i + 1].interrupt_receive_size == 0x0)
            dev->endpoints[i + 1].interrupt_receive_size = 40;

        switch (descr->bmAttributes & 0x3) {
            case 0:
                dev->endpoints[i + 1].type = Control;
                break;
            case 1:
                dev->endpoints[i + 1].type = Isochronous;
                break;
            case 2:
                dev->endpoints[i + 1].type = Bulk;
                break;
            case 3: {
                dev->endpoints[i + 1].type = Interrupt;
                dev->endpoints[i + 1].poll_frequency = descr->bInterval;
                break;
            }
        }

        LOG(ARCH, INFO, "USB_Host_Controller: Endpoint Adress: %d, Type: %s, Dir: %s, Transfer Size: %d",
               dev->endpoints[i + 1].address,
               bmTransferTypeStr[dev->endpoints[i + 1].type],
               bEndpointDirectionStr[dev->endpoints[i + 1].direction],
               dev->endpoints[i + 1].interrupt_receive_size);

        bytes_read += descr->bLength;
        descr = reinterpret_cast<EndpointDescriptor*> (((unint4) descr) + descr->bLength);
    }

    // Done Enumeration. Now set the configuration

    if (dev->dev_descr.idVendor != 0x58f) {
        char msg4[8] = USB_SETCONFIGURATION_REQUEST(1);
        error = this->sendUSBControlMsg(dev, 0, msg4);

        if (error < 0) {
            LOG(ARCH, ERROR, "USB_Host_Controller: Setting Configuration failed..");
            return (cError );
        }
    }

    // Setting the interface is not supported by all devices
    // As we are always using the first default interface we omit
    // setting this explicitly
    /*
     unint1 msg8[8] = USB_SETINTERFACE_REQUEST(iDescr->bInterfaceNumber);
     error = this->sendUSBControlMsg(dev,0,(unint1*) &msg8);

     if (error < 0) {
     LOG(ARCH,ERROR,(ARCH,ERROR,"USB_EHCI_Host_Controller: Setting Interface failed.."));
     return cError;
     }


     memset(&recv_buf,0,0x40);
     unint1 msg5[8] = USB_GETDEVICE_STATUS();
     error = this->sendUSBControlMsg(dev,0,(unint1*) &msg5,USB_DIR_IN,8,(unint1*) &recv_buf);
     */

    // if this device is a hub
    if (dev->dev_descr.bDeviceClass == 0x9) {
        LOG(ARCH, INFO, "USB_Host_Controller: Hub attached on port %d", dev->port);
        // set the configuration directly and perform a hub enumeration
        dev->endpoints[1].type = Interrupt;
        // overwrite poll frequency.. we are polling hubs less often
        dev->endpoints[1].poll_frequency = 27;  // 27 ms
        dev->activateEndpoint(1);    // activate it.. will generate interrupt queue heads

        LOG(ARCH, DEBUG, "USB_Host_Controller: Enumerating Hub..");
        // create new DeviceDriver for this hub
        USBHub *hub = new USBHub(dev, this);
        dev->driver = hub;
        // register it as it can produce interrupts
        registerDevice(hub);
        // enumerate it
        hub->enumerate();

    } else {
        // load the device driver if existend
        LOG(ARCH, DEBUG, "USB_Host_Controller: Trying to load driver for new device");

        // driver needs to activate the endpoints and do the device specific stuff
        // like registering at the OS as a char/comm or whatever device

        // lets try to find a driver for this usbdevice
        USBDeviceDriverFactory* driverfactory = theOS->getUSBDriverLibrary()->getDriverFor(dev);

        if (driverfactory != 0) {
            // get a driver instance and initialize it
            USBDeviceDriver *driver = driverfactory->getInstance(dev);
            // keep track of driver so we can delete it on device detach
            dev->driver = driver;
            registerDevice(driver);
            driver->initialize();
        } else {
            LOG(ARCH, WARN, "USB_Host_Controller: No driver found for device...");
            return (cError);
        }

        return (cOk);
    }

    return (cOk);
}


/*****************************************************************************
 * Method: USB_EHCI_Host_Controller::registerDevice(USBDeviceDriver* drv)()
 *
 * @description
 *  Registers a new USB Device Driver that has been
 *  instantiated for a new USB Device
 *******************************************************************************/
void USB_Host_Controller::registerDevice(USBDeviceDriver* drv) {
    if (drv == 0)
        return;

    for (int i = 0; i < 10; i++) {
        if (registered_devices[i] == 0) {
            registered_devices[i] = drv;
            return;
        }
    }
}

/*****************************************************************************
 * Method: USB_EHCI_Host_Controller::unregisterDevice(USBDeviceDriver* drv)()
 *
 * @description
 *  Unregisters a USB Device Driver
 *******************************************************************************/
void USB_Host_Controller::unregisterDevice(USBDeviceDriver* drv) {
    if (drv == 0)
        return;

    for (int i = 0; i < 10; i++) {
        if (registered_devices[i] == drv) {
            registered_devices[i] = 0;
            return;
        }
    }
}

