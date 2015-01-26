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

#ifndef MUTEX_HH
#define MUTEX_HH

// includes
#include "SCLConfig.hh"
#include "inc/const.hh"
#include "db/ArrayList.hh"
#include Kernel_Scheduler_hh
#include Kernel_Thread_hh
#include Kernel_MemoryManager_hh

class Resource;
class Task;

extern Kernel_ThreadCfdCl* pCurrentRunningThread;

/*!
 * \ingroup synchro
 * \brief Provides mutex functionality (-> a binary semaphore), optionally with resource management.
 *
 *     This Mutex class provides normal mutex functionality with priority inheritance and scheduling functionality.
 *
 *  This implementation is save regarding interrupts since it disables interrupts in its critical code parts.
 *
 */
class Mutex {
private:
    /*! \brief This member variable stores if the mutex is aquired or not.
     *
     * This should ALWAYS be the first variable of the Mutex class so it is guaranteed to be word
     * aligned.
     */
    int2        m_locked;

    /*
     * Number of waiting threads
     */
    int2        waitingThreads;

    /*! Stores a pointer to the thread currently locking this mutex. */
    Kernel_ThreadCfdCl* m_pThread;

    //! Stores a pointer to the Resource guarded by this mutex.
    Resource*   m_pRes;

    //! The priority at acquisition time
    TimeT       acquirePriority;

public:
    //! Constructor, initializes m_locked to false, so the Mutex can be aquired, and initializes the scheduler.
    Mutex();

    //! Destructor, deletes scheduler object, if it has been allocated.
    ~Mutex();

    /*****************************************************************************
     * Method: acquire(Resource* = 0, bool blocking = true)
     *
     * @description
     *  Tries to acquire the mutex. If blocking == true the running thread will be blocked if the
     *  mutex can not be acquired. If a resource is provided the resource will be added to the running threads
     *  acquired resources in success.
     *---------------------------------------------------------------------------*/
    ErrorT acquire(Resource* = 0, bool blocking = true);

    /*****************************************************************************
     * Method: release(Thread* pThread = pCurrentRunningThread)
     *
     * @description
     *  Releases the current Mutex
     *---------------------------------------------------------------------------*/
    ErrorT release(Thread* pThread = pCurrentRunningThread);



    /*****************************************************************************
     * Method: getWaitingThreadCount()
     *
     * @description
     *  Returns the current numnber of threads waiting on this mutex.
     *---------------------------------------------------------------------------*/
    unint2 getWaitingThreadCount() { return (waitingThreads); }
};

#endif  // MUTEX_HH

