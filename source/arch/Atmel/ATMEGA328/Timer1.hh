/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2010 University of Paderborn

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

#ifndef Timer1_HH_
#define Timer1_HH_

#include <error.hh>
#include <types.hh>
#include <hal/TimerDevice.hh>

#define ms * 1000
#define CLOCK_RATE 1000
#define MHZ * 100000


/*!
 * \brief BeagleBoardGPTimer1, implementation of HAL TimerDevice
 *
 * this is the hardware specific implementation of the TimerDevice HAL class
 */
class Timer1: public TimerDevice {

private:

protected:

public:

	Timer1();
    ~Timer1();

    /*!
     * \brief enable the general purpose timer
     */
    ErrorT enable();

    /*!
     * \brief enable the general purpose timer
     */
    ErrorT disable();

    /*!
     * \brief Set the timer register.
     */
	ErrorT setTimer( unint4 t );

    inline ErrorT tick() {
        // invoke the hal tick() method which then calls the dispatcher
        return TimerDevice::tick();
    }

};

#endif /* Timer1_HH_ */
