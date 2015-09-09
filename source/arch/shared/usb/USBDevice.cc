/*
 * USBDevice.cc
 *
 *  Created on: 27.08.2015
 *      Author: Daniel
 */

#include "USBDevice.hh"
#include "usb.h"
#include "kernel/Kernel.hh"
#include "inc/memtools.hh"

extern Kernel* theOS;

/**********************************************************************
 *
 *                    USB Device Implementation
 *
 **********************************************************************/

ArrayList* USBDevice::freeDeviceIDs;

/*****************************************************************************
 * Method: USBDevice::setAddress(unint1 addr)
 *
 * @description
 *  sets the address of the usb device and updates all queue heads for it
 *
 * @params
 *  addr       The new address for the device
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT USBDevice::setAddress(unint1 u_addr) {
    // got device descriptor ! set address
    this->addr   = u_addr;
    char msg2[8] = USB_SETADDRESS_REQUEST((unint1)u_addr);
    int error    = controller->sendUSBControlMsg(this, 0, msg2);

    // keep communicating with address 0 until we set the configuration and ask for the status!
    if (error < 0) {
        LOG(ARCH, ERROR, "USBDevice: Setting Device Address failed..");
        return (cError);
    }

    return (cOk );
}


USBDevice::USBDevice(USB_Host_Controller *p_controller, USBHub *p_parent, unint1 u_port, unint1 u_speed)
    : Resource(cGenericDevice, true, "USB_Device") {
    this->controller    = p_controller;
    this->parent        = p_parent;
    this->port          = u_port;
    this->dev_priv      = 0;
    this->numEndpoints  = 0;
    this->driver        = 0;
    this->speed         = u_speed;
    /* new devices need can only be access by control port 0 first
       an address needs to be set after successfull configuration */
   addr = 0;

   for (int i = 0; i < 4; i++) {
       endpoints[i].device_priv  = 0;
       endpoints[i].device_priv2 = 0;
       endpoints[i].data_toggle  = 0;
       endpoints[i].recv_buffer  = 0;
       endpoints[i].type = UnknownTT;
       endpoints[i].direction = UnknownDir;
       /* intitialize with some valid values before we have received
          the endpoint descriptor */
       if (speed == 1) {
           endpoints[i].interrupt_receive_size = 8;
           endpoints[i].descriptor.wMaxPacketSize = 8;
       } else {
           endpoints[i].interrupt_receive_size = 64;
           endpoints[i].descriptor.wMaxPacketSize = 64;
       }
   }

   endpoints[0].type = Control;
   endpoints[0].direction = Both;
   endpoints[0].address = 0;
}



USBDevice::~USBDevice() {
    if (driver != 0)
        delete driver;
    USBDevice::freeDeviceIDs->addTail(reinterpret_cast<ListItem*>(this->addr));
}
