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

Omap3530Clock::Omap3530Clock(T_Omap3530Clock_Init *init) :
        Clock() {

    // enable timer 1 interface clock
//	OUTW(0x48004C10,INW(0x48004C10) | 1);

// enable time 1 functional clock
//	OUTW(0x48004C00,INW(0x48004C00) | 1 );

// set GPTimer2 s´clock source to sys clock == (13 mhz)
    unint4 cm_clksel_per = INW(CM_CLKSEL_PER);
    OUTW(CM_CLKSEL_PER, cm_clksel_per | 1);

    // enable timer 2 functional clock
    OUTW(0x48005000, INW(0x48005000) | (1 <<3));
    // enable timer 2 interface clock
    OUTW(0x48005010, INW(0x48005010) | (1 << 3));

    //reset overflow counter register
    OUTW(GPT2_TOCR, 0x0);
    OUTW(GPT2_TOWR, 0xffffffff);

    OUTW(GPT2_TIOCP_CFG, 0x308);

    // set positive and negative increment after overflow to 0
    OUTW(GPT2_TPIR, 0x0);
    OUTW(GPT2_TNIR, 0x0);

    // timer load value to 0
    OUTW(GPT2_TLDR, 0x0);

    // configure for autoreload
    OUTW(GPT2_TCLR, 0x2);

    // set counter to start value
    OUTW(GPT2_TCRR, 0x0);

    OUTW(GPT2_TMAR, 0x0);
    // disable interrupt generation as we use this timer as a clock
    OUTW(GPT2_TIER, 0x0);

    // clear pending interrupts
    OUTW(GPT2_TISR, 0x7);

    // start timer
    OUTW(GPT2_TCLR, INW(GPT2_TCLR) | 0x1);

}

Omap3530Clock::~Omap3530Clock() {
}

void Omap3530Clock::reset() {

}

