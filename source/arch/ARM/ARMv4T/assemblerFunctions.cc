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

#define STACK_CONTENT(sp,offset) *( (long*) ( ( (long) sp) + offset)  )

extern Kernel* theOS;
extern "C" void restoreContext(Thread*  t)  __attribute__((noreturn));
extern "C" void dumpContext(void* sp_context);

/*!
 * Restores the context of a given thread saved at sp.
 *
 * Called whenever a thread is restored and thus will execute.
 * Called from SyscallManager to finish a syscall, SingleCPUDisptacher to resume the execution of a preempted thread,
 * and Resource to finish a syscall earlier.
 *
 */
extern "C" void restoreContext(Thread*  t)
{
    void* sp = 0;
    void* ptStartAddr = 0;
    void* mode = 0;

    if (t == 0) {
    	 ERROR("Thread t == 0!");
    }

    if (!isOk(t->popStackPointer(mode)))
	{
		 ERROR("Restore Context failed while popping the stack for context restore Mode!");
	}

    if (!isOk(t->popStackPointer(sp)))
	{
		 ERROR("Restore Context failed while popping the stack for stack pointer!");
	}

    ASSERT(sp == 0);
    if (t->getOwner() == 0) {
		 ERROR("Thread Owner == 0!");
    }

    TaskIdT pid = t->getOwner()->getId();

    LOG(HAL,DEBUG,(HAL,DEBUG,"Restore Context: t: %x, sp@ 0x%x, mode:%d" , t,sp, mode));

    //LOG(HAL,WARN,(HAL,WARN,"Return Value: %d" , t->returnValue ));

#if HAS_Board_HatLayerCfd
	ptStartAddr = (void*) ((unint)&__PageTableSec_start + pid*0x4000);
#endif

    #if USE_SAFE_KERNEL_STACKS
        // we use safe kernel stacks so we must free the stack slot
        // if we are returning to user space!
           int2 myBucketIndex =  t->getKernelStackBucketIndex();
           FREE_KERNEL_STACK_SLOT(myBucketIndex);
    #endif

    // allow new interrupts
    OUTW(MPU_INTCPS_CONTROL, 0x1);

	asm volatile(
		// thumb to arm mode code
		".align 4;"
		"mov    r0, pc;"
		"bx     r0;"
		".code  32;"
		// arm mode code

		"MOV r0, %0;"	// set saved context address
		"MOV r1, %2;"	// set PID

#if HAS_Board_HatLayerCfd
		"MOV r2, #0x0;"
		"ORR r1, r1, %2, lsl #8;"
		"MCR p15, 0, r1, c13, c0, 1;"	// set ASID and PROCID field of CONTEXTIDR register
		"MCR p15, 0, r2, c7, c5, 4;"	// Ensure completion of the CP15 write (ISB not working)
		"MCR p15, 0, %1, c2, c0, 0;"	// set TBBR0
		"MCR p15, 0, r2, c7, c5, 4;"	// Ensure completion of the CP15 write (ISB not working)

		"MCR p15, 0, r0, c7 , c5, 0;" // invalidate whole instruction cache
#endif

		//"bl  dumpContext;"
		"MOV r0, %0;"
		"MOV r1, %3;"	// set restore context mode
		"str %4, [%0, #4];"
		"b 	 restoreThreadContext;"
		:
		: "r" (sp), "r" (ptStartAddr), "r" (pid) ,"r" (mode) ,"r" (t->returnValue)
		: "r0", "r1", "r2"
	);



    while(1);
}
