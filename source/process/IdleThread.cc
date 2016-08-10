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
void IdleThread::run()
{
    _enableInterrupts();

    /* loop forever */
    while (true)
    {
        theOS->getMemoryManager()->idleEnter();
#ifdef HAS_Kernel_PowerManagerCfd
        /* Kernel has PowerManager configured. hand over control
         * to it to idle the system */
        theOS->getPowerManager()->enterIdleThread();
#endif
        // spend some time doing nothing to avoid calling idleEnter too often
        //for (int i = 0; i < 500000; i++) { NOP; }
    }
}

void IdleThread::callbackFunc(void* param) {
    // just call run to enter endless loop
    LOG(KERNEL, INFO, "Idle thread running");
    run();
}
