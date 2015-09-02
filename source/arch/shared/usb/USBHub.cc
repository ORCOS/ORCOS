/*
 * USBHub.cc
 *
 *  Created on: 27.08.2015
 *      Author: Daniel
 */

#include "USBHub.hh"
#include "usb.h"
#include "USBHostController.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

/**********************************************************************
 *
 *                    USB HUB Implementation
 *
 **********************************************************************/

static char recv_buf[256] __attribute__((aligned(4))) ATTR_CACHE_INHIBIT;

/*****************************************************************************
 * Method: USBHub::enumerate()
 *
 * @description
 *  Issues a USB HUB enumeration, possibly identifying new usb
 *  devices.
 *  Hub Enumeration Method. Starts the Enumeration Process on a newly connected
 *  Hub. State changes are check (20 ms delay) and devices activated if found.
 *******************************************************************************/
ErrorT USBHub::enumerate() {
    // be sure we are comunicating to a hub.
    if (dev->dev_descr.bDeviceClass != 0x9)
        return (cError );

    // first get the hub descriptor to see how many ports we need to activate and check for connected device
    memset(&recv_buf, 0, 0x40);

    char msg[8] = USB_GETHUB_DESCRIPTOR();
    int4 error = controller->sendUSBControlMsg(dev, 0, msg, USB_DIR_IN, 0x40, recv_buf);
    if (error < 0) {
        return (cError );
    }

    memcpy(&this->hub_descr, &recv_buf, sizeof(HubDescriptor));

    LOG(ARCH, INFO, "USBHub: Hub Ports: %d, Characteristics: %x ", hub_descr.bnrPorts, hub_descr.wHubCharacteristics);

    LOG(ARCH, DEBUG, "USBHub: Powering Ports");
    unint1 port_power_control = (hub_descr.wHubCharacteristics & 0x3);

    // power on ports..
    for (unint1 i = 1; i <= hub_descr.bnrPorts; i++) {
        // individual power control... power all ports
        char msg2[8] = USB_SETPORT_FEATURE(PORT_POWER);
        msg2[4] = i;
        error = controller->sendUSBControlMsg(dev, 0, msg2, USB_DIR_IN, 0, recv_buf);
        if (error < 0) {
            LOG(ARCH, ERROR, "USBHub: Hub Port %d PowerOn failed.. ", i);
        } else {
            // we may stop if port power is ganged as all other ports are now also powered
            if (port_power_control == 0)
                break;
        }
    }

    //OUTW(controller->operational_register_base + USBSTS_OFFSET, 0x3f);

    // activate interrupts
    dev->activateEndpoint(1);

    // try to directly enumerate the hub as it may already have connected devices
    // this is a performance optimization as we do not need to wait for the actual
    // interrupt to happen
    // give the interrupt request some time to occur
    volatile unint4 timeout = 50;
    while (timeout) {
        timeout--;
        kwait(1);
    }

    if (dev->endpoints[1].recv_buffer[0] != 0) {
        this->handleStatusChange();
        // activate interrupt transfer again so we detect further changes
        dev->activateEndpoint(1);
    }

    return (cOk );
}

static char u_recv_buf[20] ATTR_CACHE_INHIBIT;
char port_status[4] ATTR_CACHE_INHIBIT;

/*****************************************************************************
 * Method: USBHub::handleStatusChange()
 *
 * @description
 *  Handles USB HUB status changes.
 *******************************************************************************/
