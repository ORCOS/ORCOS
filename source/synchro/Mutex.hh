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
#include <assemblerFunctions.hh>
#include "db/ArrayList.hh"
#include Kernel_Scheduler_hh
#include Kernel_Thread_hh

class Resource;
class Task;

/*!
 * \ingroup synchro
 * \brief Provides mutex functionality (-> a binary semaphore), optionally with resource management.
 *
 * 	This Mutex class provides normal mutex functionality with priority inheritance and scheduling functionality.
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
    int4 				m_locked;

    /*! Stores a pointer to the thread currently locking this mutex. */
    Kernel_ThreadCfdCl* m_pThread;

#if MEM_NO_FREE
    /*! \brief Is used to store unused LinkedList Database Items
     *
     * We internally keep track of free linked list database items so that we do not have to
     * create new ones every time we need it (e.g. to add a thread to the scheduler) and delete it
     * afterwards. This saves a bit of execution time (no unneccesary creating / deleting of objects)
     * and makes it possible to use the Mutex class with a linear memory manager (which does not
     * support release of memory) without memory leaks.
     */
    LinkedList 	        m_unusedLinkedListDBItems;
#endif

    //! Stores threads which are stopped by the user, but will want to acquire the mutex when they are resumed.
    ArrayList 		    m_stoppedThreads;

    //! Stores a pointer to the Resource guarded by this mutex.
    Resource* 		    m_pRes;

    //! configurable member scheduler
    DEF_Kernel_SchedulerCfd

public    :

    //! Constructor, initializes m_locked to false, so the Mutex can be aquired, and initializes the scheduler.
    Mutex();

    //! Destructor, deletes scheduler object, if it has been allocated.
    ~Mutex();

    /*!
     * \brief Method used to acquire a Mutex
     *
     * Tries to acquire the mutex. If blocking == true the running thread will be blocked if the
     * mutex can not be acquired. If a resource is provided the resource will be added to the running threads
     * aquired resources in success.
     *
     */
    ErrorT acquire(Resource* = 0, bool blocking = true);

    /*!
     * \brief Releases an acquired Mutex object.
     *
     * pCurrentRunningThread releases the mutex.
     */
    ErrorT release();

    /*!
     * \brief Method called whenever a thread is resumed.
     *
     * This method is not called within the mutex itself but from the thread_resume syscall. It makes sure to
     * reschedule the specified thread, if it has been added to the stoppedThreads list.
     */
    void threadResume(Kernel_ThreadCfdCl* pThread);

};

#endif  // MUTEX_HH

