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

#ifndef WATCHDOG_H_
#define WATCHDOG_H_

#include "GenericDeviceDriver.hh"

/*!
 * \ingroup devicedrivers
 * \brief   Watchdog provides an abstract interface for target-dependent watchdog implementations, part of the HAL.
 *
 * The Watchdog class represents a hardware watchdog. With a watchdog you can have your system reset automatically on error conditions.
 * The Kernel kicks the watchdog at various places to indicate that it is still active. But the system designer has to take care of configuring
 * the watchdog timeout appropiate to the load his application puts on the kernel.
 */

class Watchdog: public GenericDeviceDriver {

public:

	/*!
	 * Anonymous constructor. Only used by dummy devices
	 */
	Watchdog() {}

    Watchdog( const char* name ) :
        GenericDeviceDriver( false, name ) {
    }
    ;
    ~Watchdog() {
    }

    ErrorT enable() {
        return cNotImplemented;
    }
    ErrorT disable() {
        return cNotImplemented;
    }
    ErrorT kick() {
        return cNotImplemented;
    }

};

#endif /*WATCHDOG_H_*/
