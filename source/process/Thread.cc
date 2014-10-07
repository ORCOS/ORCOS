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
#include "inc/error.hh"
#include "filesystem/KernelVariable.hh"

/* This is the hardware dependent method to start the thread the very first time
 * needs to be resolved by the linker */
extern void startThread(Thread* thread);

/* the kernel object */
extern Kernel* theOS;
extern Kernel_ThreadCfdCl* pCurrentRunningThread;
extern Task*   pCurrentRunningTask;

/* static non-const member variable initialization
 * will be executed in ctor */
ThreadIdT Thread::globalThreadIdCounter;

/* check if stack growing direction is set by scl */
#ifndef STACK_GROWS_DOWNWARDS
#error 'STACK_GROWS_DOWNWARDS' not defined!
#endif

/*! depending on the ABI, some bytes has to be reserved for the stack frame
 * (important for the task stack, otherwise the task heap could be corrupted by calling the startThread method) */
#ifndef RESERVED_BYTES_FOR_STACKFRAME
#error 'RESERVED_BYTES_FOR_STACKFRAME' not defined!
#endif

/* check whether the default stack size is specified by scl */
#ifndef DEFAULT_USER_STACK_SIZE
#error 'DEFAULT_USER_STACK_SIZE' not defined!
#endif

/*--------------------------------------------------------------------------*
 ** Thread::Thread
 *---------------------------------------------------------------------------*/
Thread::Thread(void* p_startRoutinePointer, void* p_exitRoutinePointer, Task* p_owner, Kernel_MemoryManagerCfdCl* memManager, unint4 stack_size, void* attr, bool newThread) {

    ASSERT(p_owner);
    ASSERT(memManager);
    /* do not initialize sleeptime here as it is done inside the realtimethread */
    this->myThreadId            = globalThreadIdCounter++;
    this->startRoutinePointer   = p_startRoutinePointer;
    this->exitRoutinePointer    = p_exitRoutinePointer;
    this->owner                 = p_owner;
    this->arguments             = 0;
    this->signal                = 0;
    this->signalvalue           = -1;
    this->pBlockedMutex         = 0;
    this->blockTimeout          = 0;
    this->status.clear();
    this->status.setBits( cNewFlag );
    this->name[0] = 0;

    /* inform owner task that this thread belongs to him */
    if (owner != 0)
        owner->addThread(static_cast< Kernel_ThreadCfdCl*>(this));

    if (owner != 0)
        TRACE_ADD_SOURCE(owner->getId(),this->myThreadId,0);
    else
        TRACE_ADD_SOURCE(0,this->myThreadId,0);

    /* create the thread stack inside the tasks heap!
       thus the memManager of the heap is used! */

    if (pCurrentRunningTask == 0 || pCurrentRunningTask == owner)
    {
        this->threadStack.startAddr = memManager->alloc(stack_size + RESERVED_BYTES_FOR_STACKFRAME, true);
    }
    else
    {
        /* we are creating this thread from the context of another task
         probably by new Task() from a system call. Thus, we can not access the memory of the owner task now
         as they both have the same LOG_START_ADDRESS.
         switching PID does not work as we are working on the thread stack of the calling task..
         thus, the TaskManager will set this address. */
        this->threadStack.startAddr = (void*) owner->getTaskTable()->task_heap_start;
    }

    this->threadStack.endAddr   = (void*) ((byte*) threadStack.startAddr + stack_size + RESERVED_BYTES_FOR_STACKFRAME);
    this->threadStack.top       = 0;

    Directory* taskdir = p_owner->getSysFsDirectory();
    if (taskdir) {
       char* idstr = new char[16];
       sprintf(idstr,"thread_%u",myThreadId);
       sysFsDir    = new Directory(idstr);
       taskdir->add(sysFsDir);

       EXPORT_VARIABLE(sysFsDir,SYSFS_UNSIGNED_INTEGER,blockTimeout,RO);
       EXPORT_VARIABLE(sysFsDir,SYSFS_STRING,name,RO);
       EXPORT_VARIABLE_BY_NAME(sysFsDir,"tid",SYSFS_UNSIGNED_INTEGER,myThreadId,RO);
       EXPORT_VARIABLE(sysFsDir,SYSFS_UNSIGNED_INTEGER,signal,RO);
       EXPORT_VARIABLE(sysFsDir,SYSFS_UNSIGNED_INTEGER,signalvalue,RO);
       EXPORT_VARIABLE(sysFsDir,SYSFS_UNSIGNED_INTEGER,sleepTime,RO);
       EXPORT_VARIABLE(sysFsDir,SYSFS_UNSIGNED_INTEGER,status,RO);
#ifdef HAS_PRIORITY
       EXPORT_VARIABLE_BY_NAME(sysFsDir,"effectivePriority",SYSFS_UNSIGNED_INTEGER,((PriorityThread*)this)->effectivePriority,RO);
       EXPORT_VARIABLE_BY_NAME(sysFsDir,"initialPriority",SYSFS_UNSIGNED_INTEGER,((PriorityThread*)this)->initialPriority,RO);
#endif
#ifdef REALTIME
       EXPORT_VARIABLE_BY_NAME(sysFsDir,"relativeDeadline",SYSFS_UNSIGNED_INTEGER,((RealTimeThread*)this)->relativeDeadline,RO);
       EXPORT_VARIABLE_BY_NAME(sysFsDir,"absoluteDeadline",SYSFS_UNSIGNED_INTEGER,((RealTimeThread*)this)->absoluteDeadline,RO);
       EXPORT_VARIABLE_BY_NAME(sysFsDir,"arrivalTime",SYSFS_UNSIGNED_INTEGER,((RealTimeThread*)this)->arrivalTime,RO);
       EXPORT_VARIABLE_BY_NAME(sysFsDir,"executionTime",SYSFS_UNSIGNED_INTEGER,((RealTimeThread*)this)->executionTime,RO);
       EXPORT_VARIABLE_BY_NAME(sysFsDir,"period",SYSFS_UNSIGNED_INTEGER,((RealTimeThread*)this)->period,RO);
       EXPORT_VARIABLE_BY_NAME(sysFsDir,"instance",SYSFS_UNSIGNED_INTEGER,((RealTimeThread*)this)->instance,RO);
#endif
    }

}

