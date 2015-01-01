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

#include "inc/types.hh"
#include "inc/error.hh"
#include "inc/sprintf.hh"
#include "kernel/Kernel.hh"

#include Kernel_Thread_hh

extern Kernel* theOS;


/*****************************************************************************
 * Method: kwait(int milliseconds)
 *
 * @description
 *  Kernel wait function. Uses the Board Clock to wait for a given time
 *  in milliseconds. This is a blocking wait if we can not be preempted.
 *  If no clock has been configured yet a simple for
 *  loop with 100000 executions is used to provide
 *  some delay.
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
void kwait(int milliseconds) {
    /* check if we have a board and clock
     if not we are probably initializing them */
    if ((theOS == 0) || (theOS->getBoard() == 0) || (theOS->getClock() == 0)) {
        /* if the clock is not set we emulate this ..
         * however not nearly close to milliseconds here...*/
        volatile unint4 i = 0; /* volatile so this is not optimized out  */
        while (i < 100000) i++;
    } else {
        volatile TimeT now = theOS->getClock()->getClockCycles();
        while (theOS->getClock()->getClockCycles() < (now + (milliseconds MILLISECONDS))) { }
    }
}

/*****************************************************************************
 * Method: kwait_us(int us)
 *
 * @description
 *  Busy waits the given number of microseconds.
 *  If no clock has been configured yet a simple for
 *  loop with 10000 executions is used to provide
 *  some delay.
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
void kwait_us(int us) {
    /* check if we have a board and clock
     if not we are probably initializing them */
    if ((theOS == 0) || (theOS->getBoard() == 0) || (theOS->getClock() == 0)) {
        /* if the clock is not set we emulate this ..
         * however not nearly close to us here...*/
        volatile unint4 i = 0; /* volatile so this is not optimized out  */
        while (i < 10000) i++;
    } else {
        volatile unint8 now = theOS->getClock()->getClockCycles();
        while (theOS->getClock()->getClockCycles() < (now + (us MICROSECONDS))) { }
    }
}


/*****************************************************************************
 * Method: kwait_us_nonmem(unint4 useconds )
 *
 * @description
 *  Busy waits the given number of microseconds ensuring that
 *  memory bus congestion stays small.
 *
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
void kwait_us_nonmem(int us) {
    /* check if we have a board and clock
     if not we are probably initializing them */
    if ((theOS == 0) || (theOS->getBoard() == 0) || (theOS->getClock() == 0)) {
        /* if the clock is not set we emulate this ..
         * however not nearly close to us here...*/
        volatile unint4 i = 0; /* volatile so this is not optimized out  */
        while (i < 10000) i++;
    } else {
        volatile unint8 now = theOS->getClock()->getClockCycles();

        while (theOS->getClock()->getClockCycles() < now + us MICROSECONDS) {
        /* do some looping on registers to avoid memory bus congestion*/
            for (volatile int i = 0; i < 100; i++) {
            }
        }
    }
}
