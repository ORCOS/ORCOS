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

#include "PPC405FixedIntervalTimer.hh"

PPC405FixedIntervalTimer::PPC405FixedIntervalTimer() :
    TimerDevice() {
    isEnabled = false;
    time = 0;
    elapsedCycles = 0;
    tickRate = 0;
}

PPC405FixedIntervalTimer::~PPC405FixedIntervalTimer() {
    disable();
}

void PPC405FixedIntervalTimer::doHardwareStuffOnTick() {
    // clear the fit tsr flag
    ppc_mtspr(TSR, PPC_TSR_FIS);
}

ErrorT PPC405FixedIntervalTimer::enable() {
    switch ( 3 ) {
        case 3:
            ppc_mtspr(TCR, (ppc_mfspr(TCR) | (1<<25) | (1<<24)));
            tickRate = 2097152; // / ( CLOCK_RATE / 1000000 );
            //tickRate = 10490;  //microseconds / tick (2^21);
            break;
        case 2:
            ppc_mtspr(TCR, (ppc_mfspr(TCR) | (1<<25)) & ~(1<<24));
            tickRate = 131072; // / ( CLOCK_RATE / 1000000 );
            //tickRate = 655; //(2^17);
            break;
        case 1:
            ppc_mtspr(TCR, (ppc_mfspr(TCR) | (1<<24)) & ~(1<<25));
            tickRate = 8192; // / ( CLOCK_RATE / 1000000 );
            //tickRate = 50; //(2^13);
            break;

            /* Do not use the fastest timer (2^9 clocks) speed. Seems
             * not to work on some boards for some wired reason. */
        case 0:
        default:
            ppc_mtspr(TCR, ppc_mfspr(TCR) & ~((1<<24) | (1<<25)));
            tickRate = 512; // / ( CLOCK_RATE / 1000000 );
            //tickRate = 3; //(2^9);
            break;
    }

    isEnabled = true;
    ppc_mtspr(TCR, ppc_mfspr(TCR) | PPC_TCR_FIE);

    return cOk;
}

ErrorT PPC405FixedIntervalTimer::disable() {
    isEnabled = false;
    ppc_mtspr(TCR, ppc_mfspr(TCR) & ~PPC_TCR_FIE);

    return cOk;
}

ErrorT PPC405FixedIntervalTimer::tick() {
    this->doHardwareStuffOnTick();

    return TimerDevice::tick();
}
