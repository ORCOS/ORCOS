/*
 * SMSC95xxUSBDeviceDriver.hh
 *
 *  Created on: 02.05.2013
 *      Author: dbaldin
 */

#ifndef SMSC95XXUSBDEVICEDRIVER_HH_
#define SMSC95XXUSBDEVICEDRIVER_HH_

#include "USBDeviceDriverFactory.hh"
#include "hal/CommDeviceDriver.hh"
#include "arch/shared/USBEHCIHostController.hh"
#include "lwip/netif.h"

class SMSC95xxUSBDeviceDriver: public USBDeviceDriver, public CommDeviceDriver {
public:

	USBDevice *dev;

	// endpoint number for bulk out transfer
	unint1 bulkout_ep;

	// endpoint number for bulk in transfer
	unint1 bulkin_ep;

	// interrupt endpoint number
	unint1 int_ep;

	/*! The network interface for this eem module inside lwip */
	struct netif tEMAC0Netif;

private:
	// Receive the current packet
	ErrorT recv(unint4 recv_len);

	// Try to initialize the hardware
	ErrorT init();

	/*! The ipv4 address structure of this device.  */
	struct ip4_addr tIpAddr;

	// Link status
	bool	link_up;

public:
  SMSC95xxUSBDeviceDriver(USBDevice* dev);

  virtual ~SMSC95xxUSBDeviceDriver();

  ErrorT 	initialize();

  ErrorT 	handleInterrupt();

  /*! Implements the receiving behavior
  *
  *	Called by the interrupt handler if data is available for this endpoint device.
  *	Starts a bulk in transfer and delivers the data to the network protocol stack
  */
  void 		recv();

  /* Send out the data over the USB Ethernet Device */
  ErrorT 	lowlevel_send( char* data, int len );


  /* broadcast method which sends the message to the devices broadcast address*/
  ErrorT 	broadcast( packet_layer* packet, int2 fromProtocol_ID ) { return cNotImplemented;};

  /*!
  * \brief Sends a multicast packet
  *
  *  dest_addr is needed since many mac protocols use the upper layer address to compute the multicast address
  */
  ErrorT 	multicast( packet_layer* packet, int2 fromProtocol_ID, unint4 dest_addr ) {return cNotImplemented;};

};



class SMSC95xxUSBDeviceDriverFactory : public USBDeviceDriverFactory {

public:
	SMSC95xxUSBDeviceDriverFactory(char* name);

	/* checks whether the given class,product device is supported by this driver*/
	bool isDriverFor(USBDevice* dev);

	/* factory method which creates a new instance of this driver */
	USBDeviceDriver* getInstance(USBDevice* dev) {
		return (new SMSC95xxUSBDeviceDriver(dev));
	};

	virtual ~SMSC95xxUSBDeviceDriverFactory() {};
};

#endif /* SMSC95XXUSBDEVICEDRIVER_HH_ */

