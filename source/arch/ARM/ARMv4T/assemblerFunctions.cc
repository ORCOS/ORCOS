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
extern "C" void restoreContext(Thread* t, Thread* previousThread) __attribute__((noreturn));
extern "C" void dumpContext(void* sp_context);

/*!
 * Restores the context of a given thread saved at sp.
 *
 * Called whenever a thread is restored and thus will execute.
 * Called from SyscallManager to finish a syscall, SingleCPUDisptacher to resume the execution of a preempted thread,
 * and Resource to finish a syscall earlier.
 *
 */
extern "C" void restoreContext(Thread* t, Thread* previousThread)  {
    void* sp            = 0; /* pointer to the context of the thread */
    unint ptStartAddr   = 0; /* page table start address */
    void* mode          = 0; /* thread mode */

    /* IRQS disabled is assumed! */

    ASSERT(t != 0);
    if (!isOk(t->popStackPointer(mode)))
        ERROR("Restore Context failed while popping the stack for context restore Mode!");

    if (!isOk(t->popStackPointer(sp)))
        ERROR("Restore Context failed while popping the stack for stack pointer!");

    ASSERT(sp != 0);
    ASSERT(t->getOwner() != 0);
    unint1 pid = 0xff; /* -1 == no pid change */
    if ((previousThread == 0) || (previousThread->getOwner() != t->getOwner()))
    {
        pid = t->getOwner()->getId() & 0xff; /* new pid to change to */
    }

#if HAS_Board_HatLayerCfd
    ptStartAddr =  (((unint) &__PageTableSec_start) + pid * 0x4000);

    /*
     * On context restore we must invalidate the branch prediction array as
     * otherwise we will get prefetch aborts due to address space change
     * Also see arm architecture reference manual B2.2.6
     * "In some implementations, to ensure correct operation it might be necessary to invalidate branch prediction
     *  entries on a change of instruction or instruction address mapping"
     */

// #define INVALIDATE_BRANCH_PREDICTION_ON_CONTEXT_SWITCH

#endif

    LOG(HAL, TRACE, "Restore Context: t: %x, sp@ 0x%x, mode:%x", t, sp, mode);

    /* check if if this thread is waiting on a signal value */
    int signalvalue = t->signalvalue;
    int pass_signal = t->signal != 0;
    if (pass_signal) {
        t->signal = 0;
    }

    asm volatile(
            /* thumb interwork code to branch to arm mode */
            ".align 4;"
            "mov    r0, pc;"
            "bx     r0;"
            ".code  32;"
            /* arm mode code */

#if HAS_Board_HatLayerCfd
            "CMP    %2, 0xff;"
            "BEQ    1f;"
            "MOV    r1, %2;" /* set PID */
            "ORR    r1, r1, %2, lsl #8;"
            "MOV    r0, #0;"
            "MCR    p15, 0, r0, c13, c0, 1;"    /* set ASID and PROCID field of 0 for ASID and TTBR change */
            "ISB;"
            //"MCR    p15, 0, r0, c7 , c5, 4;"  /* Ensure completion of the CP15 write (if ISB not working) */
            "MCR    p15, 0, %1, c2 , c0, 0;"    /* set TTBR0 */
            "MCR    p15, 0, r1, c13, c0, 1;"    /* set ASID and PROCID field of CONTEXTIDR register */
            "ISB;"
            //"MCR    p15, 0, r0, c7 , c5, 4;"  /* Ensure completion of the CP15 write (if ISB not working) */
#if  INVALIDATE_BRANCH_PREDICTION_ON_CONTEXT_SWITCH
            "MCR p15, 0, r0, c7 , c5, 6;"       /* invalidate whole branch predictor array */
#endif
#endif
            "1:"
            "CMP    %4, #1;"                    /* check if we need to return the signal value */
            "STREQ  %5, [%0, #4];"              /* if so put into return register r0*/
            "MOV    r0, %0;"                    /* load context address*/
            "MOV    r1, %3;"                    /* set restore context mode */

            /*LDR    sp, =__stack - 0x20;"     // temporary accessible stack position for dumpContext
             "push {r0-r3};"
             "bl  dumpContext;"
             "pop  {r0-r3};"*/

            "b         restoreThreadContext;"
            :
            : "r" (sp), "r" (ptStartAddr), "r" (pid) ,"r" (mode) , "r" (pass_signal), "r" (signalvalue)
            : "r0", "r1", "r2" , "r3"
    );

    __builtin_unreachable();
}

