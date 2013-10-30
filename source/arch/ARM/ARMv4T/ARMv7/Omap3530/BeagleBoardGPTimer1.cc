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
#include "BeagleBoardGPTimer1.hh"

extern "C" Kernel* theOS;

BeagleBoardGPTimer1::BeagleBoardGPTimer1() {

	// configure GPTimer1: accurate 1ms tick
	// generate 1ms tick, autoreload, disable, set start value
	// choose 12k functional clock as timer clock
	OUTW(CM_CLKSEL_WKUP, (INW(CM_CLKSEL_WKUP) & 0xFFFFFFFE));

	// set input to 26 mhz sys clock ~ tick every 38 ns
/*	unint4 cm_clksel_per = INW(CM_CLKSEL_WKUP);
	OUTW(CM_CLKSEL_WKUP,cm_clksel_per | 1);*/

	// GPT1_TPIR POSITIVE_INC_VALUE = 232000 = 0x38A40
	OUTW(GPT1_TPIR, 0x38A40);

	// GPT1_TNIR NEGATIVE_INC_VALUE = 768000 = 0xBB800
	OUTW(GPT1_TNIR, 0xBB800);

	// GPT1_TLDR LOAD_VALUE = 0xFFFFFFE0
	OUTW(GPT1_TLDR, 0xFFFFFFE0);

	// configure for autoreload and disable timer
	OUTW(GPT1_TCLR, ((INW(GPT1_TCLR) | 0x2) & ~ 0x1));

	// set counter to start value
	OUTW(GPT1_TCRR, 0xFFFFFFE0);

	// clear pending interrupts
	OUTW(GPT1_TISR, 0x7);

	// configure and enable interrupts within interrupt controller (MPU_INTCPS_ILR(37))
	OUTW(MPU_INTCPS_ILR(37), (INW(MPU_INTCPS_ILR(37)) & ~ 0x1 )); // normal irq
	OUTW(MPU_INTCPS_ILR(37), (INW(MPU_INTCPS_ILR(37)) & ~ 0xFC )); // priority 0 (highest)
	OUTW(MPU_INTCPS_MIR_CLEAR(1), 0x70 ); // enable interrupt: (gpt1 int no. 37: (37 mod 32) + 1): position of bit

	// enable overflow interrupt within timer (gpt1)
	OUTW(GPT1_TIER, INW(GPT1_TIER) | 0x2 );

	// initialize timer
    isEnabled = false;
    time = 0;
    elapsedCycles = 0;
    tickRate = 0;

}

BeagleBoardGPTimer1::~BeagleBoardGPTimer1() {
    disable();
}

ErrorT BeagleBoardGPTimer1::enable() {

	this->setTimer(time);

	// start timer
	OUTW(GPT1_TCLR, INW(GPT1_TCLR) | 0x1 );


	isEnabled = true;
    return cOk;
}

ErrorT BeagleBoardGPTimer1::disable() {

	isEnabled = false;

	// stop timer
	OUTW(GPT1_TCLR, INW(GPT1_TCLR) & ~0x1 );

    return cOk;
}

ErrorT BeagleBoardGPTimer1::setTimer( unint4 t ) {

	//reset overflow counter register
	OUTW(GPT1_TOCR, 0x0);

	// write t (time) to overflow wrapping register
	unint4 val =  (t / (CLOCK_RATE / 1000) );
	OUTW(GPT1_TOWR, val);

	// reset timer interrupt pending bit
	OUTW(GPT1_TISR, 0x2);

	// allow new interrupts
	theOS->getBoard()->getInterruptController()->clearIRQ( 0 );

	// again: reset timer interrupt pending bit
	OUTW(GPT1_TISR, 0x2);

	tickRate = t; // tickRate = cycles/tick

	TimerDevice::setTimer(t);
    return cOk;
}

