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

#include "IdleThread.hh"
#include "kernel/Kernel.hh"
#include "SCLConfig.hh"
#include "assemblerFunctions.hh"

extern Kernel* theOS;
extern void* __stack;

IdleThread::IdleThread() {
}

IdleThread::~IdleThread() {
}

/*****************************************************************************
 * Method: IdleThread::run()
 *
 * @description
 *  Simple idle loop that can be preempted at any time.
 *---------------------------------------------------------------------------*/
void IdleThread::run() {
    /* reset the stack pointer to the context address since we don't want to waste memory
     * and cannot access the stack from the previous thread */
    SETSTACKPTR(&__stack);
    SETPID(0);

    theOS->getMemoryManager()->idleEnter();

    _enableInterrupts();

    /* loop forever */
    while (true) {
#ifdef HAS_Kernel_PowerManagerCfd
        /* Kernel has PowerManager configured. hand over control
         * to it to idle the system */
        theOS->getPowerManager()->enterIdleThread();
#else
        /* this nop is used to ensure that the endless loop is not optimized out by the compiler */
        NOP;
#endif
    }
}
