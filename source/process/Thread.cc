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

#include <filesystem/SysFs.hh>
#include "process/Thread.hh"
#include "kernel/Kernel.hh"
#include "assemblerFunctions.hh"
#include "inc/error.hh"
#include "inc/signals.hh"

/* This is the hardware dependent method to start the thread the very first time
 * needs to be resolved by the linker */
/*****************************************************************************
 * Method: startThread(Thread* thread);
 *
 * @description
 *  This is the hardware dependent method to start the thread the very first time
 *  needs to be resolved by the linker
 *******************************************************************************/
extern void startThread(Thread* thread);

/* the kernel object */
extern Kernel* theOS;
extern Kernel_ThreadCfdCl* pCurrentRunningThread;
extern Task* pCurrentRunningTask;

/* static non-const member variable initialization
 * will be executed in ctor */
IDMap* Thread::freeIDMap;

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
Thread::Thread(void* p_startRoutinePointer,
               void* p_exitRoutinePointer,
               Task* p_owner,
               unint4 stack_size,
               void* attr) :
               myListItem(this) {
    ASSERT(p_owner);

    /* do not initialize sleeptime here as it is done inside the realtimethread */
    this->myThreadId            = freeIDMap->getNextID();
    this->startRoutinePointer   = p_startRoutinePointer;
    this->exitRoutinePointer    = p_exitRoutinePointer;
    this->owner                 = p_owner;
    this->arguments             = 0;
    this->signal                = 0;
    this->signalvalue           = -1;
    this->blockTimeout          = 0;
    this->name[0]               = 0;
    this->sysFsDir              = 0;
    this->status.clear();
    this->status.setBits(cNewFlag);
    myListItem.setData(this);

    /* inform owner task that this thread belongs to him */
    if (owner != 0)
        owner->addThread(static_cast< Kernel_ThreadCfdCl*>(this));

    if (owner != 0)
        TRACE_ADD_SOURCE(owner->getId(), this->myThreadId, 0);
    else
        TRACE_ADD_SOURCE(0, this->myThreadId, 0);

    /* allocate and map thread stack */
    this->threadStack.startAddr = (void*) theOS->getRamManager()->alloc_logical(stack_size, owner->getId(), 0);
    this->threadStack.endAddr   = reinterpret_cast<void*> ((unint4) threadStack.startAddr + stack_size - RESERVED_BYTES_FOR_STACKFRAME);
    this->threadStack.top       = 0;

    Directory* taskdir = p_owner->getSysFsDirectory();
    if (taskdir) {
        char* idstr = new char[16];
        sprintf(idstr, "thread_%u", myThreadId);
        sysFsDir = new Directory(idstr);
        taskdir->add(sysFsDir);

        SYSFS_ADD_RO_STRING(sysFsDir, name);
        SYSFS_ADD_RO_UINT_NAMED(sysFsDir, "tid", myThreadId);
        SYSFS_ADD_RO_UINT(sysFsDir, blockTimeout);
        SYSFS_ADD_RO_UINT(sysFsDir, signal);
        SYSFS_ADD_RO_UINT(sysFsDir, signalvalue);
        SYSFS_ADD_RO_UINT(sysFsDir, sleepTime);
        SYSFS_ADD_RO_UINT(sysFsDir, status);

#ifdef HAS_PRIORITY
        PriorityThread* prioThread = static_cast<PriorityThread*>(this);
        SYSFS_ADD_RO_UINT_NAMED(sysFsDir, "effectivePriority", prioThread->effectivePriority);
        SYSFS_ADD_RO_UINT_NAMED(sysFsDir, "initialPriority"  , prioThread->initialPriority);
#endif
#ifdef REALTIME
        RealTimeThread* rtThread = static_cast<RealTimeThread*>(this);
        SYSFS_ADD_RO_UINT_NAMED(sysFsDir, "relativeDeadline", rtThread->relativeDeadline);
        SYSFS_ADD_RO_UINT_NAMED(sysFsDir, "absoluteDeadline", rtThread->absoluteDeadline);
        SYSFS_ADD_RO_UINT_NAMED(sysFsDir, "arrivalTime"     , rtThread->arrivalTime);
        SYSFS_ADD_RO_UINT_NAMED(sysFsDir, "executionTime"   , rtThread->executionTime);
        SYSFS_ADD_RO_UINT_NAMED(sysFsDir, "period"          , rtThread->period);
        SYSFS_ADD_RO_UINT_NAMED(sysFsDir, "instance"        , rtThread->instance);
#endif
    }
}

Thread::~Thread() {
    /* keep empty if possible */
}

/*****************************************************************************
 * Method: Thread::run()
 *
 * @description
 *  Announces the thread to the scheduler, thus making it ready to run.
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT Thread::run() {
    /* set my state to ready */
    this->status.setBits(cReadyFlag);
    LOG(PROCESS, DEBUG, "Thread::run Id: %d Name: %s", this->getId(), this->getName() );

    /* inform the global cpu scheduler that i am ready to run */
    return (theOS->getCPUScheduler()->enter(getLinkedListItem()));
}

/*****************************************************************************
 * Method: Thread::callMain()
 *
 * @description
 *  Calls the entry method of the thread. Does not return.
 *  Causes a virtual address space switch.
 *
 *******************************************************************************/
void Thread::callMain() {
    /* clear the new flag since we are not new anymore */
    this->status.clearBits(cNewFlag);
    startThread(this);
}

/*****************************************************************************
 * Method: Thread::sleep(TimeT timePoint, LinkedListItem* item)
 *
 * @description
 *  Sends the thread to sleep until the given absolute timepoint
 *  is reached.
 *
 *******************************************************************************/
