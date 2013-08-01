/*
 * USCommDeviceDriver.hh
 *
 *  Created on: 25.01.2013
 *      Author: Daniel
 */

#ifndef USCOMMDEVICEDRIVER_HH_
#define USCOMMDEVICEDRIVER_HH_

#include "CommDeviceDriver.hh"
#include "process/Module.hh"

/*!
 * User Space CommDeviceDriver.
 *
 * Implemtens the CommDeviceDriver functions and forwards them to the user space
 * device driver. Handles all related user space wrapping functionality.
 *
 * The USCommDeviceDriver inhertis the module class, thus, representing essentially a task
 * inside the memory. It contains a schedulable thread list for executing user space calls.
 * Threads will be scheduled with the corresponding calling thread priority, making
 * User Space Device Driver execution preemtable to reduce latency effects.
 */
class USCommDeviceDriver: public CommDeviceDriver, Module {
public:
	USCommDeviceDriver(const char* dev, unint4 mmio_address, unint4 mmio_size, unint4 us_driver_addr);

	virtual ~USCommDeviceDriver();

private:
	// The Memory mapped IO address for this device
	unint4 mmio_address;

	// The physical address of the user space driver code
	unint4 us_driver_address;

	/*!
	 * \brief the Module control block for this user space driver
	 */
	ORCOS_MCB *module_cb;

	// flag indicating wheter this module is valid
	int valid_module;

public:
	 //! method which gets called whenver this devices throws a extern IRQ
	void recv() {};

	ErrorT lowlevel_send( char* data, int len );


	//! broadcast method which sends the message to the devices broadcast address
	ErrorT broadcast( packet_layer* packet, int2 fromProtocol_ID );

	/*!
	* \brief Sends a multicast packet
	*
	*  dest_addr is needed since many mac protocols use the upper layer address to compute the multicast address
	*/
	ErrorT multicast( packet_layer* packet, int2 fromProtocol_ID, unint4 dest_addr );


	//! returns the mac address of this device
	const char* getMacAddr();


	//! returns the size (amount of bytes) mac addresses have on this device
	int1 getMacAddrSize();

	/*!
	* Returns the maximum transmit unit (MTU) of this device.
	*
	* \returns The maximum amount of bytes the device is able to transmit in one unit.
	*/
    unint2 getMTU();


	//! Returns the id of the hardware address space (ethernet, wlan ..)
	int2 getHardwareAddressSpaceId();


	//! returns the broadcast address for this device medium
	const char* getBroadcastAddr();


	//! enables the hardware interrupts of this device.
	ErrorT enableIRQ();

	//! disables all interrupts of this device (does not clear them!)
	ErrorT disableIRQ();

	//! clears all interrupts of this device
	ErrorT clearIRQ();

};



#endif /* USCOMMDEVICEDRIVER_HH_ */
