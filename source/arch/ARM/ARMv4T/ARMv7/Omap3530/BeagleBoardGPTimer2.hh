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

#ifndef BEAGLEBOARDGPTIMER2_HH_
#define BEAGLEBOARDGPTIMER2_HH_

#include <error.hh>
#include <types.hh>
#include <hal/TimerDevice.hh>


/*!
 * \brief BeagleBoardGPTimer1, implementation of HAL TimerDevice
 *
 * this is the hardware specific implementation of the TimerDevice HAL class
 */
class BeagleBoardGPTimer2: public TimerDevice {

private:

protected:

public:

	BeagleBoardGPTimer2(T_BeagleBoardGPTimer2_Init * init);
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
	ErrorT setTimer( TimeT t );

    inline ErrorT tick() {
        // invoke the hal tick() method which then calls the dispatcher
        return (TimerDevice::tick());
    }

};

#endif /* BEAGLEBOARDGPTIMER1_HH_ */
