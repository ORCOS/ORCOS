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

#include <process/Task.hh>

unint4 GPIO_VAL = 0;

void startThread( Thread* thread )  __attribute__((noreturn));

/*!
 *  This method will jump to the addr given by the effective addr while
 *  setting the correct PID for the MMU and the correct stack pointer
 *
 *  thread - the thread we want to start for the first time
 *
 */
void startThread( Thread* thread ) {
    // used variables declarations. put them into registers for efficiency if possible
    // set the initial stack address. The IRQStackPointerAdress has been set to the top of the stack before.

    void* stack_addr = thread->threadStack.endAddr;
    TaskIdT PIDvar = thread->getOwner()->getId();
    void* addr = thread->getStartRoutinePointer();
    void* returnaddr = thread->getExitRoutinePointer();
    void* arguments = thread->getStartArguments();

/*
 *  This is for logging time points with an external logic analyzer
 *   GPIO_VAL = ((1UL << (29 + thread->getId() )));
 *   *((volatile unsigned*) 0x90230000) = 	0;
 *   *((volatile unsigned*) 0x90230000) = 	GPIO_VAL;
 *   *((volatile unsigned*) 0x90230000) = 	0;
 */

    ASSERT(stack_addr);
    ASSERT(addr);
    ASSERT(returnaddr);

    asm volatile(

#ifdef HAS_MemoryManager_HatLayerCfd
            "mtspr 945,%0;" // Set PID register
            "sync;"
            "isync;"
#endif

            "mr   	%%r1, %1;" /* load the stack pointer into the stack register*/
            "mtspr  %%lr, %3;" /* write the return addr into the link register*/

            // set the arguments
            "mr     %%r3, %4;"

            "mfmsr %%r2;"
            // MSR[PR] = MSR[EE] = MSR[DE] = 1
            "ori   %%r2,%%r2,0xC000@l;"
            "oris  %%r2,%%r2,0xC000@h;"

            //"ori   %%r2,%%r2,0xD200@l;"
            //"oris  %%r2,%%r2,0xD200@h;"

            "mtsrr0  %2;" /* write the start routine addr to SRR0 */
            "mtsrr1  %%r2;" /* write MSR to SRR1 */

            "rfi;" /* set MSR to SRR1 and PC to SRR0*/
            : // no output variables
            : "r" (PIDvar) , "r" (stack_addr) , "r" (addr), "r" (returnaddr), "r" (arguments) // input variables
            : // no clobber list
    );

    // this point is never reached.
    while ( true ) {
    }
}
