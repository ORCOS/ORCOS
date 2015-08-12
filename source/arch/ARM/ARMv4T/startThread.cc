/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2010 University of Paderborn

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

#include "SCLConfig.hh"
#include "assemblerFunctions.hh"
#include "process/Task.hh"
#include "kernel/Kernel.hh"
#include "memtools.hh"

extern Kernel* theOS;
extern void* __PageTableSec_start;
extern void* __stack;

/*****************************************************************************
 * Method: startThread(Thread* thread)
 *
 * @description
 *  This method will jump to the address given by the threads start routine pointer while
 *  setting the correct PID for the MMU and the correct stack pointer
 *
 * @params
 *   thread - the thread we want to start for the first time
 *******************************************************************************/
void startThread(Thread* thread) __attribute__((noreturn));

void startThread(register Thread* thread) {
    /* used variables declarations. put them into registers to ensure the varibles
     * to be accessible after SETPID (stack is not accessible any more) */

    register TaskIdT PIDvar     = thread->getOwner()->getId();
    register void* addr         = thread->getStartRoutinePointer();
    register void* returnaddr   = thread->getExitRoutinePointer();
    register void* arguments    = thread->getStartArguments();
    register unint ptStartAddr  = 0;

    ASSERT(addr);
    ASSERT(returnaddr);

    register unint4 stack_addr =  ((unint4) thread->threadStack.endAddr - RESERVED_BYTES_FOR_STACKFRAME);

#if HAS_Board_HatLayerCfd
    ptStartAddr = (((unint) &__PageTableSec_start) + PIDvar * 0x4000);
#endif

    asm volatile(
#if ARM_THUMB == 1
            // thumb to arm jump code
            ".align 4;"    // be sure we are 4 byte aligned for jump to work
            "mov    r0,pc;"// set arm based jump address
            "bx     r0;"   // jump and switch mode
            ".code 32;"    // following code is arm code
#endif

#if HAS_Board_HatLayerCfd
            "MOV    r0, #0x0;"
            "MCR    p15, 0, r0, c13, c0, 1;"    /* set ASID and PROCID field of 0 for asid and ttbr change */
            "MOV    r1, %0;"
            "ORR    r1, r1, %0, lsl #8;"        /* construct CONTEXTIDR register value */
            "ISB;"

            "MCR    p15, 0, r1, c13, c0, 1;"    /* set ASID and PROCID field of CONTEXTIDR register */
            "ISB;"

            "MCR    p15, 0, %5, c2, c0, 0;"     /* set TBBR0 */
            "ISB;"
#endif

            /* switch to system mode to load the right registers */

            "MSR    CPSR_c, #0x1F | 0xC0;"

            /* set stack pointer and link register for user mode */
            "MOV    lr, %3;"      // write the return address into the user link register (returnaddr)
            "MOV    sp, %1;"      // load the stack pointer into the user stack register (stack_addr)
            "AND    r0, sp, #3;"
            "SUB    sp, sp, r0;"  // be sure the sp is 4 byte aligned!

            // switch back to supervisor mode
            "MSR    CPSR_c, #0x13 | 0xC0;"

            /* entry method is always in arm mode */
            "MSR    SPSR, #16;"    // write saved PSR register (SPSR)

            // push task start address on temporary stack
            "MOVW   sp, #:lower16:__stack;"
            "MOVT   sp, #:upper16:__stack;"
            "push   {%2};"
            // set the arguments
            "MOV    r0, %4;"
            // jump to task and switch to user mode
            "LDM    sp, {pc}^;" // do an exception return.. copies SPSR->CPSR
            "NOP;"

            :// no output variables
            : "r" (PIDvar) , "l" (stack_addr) , "r" (addr), "l" (returnaddr), "r" (arguments), "r" (ptStartAddr) // input variables
            : "r0", "r1"// clobber list
    );

    __builtin_unreachable();
}
