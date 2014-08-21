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

#ifndef ARMv4TPROGRAMMABLEINTERVALTIMER_HH_
#define ARMv4TPROGRAMMABLEINTERVALTIMER_HH_

#include <error.hh>
#include <types.hh>
#include <hal/TimerDevice.hh>

/*!
 * \brief ARMv4TProgrammableIntervalTimer, implementation of HAL TimerDevice
 */
class ARMv4TProgrammableIntervalTimer: public TimerDevice {
public:

    ARMv4TProgrammableIntervalTimer();
    ~ARMv4TProgrammableIntervalTimer();

    /*!
     * \brief Set the timer register.
     *
     * \param t The amount of cycles
     */
    ErrorT setTimer(unint4 t);

    /*!
     * \brief enable the programmable intervall timer
     */
    ErrorT enable();

    /*!
     * \brief disable the programmable intervall timer
     */
    ErrorT disable();

    /*!
     * \brief method called whenever there is an programmaber intervall timer interrupt
     *
     * First does some hardware stuff; then delegates the call to the HAL class.
     */
    inline ErrorT tick() {
        // invoke the hal tick() method which then calls the dispatcher
        return TimerDevice::tick();
    }
};

#endif /*ARMv4TPROGRAMMABLEINTERVALTIMER_HH_*/
