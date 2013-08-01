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

extern Kernel* theOS;
extern Board_TimerCfdCl* theTimer;


extern "C" void handleSyscallIRQ(void* sp_int, void* myKernelStackBucketIndex)
{

    // there cant be nested syscalls so just store at normal IRQStackPointerAddr
    pCurrentRunningThread->setIRQStackPointerAddr( (void*) sp_int );
#if USE_SAFE_KERNEL_STACKS
    // store the used stack bucket index
    pCurrentRunningThread->setKernelStackBucketIndex((int)myKernelStackBucketIndex);
#endif

#if ENABLE_NESTED_INTERRUPTS
    // set the flag so we know that we are executing the interrupt handler
    pCurrentRunningThread->executinginthandler = true;
    // we want nested interrupts so enable interrupts again
    _enableInterrupts();
#endif

    // get the syscallmanager now (since it could have been replaced..) and call syscall handling method
    handleSyscall( (int4) sp_int);
}

extern "C" void handleTimerIRQ(void* sp_int, void* myKernelStackBucketIndex)
{
#if ENABLE_NESTED_INTERRUPTS
    if ( !pCurrentRunningThread->executinginthandler ) {
        // Store the Stack pointer addresses into the thread TCB since we got interrupted
        pCurrentRunningThread->setIRQStackPointerAddr( (void*) sp_int );
#if USE_SAFE_KERNEL_STACKS
        // store the used stack bucket index
        pCurrentRunningThread->setKernelStackBucketIndex((int)myKernelStackBucketIndex);
#endif
    }
    else {
        // we have been interrupted while executing the interrupt handler
        // set the stack pointer of the nested interrupt handler
        pCurrentRunningThread->setNestedIRQStackPointerAddr( (void*) sp_int );
    }
#else
    // No nested interrupts are enabled so just store the addresses
    // Store the Stack pointer addresses into the thread CB since we got interrupted
    pCurrentRunningThread->setIRQStackPointerAddr((void*)sp_int);
#if USE_SAFE_KERNEL_STACKS
    // store the used stack bucket index
    pCurrentRunningThread->setKernelStackBucketIndex((int)myKernelStackBucketIndex);
#endif
#endif

#ifdef HAS_Board_WatchdogCfd
        theOS->getBoard()->getWatchdog()->kick();
#endif


    // call Timer
    theTimer->tick();
}


extern "C" void handleExternalIRQ(void* sp_int, void* myKernelStackBucketIndex)
{
#if ENABLE_NESTED_INTERRUPTS
    if ( !pCurrentRunningThread->executinginthandler ) {
        // Store the Stack pointer addresses into the thread TCB since we got interrupted
        pCurrentRunningThread->setIRQStackPointerAddr( (void*) sp_int );
#if USE_SAFE_KERNEL_STACKS
        // store the used stack bucket index
        pCurrentRunningThread->setKernelStackBucketIndex((int)myKernelStackBucketIndex);
#endif
    }
    else {
        // we have been interrupted while executing the interrupt handler
        // set the stack pointer of the nested interrupt handler
        pCurrentRunningThread->setNestedIRQStackPointerAddr( (void*) sp_int );
    }
#else
    // No nested interrupts are enabled so just store the addresses
    // Store the Stack pointer addresses into the thread CB since we got interrupted
    pCurrentRunningThread->setIRQStackPointerAddr((void*)sp_int);
#if USE_SAFE_KERNEL_STACKS
    // store the used stack bucket index
    pCurrentRunningThread->setKernelStackBucketIndex((int)myKernelStackBucketIndex);
#endif
#endif


        // UART threw an interrupt
        // disable the interrupts from the comm device since it will be still pending after we exit this
        // handler routine.
        register CommDeviceDriver* uart = theOS->getBoard()->getUART();
        uart->disableIRQ();
        uart->interruptPending = true;

#if USE_WORKERTASK
        if (!uart->hasAssignedWokerThread)
        {
            if (theOS->getWorkerTask()->addJob(ExternalDeviceJob,0,(void*) uart, WORKERTHREAD_UART_PRIORITY_PARAM))
            uart->hasAssignedWokerThread = true;
        }
#else
        uart->recv();
#endif


#ifdef HAS_PRIORITY
    // the workerthread may have a higher priority so we need to reschedule here!
    theOS->getCPUDispatcher()->dispatch( theOS->getClock()->getTimeSinceStartup() - lastCycleStamp );
#endif

// if we get here we need to restore the context
    assembler::restoreContext( pCurrentRunningThread );

}

extern "C" void handleWatchdogIRQ(void* sp_int)
{
#ifdef HAS_Kernel_LoggerCfd
    theOS->getLogger()->log(ARCH,FATAL,"Watchdog Exception! - CRITICAL");
#endif
    while (true) {}
}

