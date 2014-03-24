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

#ifndef GENERICDEVICEDRIVER_H_
#define GENERICDEVICEDRIVER_H_

#include <types.hh>
#include "filesystem/Resource.hh"

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

class GenericDeviceDriver: public Resource {

public:
    // flag whether this device has an interrupt pending
    bool interruptPending;

    // flag indicating if this device has an workerthread currently assigned to it
    bool hasAssignedWorkerThread;

	/**
	 * Constructor for generic devices
	 */
    GenericDeviceDriver( bool sync_res, const char* p_name ) :
        Resource( cGenericDevice, sync_res, p_name ) {
    	interruptPending = false;
    	hasAssignedWorkerThread = false;
    }


    /**
     *  Constructor for drivers specialising from generic device driver,
     *  thus, providing its own ResourceType information.
     */
    GenericDeviceDriver( ResourceType rt, bool sync_res, const char* p_name ) :
        Resource( rt, sync_res, p_name ) {
    	interruptPending = false;
    	hasAssignedWorkerThread = false;
    }


    virtual ~GenericDeviceDriver() {
    }


    virtual ErrorT probe() {
        return (cNotImplemented);
    }


    virtual ErrorT shutdown() {
        return (cNotImplemented);
    }


    virtual ErrorT suspend() {
        return (cNotImplemented);
    }


    virtual ErrorT resume() {
        return (cNotImplemented);
    }


    /*
     * IRQ Handling method
     *
     */
    virtual ErrorT handleIRQ() {
    	return (cNotImplemented);
    }


    //! enables the hardware interrupts of this device.
    virtual  ErrorT enableIRQ() {
        return (cNotImplemented);
    }

    //! disables all interrupts of this device (does not clear them!)
    virtual  ErrorT disableIRQ() {
        return (cNotImplemented);
    }

    //! clears all interrupts of this device
    virtual   ErrorT clearIRQ() {
        return (cNotImplemented);
    }

    virtual ErrorT ioctl(int request, void* args) {
    	return (cNotImplemented);
    }


};

#endif /*GENERICDEVICEDRIVER_H_*/
