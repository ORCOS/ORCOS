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
#include <hal/Clock.hh>

// timing for 125 khz system clock with timer prescaler  / 1024
/*#define ms / 8
#define CLOCK_RATE 125

#define MILLISECONDS / 8
#define MICROSECONDS*/

// timing for 1 MHZ system clock with timer prescaler  / 1024
#define ms / 16
#define CLOCK_RATE 976

#define MILLISECONDS / 16
#define MICROSECONDS

/*!
 * \brief ATMEGA328 Timer implementation also functioning as the clock
 *
 * this is the hardware specific implementation of the TimerDevice HAL class
 */
class Timer1 {

private:
	unint4 time;
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


	ErrorT tick();




};

#endif /* Timer1_HH_ */
