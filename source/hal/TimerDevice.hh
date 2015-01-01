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

class TimerDevice : public GenericDeviceDriver {
protected:
    bool isEnabled;         // status of the underlying hardware timer
    unint4 time;            // the time [microseconds] after the callback should be called
    unint4 elapsedCycles;   // the cycles elapsed after the last call of the callback
    unint4 tickRate;        // the rate [cycles/tick] at which the Timer is ticked

    // the dispatcher which will be called by this timer
    Kernel_DispatcherCfdCl* dispatcher;

    /* pointer to the low latency thread to be activated on timer interrupt */
    Thread* llThread;
public:
    TimerDevice(const char* name);
    ~TimerDevice();

    /*****************************************************************************
     * Method: setTimer(TimeT t)
     *
     * @description
     *  Set the timer.
     *
     *  The timer device will produce a cylic timer interrupt each \c t cycles
     *
     *  @params
     *    t the number of cycles
      *******************************************************************************/
    virtual ErrorT setTimer(TimeT t) {
        time = t;
        return (cOk);
    }

    /*****************************************************************************
     * Method: setPeriod(unint4 microseconds)
     *
     * @description
     *  Configures the Timer to periodic mode issuing timer ticks (IRQS) at the given
     *  period. To be implemented by the architecture specific class.
     *******************************************************************************/
    virtual void setPeriod(unint4 microseconds) = 0;


    /*****************************************************************************
     * Method: enable()
     *
     * @description
     *  enable the timer hardware
     *
     * calling this method enables the underlying timer hardware to generate ticks.
     * The programmer of the architecture port should ensure that he sets the granularity
     * of the hardware ticks fine enough to achive accurate time measure
     *******************************************************************************/
    virtual ErrorT enable() = 0;

    /*****************************************************************************
     * Method: disable()
     *
     * @description
     * disable the timer hardware
     *
     * this method disables the time hardware and needs to be overwritten in the architecture
     * implementation.
     * when called no timer irqs are supposed to appear and thus the TimerDevice-Class
     * stops working.
     *******************************************************************************/
    virtual ErrorT disable() = 0;

    /*****************************************************************************
     * Method: handleIRQ()
     *
     * @description
     *  Handles Timer IRQS from this device. Provides the functionality for
     *  ultra low latency thread activations.
     *******************************************************************************/
    ErrorT handleIRQ();

    /*****************************************************************************
     * Method: ioctl(int request, void* args)
     *
     * @description
     *  Userspace configuration access to this timer devices.
     *******************************************************************************/
    ErrorT ioctl(int request, void* args);

    /*****************************************************************************
     * Method: tick()
     *
     * @description
     *  performs a tick to the timer object
     *
     *  method for usage in interrupt handler of the hardware timer. dont call this manually!
     *******************************************************************************/
    inline void tick() {
        // tickRate= cycles / tick
        elapsedCycles += tickRate;

        if (elapsedCycles >= time) {
            elapsedCycles = 0;
            dispatcher->dispatch();
        }
        return;
    }
};

#endif /*TIMERDEVICE_HH_*/
