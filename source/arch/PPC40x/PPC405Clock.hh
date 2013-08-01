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

#ifndef PPC405CLOCK_HH_
#define PPC405CLOCK_HH_

#include "powerpc.h"
#include <hal/Clock.hh>

/*! \brief PowerPC Clock, Implementation of HAL Clock
 *
 * This class implements a basic clock based on the internal
 * PowerPC 405 clock cycle counter.
 * The PowerPC has a 64bit wide register (internally deviced into two 32bit registers)
 * that count the clock cycles elapsed since the has been started up.
 * Based on the register value and the clock speed (which has to be set by a define correctly in
 * powerpc.h) the elapsed time is calculated.
 *
 * On each time reading function call the timer registers are read and
 * elapsed time calculation is redone to minimize rounding errors.
 * Because 64Bit Math is expensive don't be careful when you call this
 * class to get the current time.
 */
class PPC405Clock: public Clock {
public:
    PPC405Clock( const char* name );
    ~PPC405Clock();

    /*!
     * \brief return the time since the system startup in us (microseconds)
     *
     * returns the time since system startup by reading out the TimeBase
     * registers of the PPC405. This is a 64bit wide register holding
     * the clock ticks since startup.
     * for the calculation the be correct the CLOCK_RATE define needs to set
     * appropriatly!
     *
     * this value will overflow after approx. 200 years at 40Mhz clocl speed.
     */
    unint8 getTimeSinceStartup();

    /*!
     * \brief fills a given TimeStruct with the time since system startup
     *
     * the given TimeStruct is populated with the time since system startup
     * the time is broken down to the various data types. so if for example
     * exactly 1 year has passed only the year field of the struct will be 1
     * and all other fields will contain 0.
     */
   // bool getTimeSinceStartup( TimeStruct* ts );

    /*!
     * \brief Resets the time base registers.
     *
     * Resets all time base registers (TBU, TBL) to 0.
     */
    void reset();

};

#endif /*PPC405CLOCK_HH_*/
