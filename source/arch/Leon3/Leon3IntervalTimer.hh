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

#ifndef LEON3INTERVALTIMER_HH_
#define LEON3INTERVALTIMER_HH_

#include <hal/TimerDevice.hh>

/*!
 * \brief Leon3ProgrammableIntervalTimer, implementation of HAL TimerDevice
 *
 * this is the hardware specific implementation of the TimerDevice HAL class
 */
class Leon3IntervalTimer : public TimerDevice
{

private:
    int4 baseAddr;
    int irqNum;
protected:

public:

    Leon3IntervalTimer();
    ~Leon3IntervalTimer();

    /*!
     * \brief Set the timer register.
     *
     * \param t The amount of microseconds per tick
     */
    ErrorT        setTimer(unint4 t);

    /*!
     * \brief enable the intervall timer
     */
    ErrorT        enable();

    /*!
     * \brief disable the intervall timer
     */
    ErrorT        disable();

    /*!
     * \brief method called whenever there is an intervall timer interrupt
     *
     * First does some hardware stuff; then delegates the call to the HAL class.
     */
    ErrorT        tick();

    int        getIRQ();
};

#endif /*LEON3INTERVALTIMER_HH_*/
