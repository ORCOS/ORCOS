/*
 * SMSC95xxUSBDeviceDriver.hh
 *
 *  Created on: 02.05.2013
 *     Copyright & Author: dbaldin
 */

#ifndef SMSC95XXUSBDEVICEDRIVER_HH_
#define SMSC95XXUSBDEVICEDRIVER_HH_

#include "../USBDeviceDriver.hh"
#include "hal/CommDeviceDriver.hh"
#include "arch/shared/usb/USBHostController.hh"

class SMSC95xxUSBDeviceDriver: public USBDeviceDriver, public CommDeviceDriver {
public:
    /* THe usb device this driver operates on*/
    USBDevice*      dev;

    // endpoint number for bulk out transfer
    unint1          bulkout_ep;

    // endpoint number for bulk in transfer
    unint1          bulkin_ep;

    // interrupt endpoint number
    unint1          int_ep;

    struct pbuf*    last_pbuf;

    unint4          remaining_len;

    int             cur_len;

    Mutex*          mutex;

private:
    /*****************************************************************************
     * Method: recv(char* rxbuffer, unint4 recv_len)
     *
     * @description
     *  Receives the packet transfered by the IRQ usb transfer and
     *  checks the packet on completeness. If the packet is complete
     *  it is forwarded up the network stack.
     *
     * @params
     *  recv_len    The length received by the IRQ usb transfer.
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT recv(char* rxbuffer, unint4 recv_len);

    /*****************************************************************************
     * Method: init()
     *
     * @description
     *  Try to initialize the hardware. Initializes all PHY, MII etc. components
     *  using USB control transfers to the SMSC95xx usb device.
     *
     * @params
     *
     * @returns
     *  int         Error Code. cOk == 0 on success.
     *******************************************************************************/
    ErrorT init();

    /*! The ipv4 address structure of this device.  */
    struct ip4_addr tIpAddr;

    // Link status
    bool link_up;

public:
    /*****************************************************************************
     * Method: SMSC95xxUSBDeviceDriver(USBDevice* dev)
     *
     * @description
     *   Constructor.. Creates a new device driver for a SMSC95xx usb device.
     *
     * @params
     *  dev         The new SMSC95xx usb device attached.
     *******************************************************************************/
    explicit SMSC95xxUSBDeviceDriver(USBDevice* dev);

    ~SMSC95xxUSBDeviceDriver();


    static USBDeviceDriver* getInstance(USBDevice* dev) {
        return new SMSC95xxUSBDeviceDriver(dev);
    }

    /*****************************************************************************
     * Method: initialize()
     *
     * @description
     *  Tries to initialize the SMSC95xx device. Probes the endpoint. Called
     *  by the USB HC or HUB. If initialization fails the device is deactivated
     *  again.
     *
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT initialize();


    /*****************************************************************************
     * Method: reset()
     *
     * @description
     *  Resets the hardware. Initializes all PHY, MII etc. components
     *  using USB control transfers to the SMSC95xx usb device.
     *
     * @params
     *
     * @returns
     *  int         Error Code. cOk == 0 on success.
     *******************************************************************************/
    ErrorT reset();

    /*****************************************************************************
     * Method: handleInterrupt()
     *
     * @description
     *  SMSC95xx Interrupt Handler called by the USB EHCI CH Implementation
     *  if an irq request succeeds. This handler handles standard irq
     *  received from the interrupt endpoint of the smsc95 as well as
     *  interrupts due to successfull packet receptions done by the
     *  virtual interrupt created above.
     *
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT handleInterrupt();
};

#endif /* SMSC95XXUSBDEVICEDRIVER_HH_ */

