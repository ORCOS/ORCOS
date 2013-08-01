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
#include "db/ArrayDatabase.hh"
#include Mutex_Scheduler_hh
#include ThreadCfd_hh

class Resource;
class Task;

/*!\ingroup synchro
 * \brief Provides mutex functionality (-> a binary semaphore), optionally with resource management.
 *
 * 	This Mutex class provides normal mutex functionality. If the configured Scheduler
 *  is NONE, Mutex::acquire() will return cNotImplemented.
 *
 *  This implementation is save regarding interrupts since a testandset method is used.
 */
class Mutex {

private:

    /*! \brief This member variable stores if the mutex is aquired or not.
     *
     * This should ALWAYS be the first variable of the Mutex class so it is guaranteed to be word
     * aligned.
     */
    int4 				m_locked;

    //! Stores a pointer to the thread currently locking this mutex.
    ThreadCfdCl* 		m_pThread;

    /*! \brief Is used to store unused LinkedList Database Items
     *
     * We internally keep track of free linked list database items so that we do not have to
     * create new ones every time we need it (e.g. to add a thread to the scheduler) and delete it
     * afterwards. This saves a bit of execution time (no unneccesary creating / deleting of objects)
     * and makes it possible to use the Mutex class with a linear memory manager (which does not
     * support release of memory) without memory leaks.
     */
    LinkedListDatabase 	m_unusedLinkedListDBItems;

    //! Stores threads which are stopped by the user, but will want to acquire the mutex when they are resumed.
    ArrayDatabase 		m_stoppedThreads;

    //! Stores a pointer to the Resource guarded by this mutex.
    Resource* 			m_pRes;

    //! configurable member scheduler
DEF_Mutex_SchedulerCfd
    // With SCL configured Scheduler


public    :
    //! Constructor, initializes m_locked to false, so the Mutex can be aquired, and initializes the scheduler.
    Mutex();

    //! Destructor, deletes scheduler object, if it has been allocated.
    ~Mutex();

    /*! \brief Method used to aquire a Mutex
     *
     * pCurrentRunningThread tries to aquire the mutex.
     */
    ErrorT acquire(Resource* = 0, bool blocking = true);

    /*! \brief Releases an acquired Mutex object.
     *
     * pCurrentRunningThread releases the mutex.
     */
    ErrorT release();

    /*! \brief Method called whenever a thread is resumed.
     *
     * This method is not called within the mutex itself but from the thread_resume syscall. It makes sure to
     * reschedule the specified thread, if it has been added to the stoppedThreads list.
     */
    void threadResume(ThreadCfdCl* pThread);

};

#endif  // MUTEX_HH

