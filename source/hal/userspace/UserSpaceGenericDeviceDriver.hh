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

#ifndef USERSPACE_GENERICDEVICEDRIVER_H_
#define USERSPACE_GENERICDEVICEDRIVER_H_

#include <types.hh>
#include "inc/const.hh"

/*!
 * \ingroup devicedrivers
 * \brief   provides the standart interfaces for all devices. it consists currently of initialisation and power management functions
 *
 * The GenericDeviceDriver Interface is used to unify the access from the kernel to the deviced. Each device has to supply the following functions:
 *
 * \li probe
 * \li remove
 * \li shutdown
 * \li suspend
 * \li remove
 * \li getDriverDescription
 *
 * \section probeFunction the probe function
 * This function is provided to make it possible for the kernel to probe for a specific device. When a kernel is build for targets of the same
 * hardware platform but with different hardware features (a "generic" kernel) it can use the probe function before loading the driver to
 * probe for the presense of the device.
 * The driver should be able to determine if the hardware it is written for is really present and return a value according to this.
 *
 * \section removeFunction the remove function
 * When there is no longer need of usage of the hardware device in concern the kernel may unload the driver with the remove function. It should
 * free all memory used by the hardware driver and put the hardware in a power mode as low as possible (shutting it off completely is prefered).
 * The kernel will install the driver again when it needs the device again.
 *
 * \section shutdownFunction the shutdown function
 * The kernel may call this function when the device is not going to be used for a longer time period. So the driver can shut the hardware off.
 * the resume() function is called after shutdown before any usage of the hardware is done again.
 *
 * \section suspendFunction the suspend function
 * The kernel may call this function when the device should go into low power mode for a shorter period of time. The resume() function is called
 * before any use of the hardware is made before.
 *
 * The decision for having both a suspend and shutdown function was made because the hardware driver should take into account the wakeup time
 * the device would need to be ready again for operation. It is considered that calling the suspend function trades shorter wakeup time against
 * faster respond time from the hardware to be ready again.
 *
 */

class UserSpaceGenericDeviceDriver {

public:

	UserSpaceGenericDeviceDriver() {};

    ~UserSpaceGenericDeviceDriver() {
    }
    ;

    virtual ErrorT probe() {
        return cNotImplemented;
    }
    ;

    virtual ErrorT remove() {
        return cNotImplemented;
    }
    ;

    virtual ErrorT shutdown() {
        return cNotImplemented;
    }
    ;

    virtual ErrorT suspend() {
        return cNotImplemented;
    }
    ;

    virtual ErrorT resume() {
        return cNotImplemented;
    }
    ;

};

#endif /*GENERICDEVICEDRIVER_H_*/
