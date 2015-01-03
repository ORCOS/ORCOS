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

#include "Leon3Clock.hh"
#include "assemblerFunctions.hh"
#include "asm/Leon3.hh"
#include <assembler.h>
#include "kernel/Kernel.hh"

extern Kernel* theOS;

Leon3Clock::Leon3Clock( const char* name ) :
    Clock( name ) {
    baseAddr = (unsigned int) 0x80000310;

    totalTime = 0;

    //set reload value
    OUTW(baseAddr + RELOAD_REG_OFFSET, 0xFFFFFFFF);

    //set value register
    OUTW(baseAddr + VALUE_REG_OFFSET, 0xFFFFFFFF);

    //enable timer and timer interrupt
    OUTW(baseAddr + CTRL_REG_OFFSET, TCR_EN | TCR_RS | TCR_LD | TCR_IE);
}

Leon3Clock::~Leon3Clock() {
}
/*
bool Leon3Clock::getTimeSinceStartup( TimeStruct* ts ) {

    unint8 time = 0;

    // update the current Time
    time = this->getTimeSinceStartup();

    // calculate elapsed time in us

    // break time down to the datatypes provided from
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

void Leon3Clock::reset() {

    //set reload value
        OUTW(baseAddr + RELOAD_REG_OFFSET, 0xFFFFFFFF);

        //set value register
        OUTW(baseAddr + VALUE_REG_OFFSET, 0xFFFFFFFF);

        //enable timer and timer interrupt
        OUTW(baseAddr + CTRL_REG_OFFSET, TCR_EN | TCR_RS | TCR_LD | TCR_IE);
}

unint8 Leon3Clock::getTimeSinceStartup() {
    unint8 time = 0;

    // update the current Time
    assembler::readTimeRegister( (unint4*)&time );

    time = ((unint4)0xFFFFFFFF) - time;

    time = time * ((CLOCK_RATE / 1000000) -1);

    // calculate the elapsed time in us
    time += totalTime;


    return time;
}

void Leon3Clock::updateTotalTime(){
    unint4 time = 0;
    asm volatile (
            "set 0x80000314, %%g1;"
            "ld [%%g1], %0;"
            : //no output variables
            : "r" (time)
            : "g1" //clobber list
            );

    totalTime += time;

}
