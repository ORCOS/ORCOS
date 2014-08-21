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

#ifndef TIMERDEVICE_HH_
#define TIMERDEVICE_HH_

#include "inc/types.hh"
#include "inc/const.hh"
#include "SCLConfig.hh"

#include Kernel_Dispatcher_hh

/*!
 * \ingroup devicedrivers
 * \brief   Timer Device Class provides an abstract interface for all target-dependent hardware timer implementations, part of the HAL.
 *
 *  This class is the ancestor of all classes that provide access to a timer
 *  which can be programmed in a way, that a periodic timer
 *  interrupt occurs.
 *
 *  <h3>Method Reference</h3>
 *
 *  Use setTimer() to set the Timer to a new timeout.
 */

class TimerDevice {
private:

    // the dispatcher which will be called by this timer
    Kernel_DispatcherCfdCl* dispatcher;

protected:
    bool isEnabled;  // status of the underlying hardware timer
    unint4 time;  // the time [microseconds] after the callback should be called
    unint4 elapsedCycles;  // the cycles elapsed after the last call of the callback
    unint4 tickRate;  // the rate [cycles/tick] at which the Timer is ticked

    ErrorT doHardwareStuffOnTick() {
        return (cNotImplemented );
    }

public:

    TimerDevice();
    ~TimerDevice();

    /*!
     *   \brief Set the timer.
     *
     *  The timer device will produce a cylic timer interrupt each \c t cycles
     *
     *  \param t the amount of cycles
     */
    void setTimer(TimeT t) {
        time = t;
    }
    ;

    /*!
     *  \brief enable the timer hardware
     *
     * calling this method enables the underlying timer hardware to generate ticks.
     * The programmer of the architecture port should ensure that he sets the granularity
     * of the hardware ticks fine enough to achive accurate time measure
     */
    ErrorT enable() {
        return (cNotImplemented );
    }

    /*!
     * \brief disable the timer hardware
     *
     * this method disables the time hardware and needs to be overwritten in the architecture
     * implementation.
     * when called no timer irqs are supposed to appear and thus the TimerDevice-Class
     * stops working.
     */
    ErrorT disable() {
        return (cNotImplemented );
    }

    /*!
     * \brief performs a tick to the timer object
     *
     * method for usage in interrupt handler of the hardware timer. dont call this manually!
     */
    inline ErrorT tick() {

        // tickRate= cycles / tick
        elapsedCycles += tickRate;

        if (elapsedCycles >= time)
        {
            elapsedCycles = 0;
            dispatcher->dispatch();
        }

        return (cOk );
    }
    ;
};

#endif /*TIMERDEVICE_HH_*/
