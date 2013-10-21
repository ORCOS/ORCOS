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

#ifndef OMAP3530CLOCK_HH_
#define OMAP3530CLOCK_HH_

#include <hal/Clock.hh>


#define	CLOCK_RATE	( 32768 )

#define MILLSECONDS * 33
#define ms * 33

/*! \brief ARMv4T Clock, Implementation of HAL Clock
 *
 */
class Omap3530Clock: public Clock {

private:
	unint8 high_precision_time;

public:
	Omap3530Clock( T_Omap3530Clock_Init *init );
    ~Omap3530Clock();

    /*!
     * \brief return the time since the system startup in us (microseconds)
     *
     */
    unint8 getTimeSinceStartup();

    /*!
     * \brief Resets the time base registers.
     */
    void reset();

};

#endif /*OMAP3530CLOCK_HH_*/
