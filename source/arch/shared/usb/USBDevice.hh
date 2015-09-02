/*
 * USBDevice.hh
 *
 *  Created on: 27.08.2015
 *      Author: Daniel
 */

#ifndef SOURCE_ARCH_SHARED_USB_USBDEVICE_HH_
#define SOURCE_ARCH_SHARED_USB_USBDEVICE_HH_

#include "USBHostController.hh"
#include "filesystem/Resource.hh"
#include "USBDriverLibrary.hh"

class USB_Host_Controller;

typedef enum {
    EP_ERROR,
    EP_TRANSFER_ACTIVE,
    EP_TRANSFER_ERROR,
    EP_TRANSFER_FINISHED,
    EP_ENDPOINT_STALLED
} TEndPointTransferState;

class USBDevice : public Resource {
public:

    static ArrayList* freeDeviceIDs;

    /*****************************************************************************
     * Method: initialize()
     *
     * @description
     *  Static Initialization
     *******************************************************************************/
    static void initialize() {
        freeDeviceIDs = new ArrayList(255);
        /* maximum 255 devices (USB 2.0 limit) */
        for (unint4 i = 1; i < 255; i++) {
            freeDeviceIDs->addTail(reinterpret_cast<ListItem*>(i));
        }
    }

    USBDevice(USB_Host_Controller *controller, USBDevice *parent, unint1 port, unint1 speed);

    virtual ~USBDevice();

      // the device descriptor received
      DeviceDescriptor dev_descr;

      // the first interface descriptor received
      InterfaceDescriptor if_descr;

      // we support a maximum of five endpoints per device (4 + 1 control)
      DeviceEndpoint endpoints[5];

      // The controller this device is connected to
      USB_Host_Controller *controller;

      // parent device (hub) or null
      USBDevice *parent;


      // the associated device address
      unint1 addr;

      // number of endpoints this device provides
      unint1 numEndpoints;

      // port this device is connected on at the parent usb device
      unint1 port;

      // the speed of this device, 0 = full, 1 = low, 2 = high
      unint1 speed;

      // some private data used by the device driver for this device
      void* dev_priv;

      // hub descriptor    (in case this device is a hub)
      HubDescriptor hub_descr;

      // pointer to the usb device drive
      USBDeviceDriver *driver;


      /*****************************************************************************
       * Method: activateEndpoint(int endpoint)
       *
       * @description
       *  activates the endpoint
       *
       * @params
       *  endpoint     Endpoint number to activate
       * @returns
       *  int         Error Code
       *******************************************************************************/
      virtual ErrorT activateEndpoint(int endpoint) = 0;

      /*****************************************************************************
       * Method: setMaxPacketSize(int endpoint)
       *
       * @description
       *  The controller uses this method to set the maximum packet size for
       *  a device endpoint after the controller identified this size during enumeration.
       *  The specialization can use this information to update existing packet
       *  descriptors for this endpoint.
       *
       * @params
       *  endpoint     Endpoint number
       * @returns
       *  int         Error Code
       *******************************************************************************/
      virtual ErrorT setMaxPacketSize(int endpoint, unint2 maxSize) = 0;

      virtual TEndPointTransferState getEndpointTransferState(int num) = 0;

      virtual int getEndpointTransferSize(int num) = 0;

      /*****************************************************************************
       * Method: deactivate()
       *
       * @description
       *  deactivates the device and stops all transfers
       * @returns
       *  int         Error Code
       *******************************************************************************/
      virtual ErrorT deactivate() = 0;

      /*****************************************************************************
       * Method: setAddress(unint1 addr)
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
      virtual ErrorT setAddress(unint1 addr);
};


#endif /* SOURCE_ARCH_SHARED_USB_USBDEVICE_HH_ */
