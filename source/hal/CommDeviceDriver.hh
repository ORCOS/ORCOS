/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2008 University of Paderborn

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef COMMDEVICEDRIVER_H_
#define COMMDEVICEDRIVER_H_

#include "CharacterDeviceDriver.hh"

class AddressProtocol;

/*!
 * \ingroup devicedrivers
 * \brief	CommDeviceDriver which is the base class of all communication devices
 *
 * This is the base class of all communication devices. Be sure to implement all needed
 * functionality for the device. This may also include the mac functionality if there is no
 * mac support inside the hardware of this device.
 */
class CommDeviceDriver: public CharacterDeviceDriver {
private:
    //! The global id counter
    static int2 globalCommDeviceIdCounter; //!< always needs to start at 0 and must increase by 1

    //! The id of this communication device
    int2 myId;

public:
    // flag whether this device has an interrupt pending
    bool interruptPending;

#if USE_WORKERTASK
    // flag indicating if this device has an workerthread currently assigned to it
    bool hasAssignedWokerThread;
#endif

    /*!
     * All superclasses need to use this constructor to ensure
     * a correct registration process.
     */
    CommDeviceDriver( const char* name );

    /*!
     * Anonymous Devices used this constructor!
     */
    CommDeviceDriver() {};

    ~CommDeviceDriver();

    //! this method ensures that the global comm device id counter is set to 0 at startup
    static void initialize() {
        globalCommDeviceIdCounter = 0;
    }

    //! method which gets called whenver this devices throws a extern IRQ
    virtual
    void recv() = 0;

    virtual
    ErrorT lowlevel_send( char* data, int len ) = 0;

    //! unblocking send method which sends the bytes given to the destination addr
    //virtual
    //ErrorT send( char* bytes, int len, char* dest_addr, int addr_len, AddressProtocol* fromProtocol ) {
    //ErrorT send( packet_layer* packet, char* dest_addr, int addr_len, int2 fromProtocol_ID ) = 0;

    //! broadcast method which sends the message to the devices broadcast address
    virtual
    ErrorT broadcast( packet_layer* packet, int2 fromProtocol_ID ) = 0;

    /*!
    * \brief Sends a multicast packet
    *
    *  dest_addr is needed since many mac protocols use the upper layer address to compute the multicast address
    */
    virtual
    ErrorT multicast( packet_layer* packet, int2 fromProtocol_ID, unint4 dest_addr ) = 0;


    //! returns the mac address of this device
    virtual
    const char* getMacAddr() {
        return "0";
    }

    //! returns the size (amount of bytes) mac addresses have on this device
    virtual
    int1 getMacAddrSize() {
        return 0;
    }

    /*!
    * Returns the maximum transmit unit (MTU) of this device.
    *
    * \returns The maximum amount of bytes the device is able to transmit in one unit.
    */
   virtual
   unint2 getMTU() {
       return 0;
   }

    //! Returns the id of the hardware address space (ethernet, wlan ..)
    virtual
    int2 getHardwareAddressSpaceId() {
        return 0;
    }

    //! returns the broadcast address for this device medium
    virtual
    const char* getBroadcastAddr() {
        return "0";
    }

    //! returns the id of this device
    int2 getId() {
        return myId;
    }

    //! enables the hardware interrupts of this device.
    virtual
    ErrorT enableIRQ() {
        return cNotImplemented;
    }

    //! disables all interrupts of this device (does not clear them!)
    virtual
    ErrorT disableIRQ() {
        return cNotImplemented;
    }

    //! clears all interrupts of this device
    virtual
    ErrorT clearIRQ() {
        return cNotImplemented;
    }


};

#endif /*COMMDEVICEDRIVER_H_*/
