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

#ifndef EMULATEDCLOCK_HH_
#define EMULATEDCLOCK_HH_

#include <hal/Clock.hh>


/*! \brief Emulation of a clock based on timer interrupts, may overflow easily
 *
 */
class EmulatedClock: public Clock {

private:
	TimeT high_precision_time;

public:
	EmulatedClock();
    ~EmulatedClock();

    /*!
     * \brief return the time since the system startup in us (microseconds)
     *
     */
    TimeT getTimeSinceStartup();


    void incTime(unint4 cycles) {
    	high_precision_time += cycles;
    }

    /*!
     * \brief Resets the time base registers.
     */
    void reset();

};

#endif /*EMULATEDCLOCK_HH_*/
