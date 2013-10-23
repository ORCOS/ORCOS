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

#include "process/Thread.hh"
#include "kernel/Kernel.hh"
#include <assemblerFunctions.hh>

// This is the hardware dependend method to start the thread the very first time
// needs to be resolved by the linker
extern void startThread( Thread* thread );

// the kernel object
extern Kernel* theOS;
extern Thread* pCurrentRunningThread;
extern Board_ClockCfdCl* theClock;
extern unint8  lastCycleStamp;
extern Task* pCurrentRunningTask;


// static non-const member variable initialization
// will be executed in ctor
ThreadIdT Thread::globalThreadIdCounter;

// check if stack growing direction is set by scl
#ifndef STACK_GROWS_DOWNWARDS
#error 'STACK_GROWS_DOWNWARDS' not defined!
#endif

//! depending on the abi, some bytes has to be reserved for the stack frame
// (important for the task stack, otherwise the task heap could be corrupted by calling the startThread method)
#ifndef RESERVED_BYTES_FOR_STACKFRAME
#error 'RESERVED_BYTES_FOR_STACKFRAME' not defined!
#endif

// check whether the default stack size is specified by scl
#ifndef DEFAULT_USER_STACK_SIZE
#error 'DEFAULT_USER_STACK_SIZE' not defined!
#endif

/*--------------------------------------------------------------------------*
 ** Thread::Thread
 *---------------------------------------------------------------------------*/
Thread::Thread( void* startRoutinePointer, void* exitRoutinePointer, Task* owner, Kernel_MemoryManagerCfdCl* memManager,
        unint4 stack_size, void* attr, bool newThread ) {

    ASSERT(owner);
    ASSERT(memManager);

    this->myThreadId 			= globalThreadIdCounter++;
    this->startRoutinePointer 	= startRoutinePointer;
    this->exitRoutinePointer 	= exitRoutinePointer;
    this->owner 				= owner;
    this->status.clear();
    this->status.setBits( cNewFlag );
    this->arguments 			= 0;
    this->sleepCycles 			= 0;
    this->signal 				= 0;

    // inform my owner that i belong to him
    if ( owner != 0 )
        owner->addThread( static_cast< Kernel_ThreadCfdCl* > ( this ) );

    // create the thread stack inside the tasks heap!
    // thus the memManager of the heap is used!

    if (pCurrentRunningTask == 0 || pCurrentRunningTask == owner) {
    	this->threadStack.startAddr = memManager->alloc( stack_size + RESERVED_BYTES_FOR_STACKFRAME, true );
    } else {
    	// we are creating this thread from the context of another task
    	// probably new Task(). Thus, we can not access the memory of the owner task now
    	// as they both have the same LOG_START_ADDRESS.
    	// switching PID does not work as we are working on the thread stack of the calling task.. :(
    	// we will create this thread stack during startThread().
    	this->threadStack.startAddr = (void*) owner->getTaskTable()->task_heap_start;
    }

    this->threadStack.endAddr = (void*) ( (byte*) threadStack.startAddr + stack_size );
    this->threadStack.top = 0;


#ifdef CLEAR_THREAD_STACKS
    //TODO: only clear if there is no virtual memory! since otherwise we cant access the memory here right now!
    //int* addr = (int*) threadStack.startAddr;
    //int* endaddr = (int*) threadStack.endAddr;
    // be sure that endaddr - startaddr is a multiple of the word size of the used architecture
    // if not we might clear to many bytes (although this is not a problem here since no memory has been allocated)
    // right after the stack till now

    // now clear the memory area
    /*while ( addr < endaddr ) {
        *addr = 0;
        addr++;
    }*/
#endif

}

/*--------------------------------------------------------------------------*
 ** Thread::run
 *---------------------------------------------------------------------------*/
ErrorT Thread::run() {
    // set my state to ready
    this->status.setBits( cReadyFlag );

    // inform the global cpu scheduler that i am ready to run
    return (theOS->getCPUScheduler()->enter( this ));
}

/*--------------------------------------------------------------------------*
 ** Thread::callMain
 *---------------------------------------------------------------------------*/
void Thread::callMain() {
    // clear the new flag since we are not new anymore
    this->status.clearBits( cNewFlag );

    startThread( this );
}

/*--------------------------------------------------------------------------*
 ** Thread::sleep
 *---------------------------------------------------------------------------*/
void Thread::sleep( int t, LinkedListDatabaseItem* item ) {

	// prepare for sleep list update on dispatch call following
	unint8 passedtime = theClock->getTimeSinceStartup() - lastCycleStamp;
	this->sleepCycles = t + (unint4) passedtime;

    LOG(SCHEDULER,INFO,(SCHEDULER,INFO,"Thread::sleep() t=%d",this->sleepCycles));
    // unset the ready flag since we are not ready to run
    this->status.clearBits( cReadyFlag );

    // inform the current cpuscheduler that im going to sleep
    // the scheduler then puts the sleep into some sleep queue
    theOS->getCPUDispatcher()->sleep( this->sleepCycles, item );

}