void Thread::sleep(TimeT timePoint) {
    LOG(PROCESS, DEBUG, "Thread::sleep: TimePoint: %x %x", (unint4) ((timePoint >> 32) & 0xffffffff), (unint4) ((timePoint) & 0xffffffff));

    TRACE_THREAD_STOP(this->getOwner()->getId(), this->getId());

    this->sleepTime = timePoint;

    /* remove the ready flag since we are not ready to run */
    this->status.clearBits(cReadyFlag);

    /* inform the current cpu scheduler that we are going to sleep
     the scheduler then puts the thread into some sleep queue */
    theOS->getDispatcher()->sleep(this);
}

#ifdef ORCOS_SUPPORT_SIGNALS
/*****************************************************************************
 * Method: Thread::sigwait(void* sig)
 *
 * @description
 *  Sets the signal the thread will start waiting for from now on.
 *  Stops execution of the thread until the signal is raised.
 *
 * @params
 *  sig:     The signal to wait for
 *******************************************************************************/
void Thread::sigwait(SignalType signaltype, void* sig) {
    signal = sig;

    status.clearBits(cReadyFlag);
    status.setBits(cSignalFlag);

    theOS->getDispatcher()->sigwait(signaltype, this);
}
#endif

/*****************************************************************************
 * Method: Thread::block(unint4 timeout)
 *
 * @description
 *  Blocks the thread. If timeout is != 0 the thread will
 *  be automatically unblocked if the timeout is reached.
 *
 * @params
 *  timeout     Timeout of the blocked state in clock cycles.
 *
 *******************************************************************************/
void Thread::block(unint4 timeout) {
    /* remove the ready flag since we are not ready to run any more */
    this->status.clearBits(cReadyFlag);
    this->status.setBits(cBlockedFlag);
    this->blockTimeout = timeout;

    if (pCurrentRunningThread != this) {
        /* no need to save context since its not the currently running thread thats going to be blocked */
        theOS->getDispatcher()->block(this);
    } else {
        /* since we are going to be blocked save the thread context! */
        unsigned char buf[PROCESSOR_CONTEXT_SIZE + 16];
        void* stackpointer = &buf;

        LOG(PROCESS, DEBUG, "Thread::block() id=%d stackpointer=%x ", this->getId(), stackpointer);

        int returnMode = 0;
        GET_CPSR(returnMode);

        _disableInterrupts();
        /* Save Register and msr like the context save method in an interrupt handler would do */
        SAVE_CONTEXT_AT(&buf, returnMode);
        ASSERT(isOk(this->pushStackPointer(stackpointer)));

#ifdef PLATFORM_ARM
        /* context restore mode will be svc mode. */
        ASSERT(isOk(this->pushStackPointer(reinterpret_cast<void*>(0xd3))));
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

/*****************************************************************************
 * Method: Thread::unblock()
 *
 * @description
 *  Directly unblocks the thread.
 *******************************************************************************/
void Thread::unblock() {
    if (!isBlocked())
        return;

    /* unset the blocked flag since we are ready to run */
    this->status.clearBits(cBlockedFlag);
    if (!this->status.areSet(cStopped)) {
        LOG(PROCESS, DEBUG, "Thread::unblock() id=%d", this->getId());
        /* we are allowed to run again since the task is not stopped */
        this->status.setBits(cReadyFlag);
        theOS->getDispatcher()->unblock(this);
    } else {
        /* our parent task is stopped so we cant just go on running */
    }
}

/*****************************************************************************
 * Method: Thread::resume()
 *
 * @description
 *  Resumes the thread.
 *******************************************************************************/
void Thread::resume() {
    this->status.clearBits(cStopped);
    if (!this->status.areSet(cBlockedFlag)) {
        /* we are allowed to unblock this thread */
        this->status.setBits(cReadyFlag);
        theOS->getDispatcher()->unblock(this);
    } else {
       /* if (pBlockedMutex != 0)
            pBlockedMutex->threadResume(static_cast<Kernel_ThreadCfdCl*>(this));*/
    }
}

/*****************************************************************************
 * Method: Thread::stop()
 *
 * @description
 *  Stops the thread.
 *******************************************************************************/
void Thread::stop() {
    this->status.setBits(cStopped);
    /* if the thread is currently not blocked block it now */
    if (!this->status.areSet(cBlockedFlag))
        theOS->getDispatcher()->block(this);
}

/*****************************************************************************
 * Method: Thread::terminate()
 *
 * @description
 *  Terminates the thread
 *******************************************************************************/
void Thread::terminate() {
    LOG(PROCESS, INFO, "thread %d terminated", this->myThreadId);

    this->status.clear();
    this->status.setBits(cTermFlag);

#ifdef ORCOS_SUPPORT_SIGNALS
    /* be sure only threads inside our tasks are signaled as signaling is global */
    unint4 signalnum = SIGNAL_SPACE_TASK(this->owner->getId()) | SIG_CHILD_TERMINATED;
    theOS->getDispatcher()->signal(reinterpret_cast<void*>(signalnum), cOk);
#endif

    // TODO: free mapped thread stack pages


    /* remove myself from any scheduling queue */
    this->getLinkedListItem()->remove();

    /* cleanup exported kernel variable stuff */
    if (sysFsDir && owner) {
        Directory* taskdir = owner->getSysFsDirectory();
        if (taskdir) {
            taskdir->remove(sysFsDir);
            /* also deletes all exported variables inside the sysfs directory */
            delete sysFsDir;
            sysFsDir = 0;
        }
    }

    freeIDMap->freeID(this->myThreadId);
}
