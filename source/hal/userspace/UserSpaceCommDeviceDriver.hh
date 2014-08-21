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

#ifndef USERSPACECOMMDEVICEDRIVER_H_
#define USERSPACECOMMDEVICEDRIVER_H_

#include "hal/userspace/UserSpaceCharacterDeviceDriver.hh"
#include "inc/const.hh"

/*!
 * \ingroup devicedrivers
 * \brief	UserSpace Device Driver base class for communication devices.
 *
 * As the implementaion of the driver resides in userspace this class functions
 * as an abstract class providing only interfaces that need to be implemented.
 */
class UserSpaceCommDeviceDriver: public UserSpaceCharacterDeviceDriver {
public:

    /*!
     * Anonymous Devices used this constructor!
     */
    UserSpaceCommDeviceDriver() {
    }
    ;

    virtual ~UserSpaceCommDeviceDriver() {
    }
    ;

    //! method which gets called whenever this devices throws a extern IRQ
    virtual
    void recv() {
    }

    virtual ErrorT lowlevel_send(char* data, int len) {
        return cNotImplemented ;
    }

    //! returns the mac address of this device
    virtual
    const char* getMacAddr() {
        return "0";
    }

    //! returns the size (amount of bytes) mac addresses have on this device
    virtual int1 getMacAddrSize() {
        return 0;
    }

    /*!
     * Returns the maximum transmit unit (MTU) of this device.
     *
     * \returns The maximum amount of bytes the device is able to transmit in one unit.
     */
    virtual unint2 getMTU() {
        return 0;
    }

    //! Returns the id of the hardware address space (ethernet, wlan ..)
    virtual int2 getHardwareAddressSpaceId() {
        return 0;
    }

    //! returns the broadcast address for this device medium
    virtual
    const char* getBroadcastAddr() {
        return "0";
    }

    //! enables the hardware interrupts of this device.
    virtual ErrorT enableIRQ() {
        return cNotImplemented ;
    }

    //! disables all interrupts of this device (does not clear them!)
    virtual ErrorT disableIRQ() {
        return cNotImplemented ;
    }

    //! clears all interrupts of this device
    virtual ErrorT clearIRQ() {
        return cNotImplemented ;
    }

    void operator delete(void*) {
    }

};

#endif /*COMMDEVICEDRIVER_H_*/
