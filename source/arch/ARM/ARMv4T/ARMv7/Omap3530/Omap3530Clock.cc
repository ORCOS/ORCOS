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
	/*
	// configure and enable interrupts within interrupt controller (MPU_INTCPS_ILR(38))
	OUTW(MPU_INTCPS_ILR(72), (INW(MPU_INTCPS_ILR(38)) & ~ 0x1 )); // normal irq
	OUTW(MPU_INTCPS_ILR(72), (INW(MPU_INTCPS_ILR(38)) & ~ 0xFC )); // priority 0 (highest)
	OUTW(MPU_INTCPS_MIR_CLEAR(2), 0x40 ); // enable interrupt: (gpt2 int no. 38: (38 mod 32) + 1): position of bit

	// enable overflow interrupt within timer (gpt2)
	OUTW(GPT2_TIER, INW(GPT2_TIER) | 0x2 );
	*/
}

Omap3530Clock::~Omap3530Clock() {
}


void Omap3530Clock::reset() {
}

unint8 Omap3530Clock::getTimeSinceStartup() {



	  unint4 time = 0;


	  // update the current Time
	  time = INW(0x48320010);


	  // The 32khz sync timer is really generating 32 khz ticks
	  // thus divide by 32 to get the correct 1 ms reference clock
	  // we are using for the other timers

	  // we could also use the 32 khz resolution but we would have to change
	  // the timers to use one of the GPTIMER 3-7 which are not generating
	  // fixed 1ms ticks

	  // overflow every 134217,727 seconds = 2236,9 minutes = 37,28 h
	  // TODO: capture overflow so we can work longer than 37 h
	  time = time * (((CLOCK_RATE) / 1000) ) / 32;

	  return time;

}
