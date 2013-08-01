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

/*!
 * HAL Class for time dependend functions
 */

/*!
 * \brief a time structure representing a date from years down to us.
 *
 * This structure represents a date consisting of
 * years, days, hours, minutes, seconds, ms and us.
 *
 * The size of the datatype is choosen accordingly the
 * datatypes in the normal calendar. for example you want to
 * store 90 minutes, don't write 90 to the minutes field but
 * write 1 to the hours field and 30 to the minutes field.
 */
struct TimeStruct {
    unint2 years;
    unint2 days;
    unint1 hours;
    unint1 minutes;
    unint1 seconds;
    unint2 milliseconds;
    unint2 microseconds;
};

/*!
 * \brief Clock provides an abstract interface for a target-dependend hardware clock implementations, part of the HAL
 */
class Clock: public GenericDeviceDriver {
public:
    Clock( const char* name );
    ~Clock();

    /*!
     * \brief returns the time since startup
     * abstract function to get the time since system startup in [us] since
     * system startup.
     */
    virtual unint8 getTimeSinceStartup() {
        return cNotImplemented;
    }
    ;

    /*!
     * \brief writes the time since startup to the given TimeStruct
     *
     * writes the time since startup to the given TimeStruct structure.
     * this is the preferable way for retrieving the time as it is much more
     * accurate than the getTimeSinceStartup() function
     */
  /*  virtual bool getTimeSinceStartup( TimeStruct* ts ) {
        return cNotImplemented;
    }
    ;*/

    /*!
     * \brief resets the clock.
     */
    virtual void reset() {
    }
    ;
};

#endif /*CLOCK_HH_*/
