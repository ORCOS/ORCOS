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
#include "inc/signals.hh"
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
#define MAXSTACKPTRS 10

// Flag definitions
//! thread is constructed
#define cNewFlag        (BitmapT)1<<0      // 0x1
//! thread is ready to run
#define cReadyFlag      (BitmapT)1<<1      // 0x2
//! thread has stopped
#define cBlockedFlag    (BitmapT)1<<2      // 0x4
//! thread is terminated
#define cTermFlag       (BitmapT)1<<3      // 0x8
//! thread is waiting for a resource
#define cResource       (BitmapT)1<<4      // 0x10
//! thread execution stopped
#define cStopped        (BitmapT)1<<5      // 0x20
//! thread is waiting for a signal
#define cSignalFlag     (BitmapT)1<<6      // 0x40
//! thread shall terminate after current instance (soft termination)
#define cDoTermFlag     (BitmapT)1<<7      // 0x80

/*!
 * \brief Structure holding information on the stack of a thread
 */
struct ThreadStack {
    void* startAddr;                //!< The logical start addr of this stack
    void* endAddr;                  //!< The logical end addr of this stack
    void* stackptrs[MAXSTACKPTRS];  //!< stack pointer stack. contains pushed stack pointer (context) addresses
    unint1 top;
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

    /* The threads own linked list item which is used to
     * place the thread into a scheduler list. */
    LinkedListItem      myListItem;

public:
    //! The absolute time point in cycles this thread will be sleeping up to
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
     * The last signal value this thread received
     */
    unint4              signalvalue;

    /* The current timeout value of the blocked stated. Periodically
     * decremented if the thread is blocked and blockTimeout is > 0.
     * If the value reaches 0 the thread is unblocked. This allows
     * blocking operations with timeout to prevent system stalls. */
    unint4              blockTimeout;

    /* Constructor which takes the startRoutinePointer as argument */
    Thread(void* startRoutinePointer,
           void* exitRoutinePointer,
           Task* owner,
           unint4 stack_size = DEFAULT_USER_STACK_SIZE,
           void* attr = 0);

    /* Destructor */
    virtual ~Thread();

    /*****************************************************************************
     * Method: initialize()
     *
     * @description
     *  Static initialization of the Thread class
     *******************************************************************************/
    static void initialize() {
        globalThreadIdCounter = cFirstThread;
    }

    /*****************************************************************************
     * Method: getOwner()
     *
     * @description
     *  Returns the owner task of this thread.
     *
     * @returns
     *  Task*        The owning task
     *******************************************************************************/
    inline Task* getOwner() const {
        return (owner);
    }

    /*****************************************************************************
     * Method: getStatus()
     *
     * @description
     * Get the flags of this Thread
     *
     * @returns
     *  Bitmap&        The owning task
     *******************************************************************************/
    inline Bitmap& getStatus() {
        return (status);
    }

    /*****************************************************************************
     * Method: setStatus()
     *
     * @description
     *  Sets all flags of this Thread
     *
     * @params
     *  flags   The flags
     *******************************************************************************/
    inline ErrorT setStatus(Bitmap flags) {
        status = flags;
        return (cOk );
    }

    /*****************************************************************************
     * Method: setStatusFlag()
     *
     * @description
     *  Sets the given flag for this Thread
     *
     * @params
     *  flag   The flag to be set
     *******************************************************************************/
    inline void setStatusFlag(int flag) {
        status.setBits(flag);
    }

    /*****************************************************************************
     * Method: isNew()
     *
     * @description
     *  Returns true iff the thread has never been run before
     *******************************************************************************/
    inline bool isNew() const {
        return (status.areSet( cNewFlag ));
    }

    /*****************************************************************************
     * Method: isReady()
     *
     * @description
     *  Returns true iff the thread is scheduled for execution
     *******************************************************************************/
    inline bool isReady() const {
        return (status.areSet( cReadyFlag ));
    }

    /*****************************************************************************
     * Method: isBlocked()
     *
     * @description
     *   Returns true iff the thread is currently blocked (usually waiting on a
     *       resource)
     *******************************************************************************/
    inline bool isBlocked() const {
        return (status.areSet( cBlockedFlag));
    }

    /*****************************************************************************
     * Method: isStopped()
     *
     * @description
     *   Returns true iff the thread is stopped.
     *******************************************************************************/
    inline bool isStopped() const {
        return (status.areSet( cStopped));
    }

    /*****************************************************************************
     * Method: isSleeping()
     *
     * @description
     *   Returns true iff the thread is currently sleeping
     *******************************************************************************/
    inline bool isSleeping() const {
        return (!status.anySet((BitmapT) cBlockedFlag | cReadyFlag | cNewFlag));
    }

    /*****************************************************************************
     * Method: hasTerminated()
     *
     * @description
     *   Returns true iff the thread has terminated.
     *******************************************************************************/
    inline bool hasTerminated() const {
        return (status.areSet( cTermFlag));
    }

    /*****************************************************************************
     * Method: getSleepTime()
     *
     * @description
     *   Returns the currently set absolute sleeping timepoint
     *******************************************************************************/
    inline TimeT getSleepTime() const {
        return (this->sleepTime);
    }

