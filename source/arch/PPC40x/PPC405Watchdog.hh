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

#ifndef PPC405WATCHDOG_
#define PPC405WATCHDOG_

#include "powerpc.h"
#include <hal/Watchdog.hh>

/*!
 * \brief PPC405Watchdog, implementation of HAL Watchdog
 *
 * this is the hardware specific implementation of the Watchdog HAL class.
 * The PPC watchdog will reset the system if its internal timer register overflows
 * twice. An irq after the first overflow is generated, which leaves the programmer
 * the option to deal with the timeout (e.g. resolve the issue resoning the timout
 * to occur).
 * in this implementation nothing is done, so the watchdog just resets the system.
 */
class PPC405Watchdog: public Watchdog {

public:

    PPC405Watchdog( const char* name );
    ~PPC405Watchdog();

    /*!
     * \brief enable the PPC405 watchdog
     *
     * enables the watchdog, which will perform a reset when the watchdog timer
     * register overflows twice.
     */
    ErrorT enable();

    /*!
     * \brief kick the PPC405 watchdog
     *
     * invoking this function will reset the watchdog timer register. if the
     * watchdog is not kicked regulary, then a watchdog irq will occur and reset the
     * system.
     * note that this function does nothing, if the watchdog was not enabled
     * beforehand with a call to the enable() function.
     */
    ErrorT kick();

private:
    bool enabled;

};

#endif /*PPC405WATCHDOG_*/
