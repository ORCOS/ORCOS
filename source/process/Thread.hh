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

#ifndef _THREAD_HH
#define _THREAD_HH

#include "inc/const.hh"
#include "inc/types.hh"
#include "inc/Bitmap.hh"
#include "SCLConfig.hh"
#include "db/LinkedListDatabaseItem.hh"

// forward declaration of Task
class Task;
class Mutex;


#define MAXSTACKPTRS 6

#include "scheduler/ScheduleableItem.hh"
#include MemoryManagerCfd_hh

extern LinkedListDatabaseItem* pRunningThreadDbItem;

// Flag definitions
//! thread is constructed
#define cNewFlag		(BitmapT)1<<0
//! thread is ready to run
#define cReadyFlag		(BitmapT)1<<1
//! thread has stopped
#define cBlockedFlag	(BitmapT)1<<2
//! thread is terminated
#define cTermFlag		(BitmapT)1<<3
//! thread is waiting for a resource
#define cResource       (BitmapT)1<<4
//! thread execution stopped
#define cStopped		(BitmapT)1<<5
//! thread is waiting for a signal
#define cSignalFlag     (BitmapT)1<<6

/*!
 * \brief Structure holding information on the stack of a thread
 */
struct ThreadStack {
    void* startAddr;    //!< The logical start addr of this stack
    void* endAddr;      //!< The logical end addr of this stack

    void* stackptrs[MAXSTACKPTRS]; //!< stack pointer stack. contains pushed stack pointer addresses
    unsigned short  top;

    int2 myKernelStackBucketIndex;
};

/*!
 *  \brief Thread CB Class which holds information about a thread.
 *  \ingroup process
 *
 * Thread Control Block holding information about a thread.
 */
class Thread: public ScheduleableItem {

    //! The task class needs to access all private data of a thread since its the owner.
    friend class Task;
    friend class SingleCPUDispatcher;
    friend class Mutex;
    friend class Module;

public:

    //! The stack of the thread also used to save/restore the context.
    ThreadStack threadStack;

    //! The status of this thread. May cover states like running, blocked...
    Bitmap status;

    //! The pointer to the entry function.
    void* startRoutinePointer;

    //! Pointer to some arbitrary associated data
    void* pArg;

private:

    //! The Mutex we are currently blocked by
    Mutex* pBlockedMutex;

    //! The owner task of this thread.
    Task* owner;

    //! The pointer to the entry function.
    void* exitRoutinePointer;

    //!  A global Thread counter.
    static ThreadIdT globalThreadIdCounter;

    //! The Threads own Id.
    ThreadIdT myThreadId;

public:

#if ENABLE_NESTED_INTERRUPTS
    bool executinginthandler; //!< flag whether the thread is currently executing a interrupt handler
#endif

    //! The amount of cycles this thread will sleep from now on
    int4 sleepCycles;

    //! The arguments passed to the thread on startup
    void* arguments;

    //! A signal this thread waits for or null
    void* signal;

    //! Constructor which takes the startRoutinePointer as argument
    Thread( void* startRoutinePointer, void* exitRoutinePointer, Task* owner, MemoryManagerCfdCl* memManager,
            unint4 stack_size = DEFAULT_USER_STACK_SIZE, void* attr = 0, bool newThread = true );

    //! Destructor
    virtual ~Thread() {
    }
    ;

    //! initialize
    static void initialize() {
        globalThreadIdCounter = cFirstThread;
    }

    //! Returns the owner task of this thread.
    Task* getOwner() const {
        return owner;
    }

    //!  Get the flags of this Thread
    Bitmap& getStatus() {
        return status;
    }

    //!  Set the flags of this Thread
    ErrorT setStatus( Bitmap s ) {
        status = s;
        return cOk;
    }

    //! Returns true iff the thread has never been run before
    bool isNew() const {
        return status.areSet( cNewFlag );
    }

    //! Returns true iff the thread is scheduled for execution
    bool isReady() const {
        return status.areSet( cReadyFlag );
    }

    /*!
     * \brief Returns true iff the thread is currently blocked (usually waiting on a
     * 		 resource.
     */
    bool isBlocked() const {
        return status.areSet( cBlockedFlag );
    }

    /*!
     * \brief Returns true iff the thread is stopped.
     */
    bool isStopped() const {
        return status.areSet( cStopped );
    }

    bool isSleeping() const {
        return !status.anySet( (BitmapT) cBlockedFlag | cReadyFlag | cNewFlag );
    }

    //! Returns true iff the thread has terminated.
    bool hasTerminated() const {
        return status.areSet( cTermFlag );
    }

    //! Returns the currently set sleeptime
    int4 getSleepTime() {
        return this->sleepCycles;
    }

    ErrorT pushStackPointer(void* sp)
    {
        if (threadStack.top < MAXSTACKPTRS -1)
        {
            threadStack.stackptrs[threadStack.top] = sp;
            threadStack.top++;
            return cOk;
        } else return cStackOverflow;

    }

    ErrorT popStackPointer(void* &sp)
	{
		if (threadStack.top > 0)
		{
			threadStack.top--;
			sp = threadStack.stackptrs[threadStack.top];
			return cOk;
		} else return cStackUnderflow;
	}


#ifdef USE_SAFE_KERNEL_STACKS
    inline void setKernelStackBucketIndex(int2 index) {
        threadStack.myKernelStackBucketIndex = index;
    }

    inline int2 getKernelStackBucketIndex() {
        return threadStack.myKernelStackBucketIndex;
    }
#endif

    //! Returns the addr (pointer) of the startRoutine of this thread
    void* getStartRoutinePointer() const {
        return startRoutinePointer;
    }

    //! Returns the addr (pointer) of the exitRoutine of this thread
    void* getExitRoutinePointer() const {
        return exitRoutinePointer;
    }

    //! Returns the arguments passed to the thread on startup
    void* getStartArguments() const {
        return this->arguments;
    }

    //! Returns the Memory Manager of the task the thread belongs to
    MemoryManagerCfdCl* getMemManager();

    /*!
     * \brief This method sets up the thread for execution. This involves telling the scheduler
     * 		 that we are ready for running.
     */
    ErrorT run();

    /*!
     * \brief Sends the thread to sleep for 't' microseconds.
     *
     * Involves informing the scheduler.
     */
    void sleep( int t, LinkedListDatabaseItem* item = pRunningThreadDbItem );

    void sigwait( void* sig );

    /*!
     * \brief Blocks the current thread.
     *
     * Involves informing the scheduler.
     */
    void block();

    /*!
     * \brief Unblocks the thread that has been waiting on a resource.
     */
    void unblock();

    /*!
     * \brief Method needed to resume a thread if it was stopped by the task.
     */
    void resume();

    /*!
     * \brief stops the execution of this thread (controlled by task)
     */
    void stop();

    /*!
     * \brief Terminates the thread.
     *
     * Involves informing the scheduler. May cause a whole task to be terminated
     * if this is the last thread of a task.
     */
    void terminate();

    /*!
     * \brief Calling this method will setup the thread for first time execution and
     * 		 call the entry function of the thread.
     *
     * This method can be overwritten in order to allow other thread classes like e.g.
     * WorkerThread to implement their own behaviour.
     */
    virtual
    void callMain();

    //! Returns the Id of this Thread.
    inline
    ThreadIdT getId() const {
        return myThreadId;
    }

};

#endif /* _THREAD_HH */