/*--------------------------------------------------------------------------*
 ** Thread::sigwait
 *---------------------------------------------------------------------------*/
void Thread::sigwait( void* sig ) {
    signal = sig;

    status.clearBits( cReadyFlag );
    status.setBits( cSignalFlag );

    theOS->getCPUDispatcher()->sigwait( this );
}

Kernel_MemoryManagerCfdCl* Thread::getMemManager() {
    return (owner->getMemManager());
}


/*--------------------------------------------------------------------------*
 ** Thread::block
 *---------------------------------------------------------------------------*/
void Thread::block() {
    // unset the ready flag since we are not ready to run
    this->status.clearBits( cReadyFlag );
    this->status.setBits( cBlockedFlag );

    if (pCurrentRunningThread != this)
    {
        // no need to save context since its not the currently running thread thats going to be blocked
        theOS->getCPUDispatcher()->block( this );
    }
    else
    {

    	/// TODO: implement the Thread->block method in assembler.. which takes care of
    	// saveing the context correctly.. then call the c++ method which calls the CPUDispatcher
    	// thus: thread->asm_block->block

    	// since we are going to be blocked save my context!
        unsigned char buf[ PROCESSOR_CONTEXT_SIZE + 16 ];
        void* stackpointer = &buf;

        #if STACK_GROWS_DOWNWARDS
        	// TODO add macro with context size
            //void* stackpointer = &buf[ 156 ];  // ppc

			//void* stackpointer = &buf[ 0 ];
        #else
            void* stackpointer = &buf[ 400 ];
        #endif

        LOG(PROCESS,DEBUG,(PROCESS,DEBUG,"Thread::block() id=%d stackpointer=%x ",this->getId(),stackpointer));

        // Save Register and msr like the context save method in an interrupt handler would do
        SAVE_CONTEXT_AT(&buf);

        ASSERT(isOk(this->pushStackPointer( stackpointer )));

        #ifdef PLATFORM_ARM
        // context restore mode will be svc mode
        ASSERT(isOk(this->pushStackPointer( (void*) 0xd3 )));
		#endif

        // inform the current cpuscheduler that im going to sleep
        // the scheduler then puts the thread into some sleep queue
        theOS->getCPUDispatcher()->block( this );

        // restore point must be referenced in SAVE_CONTEXT_AT
        asm volatile("returnof:");

        // dont put anything behind here! since optimized code may use variables inside register
        // that have been initialized only after the context was saved at SAVE_CONTEXT!
        //LOG(PROCESS,DEBUG,(PROCESS,DEBUG,"Thread::block() returned!"));
    }

}

/*--------------------------------------------------------------------------*
 ** Thread::unblock
 *---------------------------------------------------------------------------*/
void Thread::unblock() {
    // unset the blocked flag since we are ready to run
    this->status.clearBits( cBlockedFlag );
    if ( !this->status.areSet( cStopped ) ) {

    	  LOG(PROCESS,DEBUG,(PROCESS,DEBUG,"Thread::unblock() id=%d",this->getId()));

        // we are allowed to run again since the task is not stopped
        this->status.setBits( cReadyFlag );
        theOS->getCPUDispatcher()->unblock( this );
    }
    else {
        // our parent task is stopped so we cant just go on running
    }
}

void Thread::resume() {
    this->status.clearBits( cStopped );
    if ( !this->status.areSet( cBlockedFlag ) ) {
        // we are allowed to unblock this thread
        this->status.setBits( cReadyFlag );
        theOS->getCPUDispatcher()->unblock( this );
    }
    else {
        if ( pBlockedMutex != 0 )
            pBlockedMutex->threadResume( (Kernel_ThreadCfdCl*) this );
    }
}

void Thread::stop() {
    this->status.setBits( cStopped );
    // if the thread is currently not blocked block it now
    if ( !this->status.areSet( cBlockedFlag ) )
        theOS->getCPUDispatcher()->block( this );
}

/*--------------------------------------------------------------------------*
 ** Thread::terminate
 *---------------------------------------------------------------------------*/
void Thread::terminate() {

    LOG(PROCESS,INFO,( PROCESS, INFO, "thread %d exited", this->myThreadId ));

    // remove myself from the owner
    this->owner->removeThread( this );
    this->status.clear();
    this->status.setBits( cTermFlag );

    // finally tell cpudispatcher that im gone..
    theOS->getCPUDispatcher()->terminate_thread( this );
}