    /*****************************************************************************
     * Method: pushStackPointer(void* sp)
     *
     * @description
     *  Push the address sp into the threads personal stack.
     *  This stack is to be used to store context related
     *  memory locations of a thread.
     *
     *  On interrupt or syscall the context of the thread has to be
     *  stored and the address should be pushed on this stack.
     *  Nested interrupts may force the use of multiple stack
     *  locations.
     *******************************************************************************/
    inline ErrorT pushStackPointer(void* sp) {
        if (threadStack.top < MAXSTACKPTRS - 1) {
            threadStack.stackptrs[threadStack.top] = sp;
            threadStack.top++;
            return (cOk);
        } else {
            return (cStackOverflow);
        }
    }

    /*****************************************************************************
     * Method: popStackPointer(void* &sp)
     *
     * @description
     * Pop a value from the threads personal stack.
     *******************************************************************************/
    inline ErrorT popStackPointer(void* &sp) {
        if (threadStack.top > 0) {
            threadStack.top--;
            sp = threadStack.stackptrs[threadStack.top];
            return (cOk);
        } else {
            return (cStackUnderflow);
        }
    }

    /*****************************************************************************
     * Method: getStartRoutinePointer()
     *
     * @description
     *  Returns the addr (pointer) of the startRoutine of this thread
     *******************************************************************************/
    inline void* getStartRoutinePointer() const {
        return (startRoutinePointer);
    }

    /*****************************************************************************
     * Method: getExitRoutinePointer()
     *
     * @description
     *  Returns the addr (pointer) of the exitRoutine of this thread
     *******************************************************************************/
    inline void* getExitRoutinePointer() const {
        return (exitRoutinePointer);
    }

    /*****************************************************************************
     * Method: getStartArguments()
     *
     * @description
     *  Returns the arguments passed to the thread on startup
     *******************************************************************************/
    inline void* getStartArguments() const {
        return (this->arguments);
    }

    /*****************************************************************************
     * Method: setName(char* newName)
     *
     * @description
     *  Sets the name of this thread. Maximum 13 chars
     *******************************************************************************/
   inline  void setName(char* newName) {
        int len = strlen(newName);
        if (len > 14) len = 14;
        memcpy(this->name, newName, len);
        this->name[len] = 0;
    }

   /*****************************************************************************
    * Method: getName()
    *
    * @description
    *  Gets the name of this thread
    *******************************************************************************/
    inline char* getName() {
        return (this->name);
    }

    /*****************************************************************************
     * Method: getLinkedListItem()
     *
     * @description
     * Returns the linked list item of this thread to be used to
     * place the thread on the schdulers lists as e.g. sleeplist or blocked list
     * or ready list.
     *
     *******************************************************************************/
    inline LinkedListItem* getLinkedListItem() {
        return (&myListItem);
    }

    /*****************************************************************************
     * Method: run()
     *
     * @description
     *  This method sets up the thread for execution. This involves telling the scheduler
     *  that we are ready for running.
     *******************************************************************************/
    ErrorT run();

    /*****************************************************************************
     * Method: sleep(TimeT timePoint)
     *
     * @description
     *  Sends the thread to sleep until the absolute cycles of 'timePoint' have been reached.
     *******************************************************************************/
    void sleep(TimeT timePoint);

#ifdef ORCOS_SUPPORT_SIGNALS
    /*****************************************************************************
     * Method: sigwait(void* sig)
     *
     * @description
     * Blocks the thread until the signal 'sig' is signaled from some other thread.
     * Caller will NOT return from this method. Instead the context will directly
     * be restored on signal reception!
     *******************************************************************************/
    void sigwait(SignalType signaltype, void* sig);
#endif

    /*****************************************************************************
     * Method: block(unint4 timeout = 0)
     *
     * @description
     *  Blocks the current thread.
     *
     * @params
     *  timeout   specifies a timeout in clock ticks this thread shall be woken up anyway. if 0
     *            the thread may be blocked endlessly. The timeout condition is not perfectly matched
     *            as the timeout condition is only checked every now and then (e.g. every 250 ms). The
     *            block value is not supposed to be used for realtime synchronization. It is provided
     *            to avoid deadlocks on e.g. socket operations.
     *
     *******************************************************************************/
    void block(unint4 timeout = 0);

    /*****************************************************************************
     * Method: unblock()
     *
     * @description
     *  Unblocks the thread that has been waiting on a resource.
     *******************************************************************************/
    void unblock();

    /*****************************************************************************
     * Method: resume()
     *
     * @description
     *  Method needed to resume a thread if it was stopped by the task.
     *******************************************************************************/
    void resume();

    /*****************************************************************************
     * Method: stop()
     *
     * @description
     *   stops the execution of this thread (controlled by task)
     *******************************************************************************/
    void stop();

protected:
    /*****************************************************************************
     * Method: terminate()
     *
     * @description
     *   Terminates the thread.
     *   Involves informing the scheduler. May cause a whole task to be terminated
     *   if this is the last thread of a task.
     *******************************************************************************/
    void terminate();

public:
    /*****************************************************************************
     * Method: callMain()
     *
     * @description
     *   Calling this method will setup the thread for first time execution and
     *   call the entry function of the thread.
     *   This method can be overloaded in order to allow other thread classes like e.g.
     *   WorkerThread to implement their own behavior.
     *******************************************************************************/
    virtual void callMain();

    /*****************************************************************************
     * Method: getId()
     *
     * @description
     *   Returns the Id of this Thread.
     *******************************************************************************/
    inline ThreadIdT getId() const {
        return (myThreadId);
    }
};

#endif /* _THREAD_HH */
