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
    /*****************************************************************************
     * Method: computePriority(RealTimeThread* pRTThread)
     *
     * @description
     *  Computes and assignes the proiority of/to the given thread based on the the
     *  Rate Monotonic priority rule.
     *  RM only computes the priority once for the first instance of the thread.
     *
     * @params
     *  pRTThread   Realtime Thread
     *******************************************************************************/
    void computePriority(RealTimeThread* item);


    /*****************************************************************************
     * Method: enter(LinkedListItem* item)
     *
     * @description
     *  Inserts and schedules a new realtime thread, given by its linkedlist item,
     *  into the scheduler queue.
     *  This will enter the database item in accordance with it's priority in the list
     *  of scheduled items. It also sets the arrival time of the Thread. Further a priority will
     *  be assigned in accordance with the rate monotonic method (a shorter periode means higher
     *  priority) if no priority has been assigned yet.
     *
     * @params
     *  item        Linked List item of the Realtime Thread
     *******************************************************************************/
    ErrorT enter(LinkedListItem* item);

    /*****************************************************************************
     * Method: enter(LinkedListItem* item)
     *
     * @description
     *  Inserts and schedules a new realtime thread, given by its linkedlist item,
     *  into the scheduler queue.
     *  Be careful not to use this method if a DatabaseItem already exists for the thread. Otherwise a
     *  superflous DatabaseItem will be generated which results in a memory leak!
     *
     * @params
     *  item        Linked List item of the Realtime Thread
     *******************************************************************************/
    ErrorT enter(ScheduleableItem* item) {
        return (this->enter(new LinkedListItem(item)));
    }

    /*****************************************************************************
     * Method: getNextTimerEvent( LinkedList* sleepList, TimeT currentTime )
     *
     * @description
     *  Returns the next timer event to be progammed for calling the scheduler again. This
     *  may be a preemption point.
     *  The timer event is computed by analyzing all threads in the sleep state (which are normally waiting for their next
     *  instance depending on their period). It will be set, so that the running thread will only be interrupted, if a thread
     *  with higher priority finishes 'sleeping'. If a thread with lower priority finishes no interrupt will be set, since
     *  it cannnot preempt the current thread anyway (this will prevent unnecessary context switches). This method is called
     *  by the dispatcher before it calls the getNext() method, so the priority of the 'running' thread can be gotten by looking
     *  at the next thread in line (at the head of the scheduling queue).
     *
     * @params
     *  sleepList       The list of sleeping threads which are updated.
     *  currentTime     Current system time.
     * @returns
     *  TimeT           The next absolute time point the scheduler has to be called again
     *******************************************************************************/
    TimeT getNextTimerEvent(LinkedList* sleepList, TimeT currentTime);
};

#endif /*RATEMONOTONICTHREADSCHEDULER_*/
