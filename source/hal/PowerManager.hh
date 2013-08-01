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

#ifndef POWERMANAGER_HH_
#define POWERMANAGER_HH_

#include <inc/error.hh>

/*!
 * \ingroup pwrmgmt
 * \brief   PowerManager provides an abstract interface for target-dependend PowerManager-Implementations, part of the HAL.
 */

class PowerManager {
public:
    PowerManager();
    ~PowerManager();

    /*!
     * \brief called on entering the idle thread
     *
     * suspends puts all devices (for which it makes sense of course)
     * to sleep when entering the idle thread
     */
    ErrorT enterIdleThread();
};

#endif /*POWERMANAGER_HH_*/
