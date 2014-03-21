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

#ifndef RATEMONOTONICTHREADSCHEDULER_
#define RATEMONOTONICTHREADSCHEDULER_


#include "process/RealTimeThread.hh"
#include "scheduler/PriorityThreadScheduler.hh"
#include "hal/CallableObject.hh"

/*!\ingroup scheduler
 * \ingroup realtime
 * \brief Thread scheduler which implements the rate monotonic scheduling.
 *
 * NOTE: This Scheduler will _ONLY_ work with RealTimeThreads. Do not ever try to schedule other
 * thread Types.
 */
class RateMonotonicThreadScheduler: public PriorityThreadScheduler {

public:
    /*!
     * \brief Empty method for RM.
     *
     * RM only computes the priority once for the first instance of the thread.
     */
    inline void computePriority( RealTimeThread* item ) {};

    /*!
     *  \brief Enter method which adds an already existing DatabaseItem to the scheduler.
     *
     *  This will enter the database item in accordance with it's priority in the list
     *  of scheduled items. It also sets the arrival time of the Thread. Further a priority will
     *  be assigned in accordance with the rate monotonic method (a shorter periode means higher
     *  priority) if no priority has been assigned yet.
     */
    ErrorT enter( LinkedListDatabaseItem* item );

    /*!
     * \brief Enter method which adds a thread to the scheduler for which no DatabaseItem already exists.
     *
     * Be careful not to use this method if a DatabaseItem already exists for the thread. Otherwise a
     * superflous DatabaseItem will be generated which results in a memory leak!
     */
    ErrorT enter( ScheduleableItem* item ) {
        return (this->enter( new LinkedListDatabaseItem( item ) ));
    }

    /*!
     * \brief Method returning the amount of microseconds for next timer event.
     *
     * The timer event is computed by analyzing all threads in the sleep state (which are normally waiting for their next
     * instance depending on their period). It will be set, so that the running thread will only be interrupted, if a thread
     * with higher priority finishes 'sleeping'. If a thread with lower priority finishes no interrupt will be set, since
     * it cannnot preempt the current thread anyway (this will prevent unnecessary context switches). This method is called
     * by the dispatcher before it calls the getNext() method, so the priority of the 'running' thread can be gotten by looking
     * at the next thread in line (at the head of the scheduling queue).
     */
    TimeT getNextTimerEvent(LinkedListDatabase* sleepList, TimeT currentTime );

};

#endif /*RATEMONOTONICTHREADSCHEDULER_*/
