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

#include <kernel/Kernel.hh>
#include "syscalls/handle_syscalls.hh"
#include "inc/error.hh"

#define STACK_CONTENT(sp_int,offset) *( (long*) ( ( (long) sp_int) + offset)  )

extern Kernel* theOS;
extern Board_TimerCfdCl* theTimer;

extern "C"void handleSyscallIRQ(void* sp_int, void* myKernelStackBucketIndex)
{
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"Syscall Interrupt!"));
    // print the content of the registers!
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"r0 : 0x%x, r1: 0x%x, r2: 0x%x, r3:0x%x" , STACK_CONTENT(sp_int,-128),STACK_CONTENT(sp_int,-124),STACK_CONTENT(sp_int,-120),STACK_CONTENT(sp_int,-116)));
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"r4 : 0x%x, r5: 0x%x, r6: 0x%x, r7:0x%x" , STACK_CONTENT(sp_int,-112),STACK_CONTENT(sp_int,-108),STACK_CONTENT(sp_int,-104),STACK_CONTENT(sp_int,-100)));
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"r8 : 0x%x, r9: 0x%x, r2: 0x%x, r3:0x%x" , STACK_CONTENT(sp_int,-96),STACK_CONTENT(sp_int,-92),STACK_CONTENT(sp_int,-88),STACK_CONTENT(sp_int,-84)));
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"r12 : 0x%x, r13: 0x%x, r14: 0x%x, r15:0x%x" , STACK_CONTENT(sp_int,-80),STACK_CONTENT(sp_int,-76),STACK_CONTENT(sp_int,-72),STACK_CONTENT(sp_int,-68)));
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"r16 : 0x%x, r17: 0x%x, r18: 0x%x, r19:0x%x" , STACK_CONTENT(sp_int,-64),STACK_CONTENT(sp_int,-60),STACK_CONTENT(sp_int,-56),STACK_CONTENT(sp_int,-52)));
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"r20 : 0x%x, r21: 0x%x, r22: 0x%x, r23:0x%x" , STACK_CONTENT(sp_int,-48),STACK_CONTENT(sp_int,-44),STACK_CONTENT(sp_int,-40),STACK_CONTENT(sp_int,-36)));
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"r24 : 0x%x, r25: 0x%x, r26: 0x%x, r27:0x%x" , STACK_CONTENT(sp_int,-32),STACK_CONTENT(sp_int,-28),STACK_CONTENT(sp_int,-24),STACK_CONTENT(sp_int,-20)));
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"r28 : 0x%x, r29: 0x%x, r30: 0x%x, r31:0x%x" , STACK_CONTENT(sp_int,-16),STACK_CONTENT(sp_int,-12),STACK_CONTENT(sp_int,-8),STACK_CONTENT(sp_int,-4)));
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"LR : 0x%x, CR: 0x%x, SRR1: 0x%x, SRR0:0x%x" , STACK_CONTENT(sp_int,-136),STACK_CONTENT(sp_int,-140),STACK_CONTENT(sp_int,-144),STACK_CONTENT(sp_int,-148)));


    ASSERT(isOk(pCurrentRunningThread->pushStackPointer(sp_int)));

#if USE_SAFE_KERNEL_STACKS
    // store the used stack bucket index
    pCurrentRunningThread->setKernelStackBucketIndex((int)myKernelStackBucketIndex);
#endif

#if ENABLE_NESTED_INTERRUPTS
    // we want nested interrupts so enable interrupts again
    _enableInterrupts();
#endif

    // get the syscallmanager now (since it could have been replaced..) and call syscall handling method
    handleSyscall( (int4) sp_int);
}

