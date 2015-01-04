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

#ifndef DUMMYWATCHDOG_
#define DUMMYWATCHDOG_

#include <error.hh>
#include <types.hh>
#include <hal/Watchdog.hh>

/*!
 * \brief DummyWatchdog, dummy class with no functionality
 *
 * This class can be used to remove all watchdog functionality from the OS if
 * e.g. the Board does not provide a watchdog.
 *  *
 */
class DummyWatchdog: public Watchdog {
public:

    DummyWatchdog(const char* name) {
    }

    ~DummyWatchdog() {
    }

    /*!
     * \brief Enable the watchdog
     *
     * No functionality..
     */
    ErrorT enable() {
        return (cNotImplemented);
    }

    /*!
     * \brief kick the watchdog
     *
     * No functionality..
     */
    ErrorT kick() {
        return (cNotImplemented);
    }
};

#endif /*PPC405WATCHDOG_*/
