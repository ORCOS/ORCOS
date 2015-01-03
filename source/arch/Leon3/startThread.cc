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


#include "SCLConfig.hh"
#include "assemblerFunctions.hh"
#include <process/Task.hh>

#include "kernel/Kernel.hh"
extern Kernel* theOS;

#ifdef AIS
#include <FailureMonitor.hh>
#endif

/*!
 *  This method will jump to the addr given by the effective addr while
 *  setting the correct stack pointer
 *
 *  thread - the thread we want to start for the first time
 *
 */
void startThread(Thread* thread) {

    // used variables declarations. put them into registers for efficiency if possible
    void*         stack_addr  = thread->threadStack.endAddr;
    void*         addr        = thread->getStartRoutinePointer();
    void*         returnaddr  = thread->getExitRoutinePointer();
    void*         arguments   = thread->getStartArguments();

#ifdef AIS
    thread->pArg = (void*) AISgetErrorCounter();
#endif

    ASSERT(stack_addr);
    ASSERT(addr);
    ASSERT(returnaddr);

   //LOG(KERNEL,INFO,(KERNEL,INFO,"startThread: arg:0x%x stack:0x%x",arguments,stack_addr));

    TaskIdT pid = thread->getOwner()->getId();

    asm(
        /* disable traps */
        "rd %%psr, %%l0;"
        "mov     0x00000020, %%l1;"
        "not    %%l1;"
        "and     %%l0, %%l1, %%l0;"
        "mov     %%l0, %%psr;"
        "nop; nop; nop;"                    /* write delay */

#ifdef HAS_Board_HatLayerCfd
        /* set mmu context to pid */
        "set     0x200, %%g7;"
        "sta     %4, [%%g7] 0x19;"
        /* flush cache */
        "flush;"
        "set 0x81000f, %%g7;"
        "sta %%g7, [%%g0] 2;"
#endif
        /* set WIM to the CWP */
        "set 1, %%g6;"
        "sll %%g6, %%l0, %%g6;"
        /* rotate WIM one bit to the left */
        "srl %%g6, 7, %%g5;"
        "sll %%g6, 1, %%g7;"
        "or    %%g5, %%g7, %%g6;"
        "wr %%g6, %%wim;"
        "nop;nop;nop;"

        "mov    %0   , %%sp;"                /* load the stack pointer into the stack register */
        "mov    %3     , %%i0;"                /* set the arguments */
        "mov    %3     , %%o0;"                /* set the arguments */
        "rd     %%psr, %%l0;"

        /*
         * first disable traps, then enable interrupts. No interrupts are taken if traps are disabled.
         */

        /* Enable Interrupts */
        "mov     0x00000F00, %%l1;"
        "not    %%l1;"
        "and     %%l0, %%l1, %%l0;"
        //"mov    0x80, %%l1;"
        //"or     %%l0, %%l1, %%l0;"
        "mov     %%l0, %%psr;"
        "nop; nop; nop;"                    /* write delay */

        /* write the return addr into o7 (return address used by call/ret instruction) */
        "mov    %2, %%o7;"
        "sub    %%o7, 8, %%o7;"
        "mov  %1, %%g7;"

        "save;"
        "jmpl %%g7, %%g0;"
        "rett %%g7+4;"

        : // no output variables
        : "r" (stack_addr) , "r" (addr), "r" (returnaddr), "r" (arguments), "r" (pid) // input variables
        : "%l0", "%l1"
       );

  // this point is never reached since the thread should be executing after jmp;
}
