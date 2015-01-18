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

#include "AM335xClock.hh"
#include "inc/memio.h"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

AM335xClock::AM335xClock(T_AM335xClock_Init *init) :
        Clock(CLOCK_RATE) {

    /* The clock is emulated using Timer1 of the AM335x device running with 24 mhz
     * and using the overflow register to form a 52 bit clock register */

    //reset overflow counter register
    OUTW(AM335x_GPT1_TOCR, 0x0);
    OUTW(AM335x_GPT1_TOWR, 0xffffffff);

    OUTW(AM335x_GPT1_TIOCP_CFG, 0x308);

    // set positive and negative increment after overflow to 0
    OUTW(AM335x_GPT1_TPIR, 0x0);
    OUTW(AM335x_GPT1_TNIR, 0x0);

    // timer load value to 0
    OUTW(AM335x_GPT1_TLDR, 0x0);

    // configure for autoreload
    OUTW(AM335x_GPT1_TCLR, 0x2);

    // set counter to start value
    OUTW(AM335x_GPT1_TCRR, 0x0);

    OUTW(AM335x_GPT1_TMAR, 0x0);
    // disable interrupt generation as we use this timer as a clock
    OUTW(AM335x_GPT1_TIER, 0x0);

    // clear pending interrupts
    OUTW(AM335x_GPT1_TISR, 0x7);

    // start timer
    OUTW(AM335x_GPT1_TCLR, INW(AM335x_GPT1_TCLR) | 0x1);

}

AM335xClock::~AM335xClock() {
}

/*****************************************************************************
 * Method: Omap3530Clock::reset()
 *
 * @description
 *
 *******************************************************************************/
void AM335xClock::reset() {
}

