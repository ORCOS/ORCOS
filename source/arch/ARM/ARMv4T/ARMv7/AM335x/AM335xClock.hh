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

#define MHZ *1000000
#define CLOCK_RATE    (24000000)
#define MILLISECONDS * 24000
#define ms * 24000
#define MICROSECONDS * 24

/* upon dispatching we will get at least 3 microseconds delay on this architecture  */
#define ARCH_DELAY 2 MICROSECONDS

#define AM335x_GPT1_BASE_ADDR     0x44E31000

#define TPIR_OFFSET        0x048
#define TNIR_OFFSET        0x04C
#define TLDR_OFFSET        0x02C
#define TIER_OFFSET        0x01C
#define TISR_OFFSET        0x018
#define TCLR_OFFSET        0x024
#define TCRR_OFFSET        0x028
#define TMAR_OFFSET        0x038
#define TOCR_OFFSET        0x054
#define TOWR_OFFSET        0x058

#define TIOCP_CFG_OFFSET   0x10

#define AM335x_GPT1_TIER    AM335x_GPT1_BASE_ADDR + TIER_OFFSET
#define AM335x_GPT1_TISR    AM335x_GPT1_BASE_ADDR + TISR_OFFSET
#define AM335x_GPT1_TCLR    AM335x_GPT1_BASE_ADDR + TCLR_OFFSET
#define AM335x_GPT1_TPIR    AM335x_GPT1_BASE_ADDR + TPIR_OFFSET
#define AM335x_GPT1_TNIR    AM335x_GPT1_BASE_ADDR + TNIR_OFFSET
#define AM335x_GPT1_TLDR    AM335x_GPT1_BASE_ADDR + TLDR_OFFSET
#define AM335x_GPT1_TCRR    AM335x_GPT1_BASE_ADDR + TCRR_OFFSET
#define AM335x_GPT1_TOCR    AM335x_GPT1_BASE_ADDR + TOCR_OFFSET
#define AM335x_GPT1_TOWR    AM335x_GPT1_BASE_ADDR + TOWR_OFFSET
#define AM335x_GPT1_TMAR    AM335x_GPT1_BASE_ADDR + TMAR_OFFSET
#define AM335x_GPT1_TIOCP_CFG    AM335x_GPT1_BASE_ADDR + TIOCP_CFG_OFFSET

/*! \brief ARMv4T Clock, Implementation of HAL Clock
 *
 */
class AM335xClock: public Clock {
public:
    /*****************************************************************************
     * Method: AM335xClock(T_AM335xClock_Init *init)
     *
     * @description
     *  Initializes the omap3530 clock. The timer2 is abused for this task
     *  using its overrun register to generate a 52 bit clock running at 13 MHZ.
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    explicit AM335xClock(T_AM335xClock_Init *init);

    ~AM335xClock();

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
        unint4 high    = INW(AM335x_GPT1_TOCR); /* upper 20 bits */
        unint4 low     = INW(AM335x_GPT1_TCRR); /* lower 32 bits */

        /* check for overflow */
        unint4 high2 = INW(AM335x_GPT1_TOCR);
        if (high2 != high) {
            low = INW(AM335x_GPT1_TCRR);
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
