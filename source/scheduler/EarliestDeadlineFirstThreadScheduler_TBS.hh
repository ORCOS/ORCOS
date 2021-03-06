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

#ifndef EARLIESTDEADLINEFIRSTTHREADSCHEDULER_TBS_H_
#define EARLIESTDEADLINEFIRSTTHREADSCHEDULER_TBS_H_

#include "scheduler/PriorityThreadScheduler.hh"
#include "hal/CallableObject.hh"
#include "inc/const.hh"
#include "process/RealTimeThread.hh"

#ifndef MAX
#define MAX(a, b) (a > b ? a : b)

#ifndef INVERSE_UTILIZATION
#error "You must define the inverse utilization 1/U if you want to use the EDF_TBS Scheduler!"

/*!\ingroup scheduler
 * \ingroup realtime
 * \brief Thread scheduler which implements the earliest deadline first scheduling strategy using the
 *        Total Bandwith Server for aperiodic thread scheduling.
 *
 * If you want to use this scheduler you need to provide the inverse utilisation of the periodic task set as a parameter
 * of the constructor!
 *
 * NOTE: This Scheduler will _ONLY_ work with RealTimeThreads. Do not ever try to schedule other
 * thread Types.
 *
 */
class EarliestDeadlineFirstThreadScheduler_TBS: public PriorityThreadScheduler {
private:
    TimeT lastdeadline;
    float inverse_utilization;

public:
    EarliestDeadlineFirstThreadScheduler_TBS() {
        this->inverse_utilization = INVERSE_UTILIZATION;
        lastdeadline = 0;
    }

    /*****************************************************************************
     * Method: computePriority(RealTimeThread* item)
     *
     * @description
     * This method is used by the enter() method of this class to set the priority of a
     * thread each time it is scheduled. Furthermore it is used by the getNextTimerEvent()
     * method to decide on a time interval that minimizes the amount of necessary
     * context switches.
     *---------------------------------------------------------------------------*/
    inline void computePriority(RealTimeThread* item);

    /*****************************************************************************
     * Method: enter(LinkedListItem* item)
     *
     * @description
     *  Enter method which adds an already existing DatabaseItem to the scheduler.
     *
     *  This will enter the database item in accordance with it's priority in the list
     *  of scheduled items. For this purpose a priority will be computed each time in accordance
     *  with EDF priority rules. It also sets the arrival time of the Thread.
     *---------------------------------------------------------------------------*/
    ErrorT enter(LinkedListItem* item);

    /*****************************************************************************
     * Method: enter(ScheduleableItem* item)
     *
     * @description
     *  Enter method which adds a thread to the scheduler for which no DatabaseItem already exists.
     *
     *  Be careful not to use this method if a DatabaseItem already exists for the thread. Otherwise a
     *  superflous DatabaseItem will be generated which results in a memory leak!
     *---------------------------------------------------------------------------*/
    ErrorT enter(ScheduleableItem* item) {
        return (this->enter(new LinkedListItem(item)));
    }


    /*****************************************************************************
     * Method: getNextTimerEvent(LinkedList* sleepList, unint4 dt)
     *
     * @description
     *  The timer event is computed by analyzing all threads in the sleep state (which are normally waiting for their next
     * instance depending on their period). It will be set, so that the running thread will only be interrupted, if a thread
     * with higher priority finishes 'sleeping'. For this purpose it needs to compute the future priority of the sleeping
     * threads, since their priority will change once they are scheduled. If a thread with lower future priority finishes no
     * interrupt will be set, since it cannnot preempt the current thread anyway (this will prevent unnecessary context switches).
     * This method is called by the dispatcher before it calls the getNext() method, so the priority of the 'running' thread
     * can be gotten by looking at the next thread in line (at the head of the scheduling queue). The return value is a 4 byte
     * integer, since the theOS->getTimerDevice()->setTimer() method is implemented to take an unint4 anyways (and we seldom
     * will want to run more than 70 minutes uninterrupted).
     *---------------------------------------------------------------------------*/
    unint4 getNextTimerEvent(LinkedList* sleepList, unint4 dt);
};

/*****************************************************************************
 * Method: EarliestDeadlineFirstThreadScheduler_TBS::computePriority(RealTimeThread* item)
 *
 * @description
 *
 *---------------------------------------------------------------------------*/
inline void EarliestDeadlineFirstThreadScheduler_TBS::computePriority(RealTimeThread* item) {
    if (item->period > 0) {
        // for this we first compute the absolute deadline according to the following formular:
        // arrival time + relative deadline (arrival time contains the time the current instance of this thread has arrived)
        item->absoluteDeadline = item->arrivalTime + item->relativeDeadline;
    } else {
        item->absoluteDeadline = MAX(item->arrivaltime, lastdeadline) + item->executionTime * inverse_utilization;
    }

    item->effectivePriority = ((1 << sizeof(TimeT)) - 1) - item->absoluteDeadline;
    item->initialPriority = item->effectivePriority;
}

#endif /*EARLIESTDEADLINEFIRSTTHREADSCHEDULER_TBS_H_*/
