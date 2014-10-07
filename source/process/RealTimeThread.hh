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

#ifndef REALTIMETHREAD_HH_
#define REALTIMETHREAD_HH_
#define REALTIME

#include "process/PriorityThread.hh"
#include "inc/const.hh"

/*!
 * \brief A thread class for realtime threads.
 * \ingroup process
 * \ingroup realtime
 *
 * This class adds deadlines and periods to the priority thread base class.
 */
class RealTimeThread: public PriorityThread {

// friend classes
    friend class EarliestDeadlineFirstThreadScheduler;
    friend class RateMonotonicThreadScheduler;
    friend class WorkerTask;
    friend class Task;
    friend class TaskManager;

public:
    TimeT period;               //!< The period of the real time thread in clock ticks. 0 means it isn't periodic.
    TimeT relativeDeadline;     //!< The relative deadline of the thread for every instance.
    TimeT absoluteDeadline;     //!< The current absolute deadline of the thread
    TimeT executionTime;        //!< The execution time of the thread if it's known (otherwise 0).
    TimeT arrivalTime;          //!< The arrival time of the current instance
    int instance;               //!< The instance of this realtime thread

public:
    /*! \brief This constructor provides default values for periode and executionTime.
     *
     * - Deadline describes the relative deadline to thread start.
     * - A periode of 0 means that the real time thread is aperiodic.
     * - An execution time of 0 means that the execution time for the thread is unknown.
     */
    RealTimeThread(void* startRoutinePointer, void* exitRoutinePointer, Task* owner, Kernel_MemoryManagerCfdCl* memManager, unint4 stack_size =
                           DEFAULT_USER_STACK_SIZE, void* RTThreadAttributes = 0, bool newThread =
                           true);

    /*! \brief Standard destructor for the real time thread class.
     *
     * Careful: this destructor is not virtual in accordance with the policy to minimize the use of
     * virtual in order to get a small footprint. Always use this class directly (as configured by SCL)
     * never through a pointer to a superclass. Otherwise memory leaks _will_ happen.
     */
    ~RealTimeThread();

protected:
    /*! \brief Is automatically called when the thread finishes through a syscall.
     *
     * If the period is not zero (meaning it is a periodic thread) this method will take care that the thread is
     * rescheduled. For this purpose the stack pointer is reset so when the thread runs again it will start at the
     * beginning without having to initialize everything again. It will then be entered in the sleeplist for the
     * remaining time till the next execution is due.
     */
    void terminate();

};

#endif /*REALTIMETHREAD_HH_*/
