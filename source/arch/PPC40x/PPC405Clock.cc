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

#include "PPC405Clock.hh"
#include "assemblerFunctions.hh"

PPC405Clock::PPC405Clock( ) : Clock( "clock" ) {
    reset();
}

PPC405Clock::~PPC405Clock() {
}

void PPC405Clock::reset() {
    asm volatile (
            "li        %%r5, 0;" // load 0 into register 5
            "mtspr  285, %%r5;" // reset TBU (set to 0)
            "mtspr  284, %%r5;" // reset TBL (set to 0)
            : // no output variables
            : // no input variables
            : "r5" // these registers get altered during calc
    );
}

unint8 PPC405Clock::getTimeSinceStartup() {
    register unint8 time asm("r3");

    // read time base into the return register r3 and r4
    asm volatile (
                "overflow:"
                "mftb   %%r3, 269;" // read TBU
                "mftb   %%r4, 268;" // read TBL
                "mftb   %%r11, 269;" // read TBU again
                "cmpw   %%r3, %%r11;" // compare to make sure that TBL did not overflow between reads
                "bne- overflow;"    // reread values in the case of overflow of TBL
                : // no output variables
                :
                : "11"
        );

    return time;
}
