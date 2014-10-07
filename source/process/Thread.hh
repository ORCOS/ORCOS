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

///////////////////////////////////////////////////////////////////////
/// INCLUDES
//////////////////////////////////////////////////////////////////////

#include "inc/const.hh"
#include "inc/types.hh"
#include "inc/Bitmap.hh"
#include "SCLConfig.hh"
#include "db/LinkedListItem.hh"
#include "inc/stringtools.hh"
#include "inc/memtools.hh"
#include "scheduler/ScheduleableItem.hh"
#include Kernel_MemoryManager_hh

///////////////////////////////////////////////////////////////////////
/// FORWARD DECLARATIONS
//////////////////////////////////////////////////////////////////////
class Directory;
class Task;
class Mutex;

///////////////////////////////////////////////////////////////////////
/// TYPES / DEFINES
//////////////////////////////////////////////////////////////////////
#define MAXSTACKPTRS 6

extern LinkedListItem* pRunningThreadDbItem;

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
//! thread shall terminate after current instance (soft termination)
#define cDoTermFlag     (BitmapT)1<<7

/*!
 * \brief Structure holding information on the stack of a thread
 */
struct ThreadStack {
    void* startAddr;                //!< The logical start addr of this stack
    void* endAddr;                  //!< The logical end addr of this stack
    void* stackptrs[MAXSTACKPTRS];  //!< stack pointer stack. contains pushed stack pointer (context) addresses
    unsigned short top;
};

///////////////////////////////////////////////////////////////////////
/// CLASS DEFINITIONS
//////////////////////////////////////////////////////////////////////

/*!
 *  \brief Thread CB Class which holds information about a thread.
 *  \ingroup process
 *
 * Thread Control Block holding information about a thread.
 */
class Thread: public ScheduleableItem {

    /* The task class needs to access all private data of a thread since its the owner. */
    friend class Task;
    friend class Mutex;
    friend class Module;
    friend class TaskManager;

public:

    //! The stack of the thread also used to save/restore the context.
    ThreadStack         threadStack;

    //! The status of this thread. May cover states like running, blocked...
    Bitmap              status;

    //! The pointer to the entry function.
    void*               startRoutinePointer;

private:

    //! The Mutex we are currently blocked by
    Mutex*              pBlockedMutex;

    //! The owner task of this thread.
    Task*               owner;

    //! The pointer to the entry function.
    void*               exitRoutinePointer;

    //!  A global Thread counter.
    static ThreadIdT    globalThreadIdCounter;

    //! The sysFs directory of this thread
    Directory*          sysFsDir;

    //! The Threads own Id.
    ThreadIdT           myThreadId;

    /* the name of this thread */
    char                name[16];

public:

    //! The absolute time point in cycles this thread will sleeping up to
    TimeT               sleepTime;

    //! The arguments passed to the thread on startup
    void*               arguments;

    /*
     * The signal this thread waits for or null
     * If null (0) the thread is not waiting on a signal.
     * If != 0 the restoreContext method must take care
     * of passing 'signalvalue' to the thread.
     */
    void*               signal;

    /*
     * The signal value this thread received
     */
    unint4              signalvalue;

    /* The current timeout value of the blocked stated */
    unint4              blockTimeout;

    /* Constructor which takes the startRoutinePointer as argument */
    Thread(void* startRoutinePointer, void* exitRoutinePointer, Task* owner, Kernel_MemoryManagerCfdCl* memManager, unint4 stack_size =
                   DEFAULT_USER_STACK_SIZE, void* attr = 0, bool newThread =
                   true);

    /* Destructor */
    virtual ~Thread();


    /* Static initialization of the Thread class*/
    static void initialize() {
        globalThreadIdCounter = cFirstThread;
    }

    //! Returns the owner task of this thread.
    Task* getOwner() const {
        return (owner);
    }

    //!  Get the flags of this Thread
    Bitmap& getStatus() {
        return (status);
    }

    //!  Set the flags of this Thread
    ErrorT setStatus(Bitmap s) {
        status = s;
        return (cOk );
    }

    //! Sets the given status flags
    void setStatusFlag(int) {
        status.setBits(cDoTermFlag);
    }

    //! Returns true iff the thread has never been run before
    bool isNew() const {
        return (status.areSet( cNewFlag ));
    }

    //! Returns true iff the thread is scheduled for execution
    bool isReady() const {
        return (status.areSet( cReadyFlag ));
    }

