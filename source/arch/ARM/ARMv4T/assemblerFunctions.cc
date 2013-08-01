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
		 ERROR("Restore Context failed while popping the stack for return PSR Mode!");
	}

    if (!isOk(t->popStackPointer(sp)))
	{
		 ERROR("Restore Context failed while popping the stack for stack pointer!");
	}

    ASSERT(sp == 0);

    TaskIdT pid = t->getOwner()->getId();

    LOG(HAL,TRACE,(HAL,TRACE,"Restore Context: t: %x, sp@ 0x%x, mode:%d" , t,sp, mode));

#ifdef HAS_MemoryManager_HatLayerCfd
	ptStartAddr = (void*) ((unint)&__PageTableSec_start + pid*0x4000);
#endif

    LOG(HAL,TRACE,(HAL,TRACE,"Restore Context: t: %x, sp@ 0x%x, mode:%d" , t,sp, mode));

    #if USE_SAFE_KERNEL_STACKS
        // we use safe kernel stacks so we must free the stack slot
        // if we are returning to user space!
           int2 myBucketIndex =  t->getKernelStackBucketIndex();
           FREE_KERNEL_STACK_SLOT(myBucketIndex);
    #endif

    // allow new interrupts
    OUTW(MPU_INTCPS_CONTROL, 0x1);


    if ((int)mode == 1) {
    	/*if (( (*(unint4*)sp) & 0x1f) != 16 ) {
    		//PSR corrupted
    		ERROR("PSR Corrupt of returning context!");
    		//TODO: handle the error inside the assembler part
    	}*/

		asm volatile(

				// set stack pointer
				"MOV    r0, %0;"

				"MOV r2, #0x0;"

				"MOV r1, %2;"
				"ORR r1, r1, %2, lsl #8;"
				"MCR p15, 0, r1, c13, c0, 1;"	// set ASID and PROCID field of CONTEXTIDR register
				"MCR p15, 0, r2, c7, c5, 4;"	// Ensure completion of the CP15 write (ISB not working)

				"MCR p15, 0, %1, c2, c0, 0;"	// set TBBR0
				"MCR p15, 0, r2, c7, c5, 4;"	// Ensure completion of the CP15 write (ISB not working)


				"b 		restoreUserContext;"
				:
				: "r" (sp), "r" (ptStartAddr), "r" (pid)
				: "r0" ,"r1","r2"
		);
    } else

    if ((int)mode == 2) {
    	/*if (( (*(unint4*)sp) & 0x1f) != 19 ) {
    		//PSR corrupted
    		ERROR("PSR Corrupt of returning context!");
    		//TODO: handle the error inside the assembler part
    	}*/

    	asm volatile(

			// set stack pointer
			"MOV    r0, %0;"

			"MOV r2, #0x0;"

			"MOV r1, %2;"
			"ORR r1, r1, %2, lsl #8;"
			"MCR p15, 0, r1, c13, c0, 1;"	// set ASID and PROCID field of CONTEXTIDR register
			"MCR p15, 0, r2, c7, c5, 4;"	// Ensure completion of the CP15 write (ISB not working)

			"MCR p15, 0, %1, c2, c0, 0;"	// set TBBR0
			"MCR p15, 0, r2, c7, c5, 4;"	// Ensure completion of the CP15 write (ISB not working)

			"b 		restoreSVCContext;"
			:
    		: "r" (sp), "r" (ptStartAddr), "r" (pid)
    		: "r0" ,"r1","r2"
    	);

    }


    memdump((unint4) t, sizeof(Thread) / 4 +1);

	ERROR("Returning Mode unknown!");
	// TODO forwar error to robust/TaskErrorHandler -> implent policy for this error there

    while(1);
}
