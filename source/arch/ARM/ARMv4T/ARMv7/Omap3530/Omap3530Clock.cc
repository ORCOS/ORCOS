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

#include "Omap3530Clock.hh"
#include "inc/memio.h"
#include "OMAP3530.h"

Omap3530Clock::Omap3530Clock( const char* name ) :
    Clock( name ) {

#if CLOCK_RATE != (26 MHZ)
#warning "Omap3530Clock should be used with BeagleBoardGPTimer2 for 26Mhz resolution!"
#endif
	// set input to 26 mhz sys clock ~ tick every 38 ns
	unint4 cm_clksel_per = INW(CM_CLKSEL_PER);
	OUTW(CM_CLKSEL_PER,cm_clksel_per | 2);

	// enable timer 3 functional clock
	OUTW(0x48005000,INW(0x48005000) | (1 <<4));
	// enabel timer 3 interface clock
 	OUTW(0x48005010,INW(0x48005010) | (1 << 4));


	// GPT1_TPIR POSITIVE_INC_VALUE = 232000 = 0x38A40
	OUTW(GPT3_TPIR, 0x0);

	// GPT1_TNIR NEGATIVE_INC_VALUE = 768000 = 0xBB800
	OUTW(GPT3_TNIR, 0x0);

	// GPT1_TLDR LOAD_VALUE = 0xFFFFFFE0
	OUTW(GPT3_TLDR, 0x0);

	// configure for autoreloa and disable timer
	OUTW(GPT3_TCLR,  0x2 );

	// set counter to start value
	OUTW(GPT3_TCRR, 0x0);

	// clear pending interrupts
	OUTW(GPT3_TISR, 0x7);
	OUTW(GPT3_TIER, 0x0);

	unint4 time_32khz = INW(0x48320010);

	// 1 clock tick at 32 khz is approx. 812 clock ticks at 26 mhz
	// initialize time base
	high_precision_time = time_32khz * 812;
	// enable timer
	OUTW(GPT3_TCLR, INW(GPT3_TCLR) | 0x1 );

}

Omap3530Clock::~Omap3530Clock() {
}


void Omap3530Clock::reset() {
}

unint8 Omap3530Clock::getTimeSinceStartup() {

	  // update the current Time
	  unint4 time_32khz = INW(0x48320010);

	  // The 32khz sync timer is really generating 32 khz ticks
	  // thus divide by 32 to get the correct 1 ms reference clock
	  // we are using for the other timers

	  // we could also use the 32 khz resolution but we would have to change
	  // the timers to use one of the GPTIMER 3-7 which are not generating
	  // fixed 1ms ticks


	  //time_32khz = time_32khz * (((CLOCK_RATE) / 1000) );

	  unint4 high_precision_cycles = INW(GPT3_TCRR);
	  OUTW(GPT3_TCRR, 0x0);
	  high_precision_time += high_precision_cycles;

#if CLOCK_RATE != (26 MHZ)
	  return high_precision_time * (((CLOCK_RATE) / 1000000)) / 26;
#else
	  // TODO compare 32khz clock with emulated time on a 32khz basis
	  // to limit drift.

	  return high_precision_time;
#endif

}
