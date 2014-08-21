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

#include "assemblerFunctions.hh"
#include <process/Task.hh>
#include "kernel/Kernel.hh"
#include "inc/error.hh"
#include "assembler.h"
#include "inc/memio.h"

extern Kernel* theOS;
extern "C" void restoreContext(Thread* t) __attribute__((noreturn));
extern "C" void dumpContext(void* sp_context);

/*!
 * Restores the context of a given thread saved at sp.
 *
 * Called whenever a thread is restored and thus will execute.
 * Called from SyscallManager to finish a syscall, SingleCPUDisptacher to resume the execution of a preempted thread,
 * and Resource to finish a syscall earlier.
 *
 */
extern "C" void restoreContext(Thread* t) {
    void* sp = 0;
    void* ptStartAddr = 0;
    void* mode = 0;

    ASSERT(t != 0);

    if (!isOk(t->popStackPointer(mode)))
        ERROR("Restore Context failed while popping the stack for context restore Mode!");

    if (!isOk(t->popStackPointer(sp)))
        ERROR("Restore Context failed while popping the stack for stack pointer!");

    ASSERT(sp != 0);
    ASSERT(t->getOwner() != 0);
    TaskIdT pid = t->getOwner()->getId();

#if HAS_Board_HatLayerCfd
#if ENABLE_BRANCH_PREDICTION
    /*
     * On Context Restore we must invalidate the branch prediction array as
     * otherwise we may get invalid instruction aborts..
     */
    LOG(HAL,TRACE,"flushing branch prediction array!");
    asm volatile(
            "MCR p15, 0, r0, c7 , c5, 6;"  // invalidate whole branch predictor array
            :
            :
            : "r0"
    );
#endif
#endif

    // indicate no process change from now on until
    // we really dispatch to avoid instruction cache to be flushed again

    LOG(HAL, TRACE, "Restore Context: t: %x, sp@ 0x%x, mode:%x" , t,sp, mode);

#if HAS_Board_HatLayerCfd
    ptStartAddr = (void*) (((unint) &__PageTableSec_start) + pid * 0x4000);
#endif

    int signalvalue = t->signalvalue;
    int pass_signal = t->signal != 0;
    if (pass_signal)
    {
        t->signal = 0;
    }

    asm volatile(
            // thumb to arm mode code
            ".align 4;"
            "mov    r0, pc;"
            "bx     r0;"
            ".code  32;"
            // arm mode code

            "MOV r0, %0;"// set saved context address
            "MOV r1, %2;"// set PID

#if HAS_Board_HatLayerCfd
            "MOV r2, #0x0;"
            "ORR r1, r1, %2, lsl #8;"
            "MCR p15, 0, r1, c13, c0, 1;"  // set ASID and PROCID field of CONTEXTIDR register
            //"ISB;"
            "MCR p15, 0, r2, c7, c5, 4;"// Ensure completion of the CP15 write (ISB not working)
            "MCR p15, 0, %1, c2, c0, 0;"// set TBBR0
            //"ISB;"
            "MCR p15, 0, r2, c7, c5, 4;"// Ensure completion of the CP15 write (ISB not working)

#endif
            "LDR	sp, =__stack - 0x20;"  // temporary accessible stack position for jump to task

            "MOV r0, %0;"
            "MOV r1, %3;"// set restore context mode

            "CMP   %4,1;"
            "STREQ %5, [%0, #4];"

            /*"push {r0-r3};"
             "bl  dumpContext;"
             "pop  {r0-r3};"*/

            "b 	 restoreThreadContext;"
            :
            : "r" (sp), "r" (ptStartAddr), "r" (pid) ,"r" (mode) , "r" (pass_signal), "r" (signalvalue)
            : "r0", "r1", "r2" , "r3"
    );

    while (1)
        ;
}

