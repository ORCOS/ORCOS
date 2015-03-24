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

#include "CharacterDevice.hh"
#include "GenericDeviceDriver.hh"
#include "lwip/netif.h"

class AddressProtocol;

/*!
 * \ingroup devicedrivers
 * \brief    CommDeviceDriver which is the base class of all communication devices
 *
 * This is the base class of all communication devices. Be sure to implement all needed
 * functionality for the device. This may also include the mac functionality if there is no
 * mac support inside the hardware of this device.
 */
class CommDeviceDriver: public CharacterDevice {
private:
    //! The global id counter
    //static int2 globalCommDeviceIdCounter;  //!< always needs to start at 0 and must increase by 1

protected:
    /*! The network interface for this communication device driver inside lwip network stack */
    struct netif st_netif;
public:
    /*****************************************************************************
     * Method: CommDeviceDriver(const char* name)
     *
     * @description
     *  All superclasses need to use this constructor to ensure
     *  a correct registration process.
     *******************************************************************************/
    explicit CommDeviceDriver(const char* name);


    virtual ~CommDeviceDriver();

    //! this method ensures that the global comm device id counter is set to 0 at startup
    /*  static void initialize() {
     globalCommDeviceIdCounter = 0;
     }*/

    /*****************************************************************************
     * Method: getNetworkInterface()
     *
     * @description
     *  Returns the network interface of this communication device
     *******************************************************************************/
    struct netif* getNetworkInterface() {
        return (&st_netif);
    }

    /*****************************************************************************
     * Method: getMacAddr()
     *
     * @description
     *  returns the mac address of this device
     *******************************************************************************/
    virtual const char* getMacAddr(int& length) {
        length = 0;
        return ("0");
    }

    /*****************************************************************************
     * Method: getMTU()
     *
     * @description
     *  Returns the maximum transmit unit (MTU) of this device.
     *
     * @returns
     *  The maximum amount of bytes the device is able to transmit in one unit.
     *******************************************************************************/
    virtual unint2 getMTU() {
        return (0);
    }

    /*****************************************************************************
     * Method: ioctl(int request, void* args)
     *
     * @description
     *  Provides generic I/O Control options for network devices
     *  as e.g. setting the IP address, Gateway etc.
     *******************************************************************************/
    virtual ErrorT ioctl(int request, void* args);
};

#endif /*COMMDEVICEDRIVER_H_*/