extern "C"void handleTimerIRQ(void* sp_int, void* myKernelStackBucketIndex)
{
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"Timer Interrupt!"));
    // print the content of the registers!
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"r0 : 0x%x, r1: 0x%x, r2: 0x%x, r3:0x%x" , STACK_CONTENT(sp_int,-128),STACK_CONTENT(sp_int,-124),STACK_CONTENT(sp_int,-120),STACK_CONTENT(sp_int,-116)));
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"r4 : 0x%x, r5: 0x%x, r6: 0x%x, r7:0x%x" , STACK_CONTENT(sp_int,-112),STACK_CONTENT(sp_int,-108),STACK_CONTENT(sp_int,-104),STACK_CONTENT(sp_int,-100)));
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"r8 : 0x%x, r9: 0x%x, r2: 0x%x, r3:0x%x" , STACK_CONTENT(sp_int,-96),STACK_CONTENT(sp_int,-92),STACK_CONTENT(sp_int,-88),STACK_CONTENT(sp_int,-84)));
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"r12 : 0x%x, r13: 0x%x, r14: 0x%x, r15:0x%x" , STACK_CONTENT(sp_int,-80),STACK_CONTENT(sp_int,-76),STACK_CONTENT(sp_int,-72),STACK_CONTENT(sp_int,-68)));
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"r16 : 0x%x, r17: 0x%x, r18: 0x%x, r19:0x%x" , STACK_CONTENT(sp_int,-64),STACK_CONTENT(sp_int,-60),STACK_CONTENT(sp_int,-56),STACK_CONTENT(sp_int,-52)));
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"r20 : 0x%x, r21: 0x%x, r22: 0x%x, r23:0x%x" , STACK_CONTENT(sp_int,-48),STACK_CONTENT(sp_int,-44),STACK_CONTENT(sp_int,-40),STACK_CONTENT(sp_int,-36)));
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"r24 : 0x%x, r25: 0x%x, r26: 0x%x, r27:0x%x" , STACK_CONTENT(sp_int,-32),STACK_CONTENT(sp_int,-28),STACK_CONTENT(sp_int,-24),STACK_CONTENT(sp_int,-20)));
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"r28 : 0x%x, r29: 0x%x, r30: 0x%x, r31:0x%x" , STACK_CONTENT(sp_int,-16),STACK_CONTENT(sp_int,-12),STACK_CONTENT(sp_int,-8),STACK_CONTENT(sp_int,-4)));
    LOG(PROCESS,TRACE,(PROCESS,TRACE,"LR : 0x%x, CR: 0x%x, SRR1: 0x%x, SRR0:0x%x" , STACK_CONTENT(sp_int,-136),STACK_CONTENT(sp_int,-140),STACK_CONTENT(sp_int,-144),STACK_CONTENT(sp_int,-148)));


    // the idle thread could be interrupted by the timer. check this!
    if (pCurrentRunningThread != 0) {

        ASSERT(isOk(pCurrentRunningThread->pushStackPointer(sp_int)));

        #if USE_SAFE_KERNEL_STACKS
            // store the used stack bucket index
            pCurrentRunningThread->setKernelStackBucketIndex((int)myKernelStackBucketIndex);
        #endif

    } else
    {
       #if USE_SAFE_KERNEL_STACKS
           FREE_KERNEL_STACK_SLOT(myKernelStackBucketIndex);
       #endif
    }



#ifdef HAS_Board_WatchdogCfd
    theOS->getBoard()->getWatchdog()->kick();
#endif

    ASSERT(theTimer);
    // call Timer
    theTimer->tick();
}

extern "C"void handleExternalIRQ(void* sp_int, void* myKernelStackBucketIndex)
{
   // LOG(ARCH,ERROR,(ARCH,ERROR,"External Interrupt!"));

    if (pCurrentRunningThread != 0) {
        // Store the Stack pointer addresses into the thread TCB since we got interrupted
        ASSERT(isOk(pCurrentRunningThread->pushStackPointer(sp_int)));

        #if USE_SAFE_KERNEL_STACKS
            // store the used stack bucket index
            pCurrentRunningThread->setKernelStackBucketIndex((int)myKernelStackBucketIndex);
        #endif

    } else
    {
       #if USE_SAFE_KERNEL_STACKS
           FREE_KERNEL_STACK_SLOT(myKernelStackBucketIndex);
       #endif
    }

    // read the interrupt status vector from the interrupt controller
    // this implementation always needs an interrupt controller to be present
    int regval = theOS->getBoard()->getInterruptController()->getIRQStatusVector();

    if (  regval & OPB_UART_LITE_IRQ ) {
        // UART threw an interrupt
        // disable the interrupts from the comm device since it will be still pending after we exit this
        // handler routine.
    	theOS->getInterruptManager()->handleIRQ(OPB_UART_LITE_IRQ);
    }
    if (  regval & PLB_EMAC0_IRQ ) {
        // ETH threw an interrupt
        // disable the interrupts from the comm device since it will be still pending after we exit this
        // handler routine.
    	theOS->getInterruptManager()->handleIRQ(PLB_EMAC0_IRQ);
    }
    if (  regval & PUSH_BUTTON_IRQ ) {
    	theOS->getInterruptManager()->handleIRQ(PUSH_BUTTON_IRQ);
    }


    // the workerthread may have a higher priority so we need to reschedule here!
    theOS->getCPUDispatcher()->dispatch( );

}


extern "C" void handleCriticalError(void* sp_int, unint4 irq, unint4 srr2, unint4 srr3)
{
    LOG(ARCH,FATAL,(ARCH,FATAL,"Critical Exception: 0x%x, SRR2: 0x%x, SRR3: 0x%x",irq,srr2,srr3));
	theOS->getErrorHandler()->handleError(cDataAbortError);
}

extern "C" void handleTLBError(void* sp_int)
{
	theOS->getErrorHandler()->handleError(cMappingError);
}

extern "C"void handleWatchdogIRQ(void* sp_int)
{
	theOS->getErrorHandler()->handleError(cWatchdog);
}