Thread::~Thread() {
/* keep empty if possible */
}

/*--------------------------------------------------------------------------*
 ** Thread::run
 *---------------------------------------------------------------------------*/
ErrorT Thread::run() {
    /* set my state to ready */
    this->status.setBits( cReadyFlag );

    /* inform the global cpu scheduler that i am ready to run */
    return (theOS->getCPUScheduler()->enter(this));
}

/*--------------------------------------------------------------------------*
 ** Thread::callMain
 *---------------------------------------------------------------------------*/
void Thread::callMain() {
    /* clear the new flag since we are not new anymore */
    this->status.clearBits( cNewFlag );

    startThread(this);
}

/*--------------------------------------------------------------------------*
 ** Thread::sleep
 *---------------------------------------------------------------------------*/
void Thread::sleep(TimeT timePoint, LinkedListItem* item) {

    LOG(PROCESS, DEBUG, "Thread::sleep: TimePoint: %x %x",
        (unint4) ((timePoint >> 32) & 0xffffffff),
        (unint4) ((timePoint) & 0xffffffff));

    TRACE_THREAD_STOP(this->getOwner()->getId(),this->getId());

    this->sleepTime = timePoint;

    /* remove the ready flag since we are not ready to run */
    this->status.clearBits( cReadyFlag );

    /* inform the current cpu scheduler that we are going to sleep
     the scheduler then puts the thread into some sleep queue */
    theOS->getDispatcher()->sleep(item);

}

/*--------------------------------------------------------------------------*
 ** Thread::sigwait
 *---------------------------------------------------------------------------*/
#ifdef ORCOS_SUPPORT_SIGNALS
void Thread::sigwait(void* sig) {
    signal = sig;

    status.clearBits( cReadyFlag );
    status.setBits( cSignalFlag );

    theOS->getDispatcher()->sigwait(this);
}
#endif

/*--------------------------------------------------------------------------*
 ** Thread::getMemManager
 *---------------------------------------------------------------------------*/
Kernel_MemoryManagerCfdCl* Thread::getMemManager() {
    return (owner->getMemManager());
}

/*--------------------------------------------------------------------------*
 ** Thread::block
 *---------------------------------------------------------------------------*/
