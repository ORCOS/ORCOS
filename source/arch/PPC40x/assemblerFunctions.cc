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

#define STACK_CONTENT(sp,offset) *((long*) (((long) sp) + offset))

extern Kernel* theOS;
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
    if (!isOk(t->popStackPointer(sp))) {
        ERROR("Restore Context failed while popping the stack!");
    }

    ASSERT(sp);

    TaskIdT pid = t->getOwner()->getId();

    LOG(PROCESS, TRACE, (PROCESS, TRACE, "Restore Context: sp @  0x%x.", sp));

#if USE_SAFE_KERNEL_STACKS
    // we use safe kernel stacks so we must free the stack slot
    // if we are returning to user space!
    int2 myBucketIndex = t->getKernelStackBucketIndex();
    FREE_KERNEL_STACK_SLOT(myBucketIndex);
#endif

    // print the content of the registers!
    LOG(PROCESS, TRACE, (PROCESS,TRACE,"r0 : 0x%x, r1: 0x%x, r2: 0x%x, r3:0x%x" , STACK_CONTENT(sp,-128),STACK_CONTENT(sp,-124),STACK_CONTENT(sp,-120),STACK_CONTENT(sp,-116)));
    LOG(PROCESS, TRACE, (PROCESS,TRACE,"r4 : 0x%x, r5: 0x%x, r6: 0x%x, r7:0x%x" , STACK_CONTENT(sp,-112),STACK_CONTENT(sp,-108),STACK_CONTENT(sp,-104),STACK_CONTENT(sp,-100)));
    LOG(PROCESS, TRACE, (PROCESS,TRACE,"r8 : 0x%x, r9: 0x%x, r2: 0x%x, r3:0x%x" , STACK_CONTENT(sp,-96),STACK_CONTENT(sp,-92),STACK_CONTENT(sp,-88),STACK_CONTENT(sp,-84)));
    LOG(PROCESS, TRACE, (PROCESS,TRACE,"r12 : 0x%x, r13: 0x%x, r14: 0x%x, r15:0x%x" , STACK_CONTENT(sp,-80),STACK_CONTENT(sp,-76),STACK_CONTENT(sp,-72),STACK_CONTENT(sp,-68)));
    LOG(PROCESS, TRACE, (PROCESS,TRACE,"r16 : 0x%x, r17: 0x%x, r18: 0x%x, r19:0x%x" , STACK_CONTENT(sp,-64),STACK_CONTENT(sp,-60),STACK_CONTENT(sp,-56),STACK_CONTENT(sp,-52)));
    LOG(PROCESS, TRACE, (PROCESS,TRACE,"r20 : 0x%x, r21: 0x%x, r22: 0x%x, r23:0x%x" , STACK_CONTENT(sp,-48),STACK_CONTENT(sp,-44),STACK_CONTENT(sp,-40),STACK_CONTENT(sp,-36)));
    LOG(PROCESS, TRACE, (PROCESS,TRACE,"r24 : 0x%x, r25: 0x%x, r26: 0x%x, r27:0x%x" , STACK_CONTENT(sp,-32),STACK_CONTENT(sp,-28),STACK_CONTENT(sp,-24),STACK_CONTENT(sp,-20)));
    LOG(PROCESS, TRACE, (PROCESS,TRACE,"r28 : 0x%x, r29: 0x%x, r30: 0x%x, r31:0x%x" , STACK_CONTENT(sp,-16),STACK_CONTENT(sp,-12),STACK_CONTENT(sp,-8),STACK_CONTENT(sp,-4)));
    LOG(PROCESS, TRACE, (PROCESS,TRACE,"LR : 0x%x, CR: 0x%x, SRR1: 0x%x, SRR0:0x%x" , STACK_CONTENT(sp,-136),STACK_CONTENT(sp,-140),STACK_CONTENT(sp,-144),STACK_CONTENT(sp,-148)));

    asm volatile(
            // set process id so we can access the stack
            "   mtspr 945,%1;"
            "   sync;"
            "   isync;"
            // Set the stack pointer to sp
            "    mr    %%r1,%0;"
            // branch to the leave handler function
            "    ba    _leaveHandler;"
            :
            : "r" (sp), "r" (pid)
            :// no clobber list needed here for r1 since context is restored
    );
}
