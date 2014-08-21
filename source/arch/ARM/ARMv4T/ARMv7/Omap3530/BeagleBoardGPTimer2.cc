/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2010 University of Paderborn

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

#include "ARMv4TProgrammableIntervalTimer.hh"
#include "OMAP3530.h"
#include "inc/memio.h"
#include <kernel/Kernel.hh>
#include "BeagleBoardGPTimer2.hh"

extern "C" Kernel* theOS;

BeagleBoardGPTimer2::BeagleBoardGPTimer2(T_BeagleBoardGPTimer2_Init * init) {

    // TODO: use init addresses!

    // set input to 26 mhz sys clock ~ tick every 38 ns
    unint4 cm_clksel_per = INW(CM_CLKSEL_CORE);
    OUTW(CM_CLKSEL_CORE, cm_clksel_per | (1 << 6));

    // we will use GP10 for the timer functionality as it is connected to the L4 Interface
    // we can access the GP10 (default) with 100 MHZ.
    // setting the time is not so urgent
    // GPTimer 2 (at 200) MHZ interface will be used for the clock as it is accessed more
    // recently

    // enable timer 2 functional clock
    //OUTW(0x48005000,INW(0x48005000) | (1 <<3));
    // enabel time 2 interface clock
    //OUTW(0x48005010,INW(0x48005010) | (1 << 3));

    // enable GPTimer 10 functional clock
    OUTW(CM_FCLKEN1_CORE, INW(CM_FCLKEN1_CORE) | (1 << 11));

    // enable GPTimer 10 interface clock
    OUTW(CM_ICLKEN1_CORE, INW(CM_ICLKEN1_CORE) | (1 << 11));

    // GPT1_TPIR POSITIVE_INC_VALUE = 232000 = 0x38A40
    OUTW(GPT10_TPIR, 0x0);

    // GPT1_TNIR NEGATIVE_INC_VALUE = 768000 = 0xBB800
    OUTW(GPT10_TNIR, 0x0);

    // GPT1_TLDR LOAD_VALUE = 0xFFFFFFE0
    OUTW(GPT10_TLDR, 0x0);

    // configure for autoreload, compare mode and disable timer
    OUTW(GPT10_TCLR, 0x2 | (0x1 << 6));

    // set counter to start value
    OUTW(GPT10_TCRR, 0x0);

    // clear pending interrupts
    OUTW(GPT10_TISR, 0x7);

    // configure and enable interrupts within interrupt controller (MPU_INTCPS_ILR(38))
    /*OUTW(MPU_INTCPS_ILR(38), (INW(MPU_INTCPS_ILR(38)) & ~ 0x1 )); // normal irq
     OUTW(MPU_INTCPS_ILR(38), (INW(MPU_INTCPS_ILR(38)) & ~ 0xFC )); // priority 0 (highest)
     OUTW(MPU_INTCPS_MIR_CLEAR(1), 0xF0 ); // enable interrupt: (gpt2 int no. 38: (38 mod 32) + 1): position of bit*/

    OUTW(MPU_INTCPS_ILR(46), (INW(MPU_INTCPS_ILR(46)) & ~ 0x1 ));// normal irq
    OUTW(MPU_INTCPS_ILR(46), (INW(MPU_INTCPS_ILR(46)) & ~ 0xFC ));  // priority 0 (highest)
    OUTW(MPU_INTCPS_MIR_CLEAR(1), 0x4000);  // enable interrupt: (gpt2 int no. 46: (46 mod 32) + 1): position of bit

    // enable intterupt on timer match (compare mode) (gpt2)
    OUTW(GPT10_TIER, INW(GPT10_TIER) | 0x1);

    // initialize timer
    isEnabled = false;
    time = 0;
    elapsedCycles = 0;
    tickRate = 0;

}

BeagleBoardGPTimer2::~BeagleBoardGPTimer2() {
    disable();
}

ErrorT BeagleBoardGPTimer2::enable() {

    this->setTimer(time);

    // start timer
    OUTW(GPT10_TCLR, INW(GPT10_TCLR) | 0x1);

    isEnabled = true;
    return (cOk );
}

ErrorT BeagleBoardGPTimer2::disable() {

    isEnabled = false;

    // stop timer
    OUTW(GPT10_TCLR, INW(GPT10_TCLR) & ~0x1);

    return (cOk );
}

ErrorT BeagleBoardGPTimer2::setTimer(TimeT t) {

    TimeT currentTime = theOS->getClock()->getClockCycles();
    /* set to 5 ticks if currentTime > t*/
    unint4 dt = 5;

    /* otherwise generate the next irq in (t- currentTime) ticks */
    if (currentTime < t)
        dt = t - currentTime;

    // set match value
    OUTW(GPT10_TMAR, dt);

    OUTW(GPT10_TCRR, 0x0);

    // reset timer interrupt pending bit
    OUTW(GPT10_TISR, 0x1);

    // allow new interrupts
    theOS->getBoard()->getInterruptController()->clearIRQ(0);

    // again: reset timer interrupt pending bit
    OUTW(GPT10_TISR, 0x1);

    tickRate = t;  // tickRate = cycles/tick

    TimerDevice::setTimer(t);
    return (cOk );
}