    /*!
     * \brief Returns true iff the thread is currently blocked (usually waiting on a
     * 		 resource.
     */
    bool isBlocked() const {
        return (status.areSet( cBlockedFlag));
    }

    /*!
     * \brief Returns true iff the thread is stopped.
     */
    bool isStopped() const {
        return (status.areSet( cStopped));
    }

    bool isSleeping() const {
        return (!status.anySet((BitmapT) cBlockedFlag | cReadyFlag | cNewFlag));
    }

    //! Returns true iff the thread has terminated.
    bool hasTerminated() const {
        return (status.areSet( cTermFlag));
    }

    //! Returns the currently set sleeptime
    TimeT getSleepTime() const {
        return (this->sleepTime);
    }

    /*
     * Push the address sp into the threads personal stack.
     * This stack is to be used to store context related
     * memory locations of a thread.
     *
     * On interrupt or syscall the context of the thread has to be
     * stored and the address should be pushed on this stack.
     * Nested interrupts may force the use of multiple stack
     * locations.
     */
    ErrorT __attribute__((noinline)) pushStackPointer(void* sp) {
        if (threadStack.top < MAXSTACKPTRS - 1)
        {
            threadStack.stackptrs[threadStack.top] = sp;
            threadStack.top++;
            return (cOk );
        }
        else
        {

            return (cStackOverflow );
        }

    }

    /*
     * Pop a value from the threads personal stack.
     */
    ErrorT __attribute__((noinline)) popStackPointer(void* &sp) {
        if (threadStack.top > 0)
        {
            threadStack.top--;
            sp = threadStack.stackptrs[threadStack.top];
            return (cOk );
        }
        else
            return (cStackUnderflow );
    }

    //! Returns the addr (pointer) of the startRoutine of this thread
    void* getStartRoutinePointer() const {
        return (startRoutinePointer);
    }

    //! Returns the addr (pointer) of the exitRoutine of this thread
    void* getExitRoutinePointer() const {
        return (exitRoutinePointer);
    }

    //! Returns the arguments passed to the thread on startup
    void* getStartArguments() const {
        return (this->arguments);
    }

    //! Returns the Memory Manager of the task the thread belongs to
    Kernel_MemoryManagerCfdCl* getMemManager();

    /* Sets the name of this thread */
    void setName(char* newName) {
        int len = strlen(newName);
        if (len > 14) len = 14;
        memcpy(this->name,newName,len);
        this->name[len] = 0;
    }

    /* Gets the name of this thread */
    char* getName() {
        return (this->name);
    }

    /*!
     * \brief This method sets up the thread for execution. This involves telling the scheduler
     * 		 that we are ready for running.
     */
    ErrorT run();

    /*!
     * \brief Sends the thread to sleep until the absolute cycles of 'timePoint' have been reached.
     *
     * Sends the thread to sleep. Involves informing the scheduler.
     */
    void sleep(TimeT timePoint, LinkedListItem* item = pRunningThreadDbItem);

#ifdef ORCOS_SUPPORT_SIGNALS
    /*!
     * Blocks the thread until the signal 'sig' is signaled from some other thread.
     * Caller will NOT return from this method. Instead the context will directly
     * be restored on signal reception!
     */
    void sigwait(void* sig);
#endif

    /*!
     * \brief Blocks the current thread.
     *
     * \param timeout   specifies a timeout in clock ticks this thread shall be woken up anyway. if 0
     *                  the thread may be blocked endlessly. The timeout condition is not perfectly matched
     *                  as the timeout condition is only checked every now and then (e.g. every 250 ms). The
     *                  block value is not supposed to be used for realtime synchronization. It is provided
     *                  to avoid deadlocks on e.g. socket operations.
     *
     * Involves informing the scheduler.
     */
    void block(unint4 timeout = 0);

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

protected:
    /*!
     * \brief Terminates the thread.
     *
     * Involves informing the scheduler. May cause a whole task to be terminated
     * if this is the last thread of a task.
     */
    void terminate();

public:
    /*!
     * \brief Calling this method will setup the thread for first time execution and
     * 		 call the entry function of the thread.
     *
     * This method can be overwritten in order to allow other thread classes like e.g.
     * WorkerThread to implement their own behaviour.
     */
    virtual void callMain();

    /* Returns the Id of this Thread. */
    inline ThreadIdT getId() const {
        return (myThreadId);
    }

};

#endif /* _THREAD_HH */
