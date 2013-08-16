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

#ifndef BEAGLEBOARDGPTIMER1_HH_
#define BEAGLEBOARDGPTIMER1_HH_

#include <error.hh>
#include <types.hh>
#include <hal/TimerDevice.hh>

//#define	CLOCK_RATE	(26 MHZ)

/*!
 * \brief BeagleBoardGPTimer1, implementation of HAL TimerDevice
 *
 * this is the hardware specific implementation of the TimerDevice HAL class
 */
class BeagleBoardGPTimer2: public TimerDevice {

private:

protected:

public:

	BeagleBoardGPTimer2();
    ~BeagleBoardGPTimer2();

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

#endif /* BEAGLEBOARDGPTIMER1_HH_ */