void Thread::block(unint4 timeout) {
    /* remove the ready flag since we are not ready to run any more */
    this->status.clearBits( cReadyFlag );
    this->status.setBits( cBlockedFlag );
    this->blockTimeout = timeout;

    if (pCurrentRunningThread != this)
    {
        /* no need to save context since its not the currently running thread thats going to be blocked */
        theOS->getDispatcher()->block(this);
    }
    else
    {
        /* since we are going to be blocked save the thread context! */
        unsigned char buf[ PROCESSOR_CONTEXT_SIZE + 16];
        void* stackpointer = &buf;

        LOG(PROCESS, DEBUG, "Thread::block() id=%d stackpointer=%x ",this->getId(),stackpointer);

        /* Save Register and msr like the context save method in an interrupt handler would do */
        SAVE_CONTEXT_AT(&buf);
        ASSERT(isOk(this->pushStackPointer( stackpointer )));

#ifdef PLATFORM_ARM
        /* context restore mode will be svc mode */
        ASSERT(isOk(this->pushStackPointer( (void*) 0xd3 )));
#endif

        /* inform the current cpu scheduler (non returning call).
           the scheduler then puts the thread into some blocked queue */
        theOS->getDispatcher()->block(this);

        /* restore point must be referenced in SAVE_CONTEXT_AT */
        asm volatile("returnof:");

        /* don not put anything here! since optimized code may use variables inside register
         that have been initialized only after the context was saved at SAVE_CONTEXT! */
    }

}

/*--------------------------------------------------------------------------*
 ** Thread::unblock
 *---------------------------------------------------------------------------*/
void Thread::unblock() {
    /* unset the blocked flag since we are ready to run */
    this->status.clearBits( cBlockedFlag);
    if (!this->status.areSet( cStopped))
    {

        LOG(PROCESS, DEBUG, "Thread::unblock() id=%d",this->getId());

        /* we are allowed to run again since the task is not stopped */
        this->status.setBits( cReadyFlag);
        theOS->getDispatcher()->unblock(this);
    }
    else
    {
        /* our parent task is stopped so we cant just go on running */
    }
}

void Thread::resume() {
    this->status.clearBits( cStopped );
    if (!this->status.areSet( cBlockedFlag ))
    {
        /* we are allowed to unblock this thread */
        this->status.setBits( cReadyFlag);
        theOS->getDispatcher()->unblock(this);
    }
    else
    {
        if (pBlockedMutex != 0)
            pBlockedMutex->threadResume((Kernel_ThreadCfdCl*) this);
    }
}

void Thread::stop() {
    this->status.setBits( cStopped );
    /* if the thread is currently not blocked block it now */
    if (!this->status.areSet( cBlockedFlag ))
        theOS->getDispatcher()->block(this);
}

/*--------------------------------------------------------------------------*
 ** Thread::terminate
 *---------------------------------------------------------------------------*/
void Thread::terminate() {

    LOG(PROCESS, INFO, "thread %d terminated", this->myThreadId );

    if (pBlockedMutex != 0) {
        pBlockedMutex->threadRemove(this);
        pBlockedMutex = 0;
    }

    this->status.clear();
    this->status.setBits( cTermFlag );

#ifdef ORCOS_SUPPORT_SIGNALS
    /* be sure only threads inside our tasks are signaled as signaling is global */
    unint4 signalnum = (this->owner->getId() << 16) | (SIG_CHILD_TERMINATED);
    theOS->getDispatcher()->signal((void*) signalnum, cOk);
#endif

    /* free thread stack. we are using it here thus the code
     * path following is critical!
     * however free only if this stack has been allocated from the memory manager.
     * The only exception is the initial thread that is created by another process!
     * */
    if (pCurrentRunningTask == this->owner && this->owner->getMemManager()->containsAddr(this->threadStack.startAddr) )
        this->owner->getMemManager()->free(this->threadStack.startAddr);

    /* cleanup exported kernel variable stuff */
    if (sysFsDir && owner) {
       Directory* taskdir = owner->getSysFsDirectory();
       if (taskdir) {
           taskdir->remove(sysFsDir);
           /* also deletes all exported variables inside the sysfs directory */
           delete sysFsDir;
       }
    }

}
