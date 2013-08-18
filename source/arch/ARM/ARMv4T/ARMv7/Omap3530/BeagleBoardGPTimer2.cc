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

BeagleBoardGPTimer2::BeagleBoardGPTimer2() {

	// configure GPTimer1: accurate 1ms tick
	// generate 1ms tick, autoreload, disable, set start value

	// set input to 26 mhz sys clock ~ tick every 38 ns
	unint4 cm_clksel_per = INW(CM_CLKSEL_PER);
	OUTW(CM_CLKSEL_PER,cm_clksel_per | 1);

	// enable timer 2 functional clock
	OUTW(0x48005000,INW(0x48005000) | (1 <<3));
	// enabel time 2 interface clock
 	OUTW(0x48005010,INW(0x48005010) | (1 << 3));

//	OUTW(CM_CLKSEL_WKUP, (INW(CM_CLKSEL_WKUP) & 0xFFFFFFFE));

	// GPT1_TPIR POSITIVE_INC_VALUE = 232000 = 0x38A40
	OUTW(GPT2_TPIR, 0x0);

	// GPT1_TNIR NEGATIVE_INC_VALUE = 768000 = 0xBB800
	OUTW(GPT2_TNIR, 0x0);

	// GPT1_TLDR LOAD_VALUE = 0xFFFFFFE0
	OUTW(GPT2_TLDR, 0x0);

	// configure for autoreload, compare mode and disable timer
	OUTW(GPT2_TCLR,  0x2 | (0x1 << 6));

	// set counter to start value
	OUTW(GPT2_TCRR, 0x0);

	// clear pending interrupts
	OUTW(GPT2_TISR, 0x7);

	// configure and enable interrupts within interrupt controller (MPU_INTCPS_ILR(38))
	OUTW(MPU_INTCPS_ILR(38), (INW(MPU_INTCPS_ILR(38)) & ~ 0x1 )); // normal irq
	OUTW(MPU_INTCPS_ILR(38), (INW(MPU_INTCPS_ILR(38)) & ~ 0xFC )); // priority 0 (highest)
	OUTW(MPU_INTCPS_MIR_CLEAR(1), 0xF0 ); // enable interrupt: (gpt2 int no. 38: (38 mod 32) + 1): position of bit

	// enable intterupt on timer match (compare mode) (gpt2)
	OUTW(GPT2_TIER, INW(GPT2_TIER) | 0x1 );

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
	OUTW(GPT2_TCLR, INW(GPT2_TCLR) | 0x1 );


	isEnabled = true;
    return cOk;
}

ErrorT BeagleBoardGPTimer2::disable() {

	isEnabled = false;

	// stop timer
	OUTW(GPT2_TCLR, INW(GPT2_TCLR) & ~0x1 );

    return cOk;
}

ErrorT BeagleBoardGPTimer2::setTimer( unint4 t ) {

	//reset overflow counter register
	OUTW(GPT2_TOCR, 0x0);

	//printf("Timer: %d , us=%d\r",t, t/13);

	// write t (time) to overflow wrapping register
	//unint4 val =  (t / ((CLOCK_RATE / 1000000)));
	OUTW(GPT2_TCRR, 0x0);

	// set match value
	OUTW(GPT2_TMAR, t);

	// reset timer interrupt pending bit
	OUTW(GPT2_TISR, 0x1);

	// allow new interrupts
	theOS->getBoard()->getInterruptController()->clearIRQ( 0 );

	// again: reset timer interrupt pending bit
	OUTW(GPT2_TISR, 0x1);

	tickRate = t; // tickRate = cycles/tick

	TimerDevice::setTimer(t);
    return cOk;
}

