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
#include "inc/memio.h"
#include "OMAP3530.h"

#define CLOCK_RATE    (13000000)
#define MILLISECONDS * 13000
#define ms * 13000
#define MICROSECONDS * 13

/* upon dispatching we will get at least 3 microseconds delay on this architecture  */
#define ARCH_DELAY 3 MICROSECONDS

/*! \brief ARMv4T Clock, Implementation of HAL Clock
 *
 */
class Omap3530Clock: public Clock {
private:
    unint8 high_precision_time;

public:
    /*****************************************************************************
     * Method: Omap3530Clock(T_Omap3530Clock_Init *init)
     *
     * @description
     *  Initializes the omap3530 clock. The timer2 is abused for this task
     *  using its overrun register to generate a 52 bit clock running at 13 MHZ.
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    explicit Omap3530Clock(T_Omap3530Clock_Init *init);

    ~Omap3530Clock();

     /*****************************************************************************
     * Method: getClockCycles()
     *
     * @description
     *  Returns the Clock cycles since startup. takes 400 ns to get due
     *  to interface clocking.
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    inline unint8 getClockCycles() {
        unint4 high    = INW(GPT2_TOCR); /* upper 20 bits */
        unint4 low     = INW(GPT2_TCRR); /* lower 32 bits */

        /* check for overflow */
        unint4 high2 = INW(GPT2_TOCR);
        if (high2 != high) {
            low = INW(GPT2_TCRR);
            high = high2;
        }

        unint8 ret = (((unint8) high) << 32) | ((unint8) low);
        return (ret);
    }

    /*****************************************************************************
     * Method: reset()
     *
     * @description
     *   Resets the time base registers.
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    void reset();
};

#endif /*OMAP3530CLOCK_HH_*/
