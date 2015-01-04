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

#ifndef PPC405FIXEDINTERVALTIMER_HH_
#define PPC405FIXEDINTERVALTIMER_HH_

#include <error.hh>
#include <types.hh>
#include "powerpc.h"
#include <hal/TimerDevice.hh>

/*!
 * \brief PPC405FixedIntervalTimer, implementation of HAL TimerDevice
 *
 * this is the hardware specific implementation of the TimerDevice HAL class
 */
class PPC405FixedIntervalTimer: public TimerDevice {
protected:
    /*!
     * \brief performs hardware specific stuff individual to the PPC405 hardware
     *
     * resets the timer registers on a tick to ensure that the next timer
     * irq can occur
     */
    void doHardwareStuffOnTick();

public:
    PPC405FixedIntervalTimer();
    ~PPC405FixedIntervalTimer();

    //! see TimerDevice::enable();
    ErrorT enable();

    //! see TimerDevice::disable();
    ErrorT disable();

    ErrorT tick();
};

#endif /*PPC405FIXEDINTERVALTIMER_HH_*/
