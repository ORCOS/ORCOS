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
#include <process/Task.hh>
#include "kernel/Kernel.hh"
#include "memtools.hh"

extern Kernel* theOS;

extern void* __PageTableSec_start;
extern void* __stack;
void startThread( Thread* thread )  __attribute__((noreturn));


/*!
 *  This method will jump to the addr given by the effective addr while
 *  setting the correct PID for the MMU and the correct stack pointer
 *
 *  thread - the thread we want to start for the first time
 *
 */
void startThread( register Thread* thread ) {

	// used variables declarations. put them into registers to ensure the varibles
	// to be accessable after SETPID (stack is not accessible any more)

	register TaskIdT PIDvar = thread->getOwner()->getId();
    register void* addr = thread->getStartRoutinePointer();
    register void* returnaddr = thread->getExitRoutinePointer();
    register void* arguments = thread->getStartArguments();
    register void* ptStartAddr = 0;


    ASSERT(addr);
    ASSERT(returnaddr);

    SETPID(PIDvar);

    // check whether the stack for this thread needs to be allocated
    // stack allocation might have been delayed up to this point
    // as we might have been impossible to change the context (PID) to this task
    if (thread->threadStack.startAddr == 0) {
    	int stack_size = (int) thread->threadStack.endAddr;
    	thread->threadStack.startAddr = thread->getMemManager()->alloc( stack_size + RESERVED_BYTES_FOR_STACKFRAME, true );
    	thread->threadStack.endAddr = (void*) ( (byte*) thread->threadStack.startAddr + stack_size );
    }
    // get stack address
    register void* stack_addr = (void*) ((unint4)thread->threadStack.endAddr-16);


#ifdef HAS_MemoryManager_HatLayerCfd
	ptStartAddr = (void*) ((unint)&__PageTableSec_start + PIDvar*0x4000);
#endif

    asm volatile(

    #ifdef HAS_MemoryManager_HatLayerCfd
    		"MOV r0, #0x0;"

    		"MOV r1, %0;"
			"ORR r1, r1, %0, lsl #8;"
			"MCR p15, 0, r1, c13, c0, 1;"	// set ASID and PROCID field of CONTEXTIDR register
			"MCR p15, 0, r0, c7, c5, 4;"	// Ensure completion of the CP15 write (ISB not working)

    		"MCR p15, 0, %5, c2, c0, 0;"	// set TBBR0
			"MCR p15, 0, r0, c7, c5, 4;"	// Ensure completion of the CP15 write (ISB not working)
    #endif


				// switch to system mode (cps instruction not working here) to load the right registers
				"MSR	CPSR_c, #0x1F | 0xC0;"

    			// set stack pointer and link register for user mode
				"mov 	lr, %3;" 				// write the return address into the user link register (returnaddr)
				"mov	sp, %1;" 				// load the stack pointer into the user stack register (stack_addr)
    			"and    r0, sp, #3;"
    			"sub    sp, sp, r0;"			// be sure the sp is 4 byte aligned!


				// switch to back to supervisor mode
				"MSR	CPSR_c,#0x13 | 0xC0;"
    			"MSR	SPSR, #16;"				// write saved PSR register (SPSR)

    			// push task start address on stack
    		    "push     {%2};"
    			// set the arguments
    			"MOV 	r0, %4;"
				// jump to task and switch to user mode
    			"LDM      sp!, {pc}^;"		// do a exception return.. copies SPSR->CPSR

                : // no output variables
                : "r" (PIDvar) , "r" (stack_addr) , "r" (addr), "r" (returnaddr), "r" (arguments), "r" (ptStartAddr) // input variables
                : "r0", "r1" // clobber list
        );

    // this point is never reached.
    while ( true ) {
    }
}