void USBHub::handleStatusChange() {
    ErrorT error;

    for (unint1 i = 1; i <= hub_descr.bnrPorts; i++) {
        if ((dev->endpoints[1].recv_buffer[0] & (1 << i)) != 0) {
            // get status of port
            char msg4[8] = USB_GETPORT_STATUS();
            msg4[4] = i;
            port_status[0] = 0;
            port_status[1] = 0;
            port_status[2] = 0;
            port_status[3] = 0;

            error = controller->sendUSBControlMsg(dev, 0, msg4, USB_DIR_IN, 4, port_status);

            // check reason of status change
            if (port_status[2] & 1) {
                LOG(ARCH, DEBUG, "USBHub: Port %d Connection Status changed..", i);

                // clear the status change bit of this port
                char msg1[8] = USB_CLEARPORT_FEATURE(C_PORT_CONNECTION);
                msg1[4] = i;
                error = controller->sendUSBControlMsg(dev, 0, msg1);
                if (error < 0) {
                    LOG(ARCH, ERROR, "USBHub: Clearing Port Feature failed.. ");
                    return;
                }

                kwait(2);

                if (port_status[0] & 1) {
                    LOG(ARCH, INFO, "USBHub: Device attached on port %d..", i);

                    // device on this hub port!
                    // reset port and enumerate the device
                    char msg3[8] = USB_SETPORT_FEATURE(PORT_RESET);
                    msg3[4] = i;

                    // wait until port is enabled
                    error = controller->sendUSBControlMsg(dev, 0, msg3);
                    if (error < 0) {
                        LOG(ARCH, ERROR, "USBHub: Resetting hub port %d failed.. ", i);
                        return;
                    }

                    volatile unint1 enabled = 0;
                    volatile int4 timeout = 100;

                    while (enabled == 0) {
                        msg4[4] = i;
                        memset(&u_recv_buf, 0, 20);
                        error = controller->sendUSBControlMsg(dev, 0, msg4, USB_DIR_IN, 20, u_recv_buf);
                        enabled = u_recv_buf[0] & (1 << 1);
                        timeout--;

                        if (timeout < 0) {
                            LOG(ARCH, ERROR, "USBHub: Enabling port failed.. ");
                            return;
                        }
                    }

                    char msg5[8] = USB_CLEARPORT_FEATURE(C_PORT_RESET);
                    msg5[4] = i;
                    error = controller->sendUSBControlMsg(dev, 0, msg5);
                    if (error < 0) {
                        LOG(ARCH, ERROR, "USBHub: Clearing Port Feature failed.. ");
                        return;
                    }

                    LOG(ARCH, INFO, "USBHub: Port enabled.. Enumerating ");

                    unint1 speed = 2;
                    if (u_recv_buf[1] & (1 << 1)) {
                        speed = 1;
                        LOG(ARCH, INFO, "USBHub: Low-Speed Device attached.");
                    } else if (u_recv_buf[1] & (1 << 2)) {
                        speed = 2;
                        LOG(ARCH, INFO, "USBHub: High-Speed Device attached.");
                    } else {
                        speed = 0;
                        LOG(ARCH, INFO, "USBHub: Full-Speed Device attached.");
                    }

                    // check if there is a device present which is just resetting
                    if (this->portDevices[i] != 0) {
                        LOG(ARCH, INFO, "USBHub: Deactivating Device..");

                        // did we create a device on this port?
                        if (this->portDevices[i] != 0) {
                            this->portDevices[i]->deactivate();

                            // remove from registered devices list
                            controller->unregisterDevice(portDevices[i]->driver);

                            delete portDevices[i];
                            this->portDevices[i] = 0;
                        }
                    }

                    USBDevice *newdev = controller->createDevice(dev, i, speed);  // deletion 10 lines down
                    this->portDevices[i] = newdev;

                    // try to enumerate the port
                    // this initializes all devices
                    controller->enumerateDevice(newdev);
                } else {
                    LOG(ARCH, INFO, "USBHub: Device detached..", i);

                    if (this->portDevices[i] != 0) {
                        LOG(ARCH, INFO, "USBHub: Deactivating Device..");

                        // did we create a device on this port?
                        if (this->portDevices[i] != 0) {
                            this->portDevices[i]->deactivate();

                            // remove from registered devices list
                            controller->unregisterDevice(portDevices[i]->driver);

                            delete portDevices[i];
                            this->portDevices[i] = 0;
                        }
                        return;
                    }
                }
            }  // PORT_CONNECTION BIT

            if (port_status[2] & (1 << 2)) {
                char msg5[8] = USB_CLEARPORT_FEATURE(C_PORT_SUSPEND);
                msg5[4] = i;
                error = controller->sendUSBControlMsg(dev, 0, msg5);
                if (error < 0) {
                    LOG(ARCH, ERROR, "USBHub: Clearing Port Feature failed.. ");
                    return;
                }
            }  // PORT_SUSPEND

            if (port_status[2] & (1 << 4)) {
                char msg5[8] = USB_CLEARPORT_FEATURE(C_PORT_RESET);
                msg5[4] = i;
                error = controller->sendUSBControlMsg(dev, 0, msg5);
                if (error < 0) {
                    LOG(ARCH, ERROR, "USBHub: Clearing Port Feature failed.. ");
                    return;
                }
            }
            if (port_status[2] & (1 << 3)) {
                // unknown port status change
                LOG(ARCH, ERROR, "USBHub: Port Overpower!.. ");
                // TODO: handle by clearing stall
                //while(1);
            }
            if (port_status[2] & (1 << 1)) {
                // unknown port status change
                LOG(ARCH, ERROR, "USBHub: Port Disabled!.. ");
                // TODO: handle by clearing stall
                //while(1);
            }
        }
    }
}

USBHub::USBHub(USBDevice *p_dev, USB_Host_Controller *p_controller) :
        USBDeviceDriver() {
    this->dev = p_dev;
    this->controller = p_controller;
    for (int i = 0; i < 6; i++) {
        this->portDevices[i] = 0;
    }
}

USBHub::~USBHub() {
    /* delete all devices as they are no longer there */
    for (int i = 0; i < 6; i++) {
        if (this->portDevices[i] != 0)
            delete this->portDevices[i];
    }
}

/*****************************************************************************
 * Method: USBHub::handleInterrupt()
 *
 * @description
 *  Handles IRQs generated by the USB HUB
 *******************************************************************************/
ErrorT USBHub::handleInterrupt() {
    // TODO: this is host controller specific!

    //QH* qh = dev->endpoints[1].queue_head;

    //if ((QT_TOKEN_GET_STATUS(qh->qh_overlay.qt_token) != 0x80)) {
     if (dev->getEndpointTransferState(1) == EP_TRANSFER_FINISHED) {
        LOG(ARCH, DEBUG, "USB_Hub: Interrupt Request finished, status: %x", dev->endpoints[1].recv_buffer[0]);

        // get status of port
        this->handleStatusChange();
        dev->endpoints[1].recv_buffer[0] = 0;

        // reactivate the interrupt transfer so we get interrupted again
        dev->activateEndpoint(1);

        return (cOk);
    }

    return (cError);
}
