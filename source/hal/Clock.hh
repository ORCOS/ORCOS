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

#ifndef CLOCK_HH_
#define CLOCK_HH_

#include <error.hh>
#include <types.hh>
#include "GenericDeviceDriver.hh"
#include "CallableObject.hh"


/*!
 *  Clock provides an abstract interface for a
 *  target-dependent hardware clock implementations.
 *  Part of the HAL.
 */
class Clock: public CallableObject {
private:
    /* The Date Time at synchronization point*/
    unint4 synchDateTime;

    /* The local time at synchronization point*/
    TimeT synchLocalCycles;

    unint4 frequency;

public:
    Clock(unint4 frequency);

    virtual ~Clock();

    /*****************************************************************************
     * Method: getClockCycles()
     *
     * @description
     *  returns the clock cycles since startup.
     *  Abstract function to get the time since system startup in [us] since
     *  system startup.
     *******************************************************************************/
    virtual TimeT getClockCycles() = 0;

    /*****************************************************************************
     * Method: getDateTime()
     *
     * @description
     * Returns the current Date Time as an 4 byte unsigned integer
     *        counting as seconds from January 1. 1970 if NTP SYNC was
     *        successfull. Seconds since startup otherwise.
     *
     * @returns
     *  int         The time in seconds since January 1. 1970
     *******************************************************************************/
    unint4 getDateTime();

    /*****************************************************************************
     * Method: callbackFunc(void* param)
     *
     * @description
     *  Callback scheduled after system startup to
     *  perform NTP time synchronization if enabled.
     *******************************************************************************/
    void callbackFunc(void* param);

    /*****************************************************************************
     * Method: reset()
     *
     * @description
     *   Resets the clock.
     *******************************************************************************/
    virtual void reset() {}
};

#endif /*CLOCK_HH_*/
