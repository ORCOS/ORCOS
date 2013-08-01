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

PPC405Clock::PPC405Clock( const char* name ) :
    Clock( name ) {

	reset();
}

PPC405Clock::~PPC405Clock() {
}

/*
bool PPC405Clock::getTimeSinceStartup( TimeStruct* ts ) {
    unint8 time = 0; // make sure it fits


    // update the current Time
    assembler::readTimeRegister( &time );

    // calculate elapsed time in us
    time = time / ( CLOCK_RATE / 1000000 );

    // break time down to the datatyped provided from
    // TimeStruct
    ts->years = time / ( 1000 * 1000 * 60 * 60 * 24 * 365 );
    time -= ts->years * ( 1000 * 1000 * 60 * 60 * 24 * 365 );

    ts->days = time / ( 1000 * 1000 * 60 * 60 * 24 );
    time -= ts->days * ( 1000 * 1000 * 60 * 60 * 24 );

    ts->hours = time / ( 1000 * 1000 * 60 * 60 );
    time -= ts->hours * ( 1000 * 1000 * 60 * 60 );

    ts->minutes = time / ( 1000 * 1000 * 60 );
    time -= ts->minutes * ( 1000 * 1000 * 60 );

    ts->seconds = time / ( 1000 * 1000 );
    time -= ts->seconds * ( 1000 * 1000 );

    ts->milliseconds = time / ( 1000 );
    time -= ts->milliseconds * ( 1000 );

    ts->microseconds = time;

    return true;
}*/

void PPC405Clock::reset() {
    asm volatile (
            "li		%%r5, 0;" // load 0 into register 5
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
