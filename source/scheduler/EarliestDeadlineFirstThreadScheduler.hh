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

#ifndef EARLIESTDEADLINEFIRSTTHREADSCHEDULER_H_
#define EARLIESTDEADLINEFIRSTTHREADSCHEDULER_H_


#include "scheduler/PriorityThreadScheduler.hh"
#include "hal/CallableObject.hh"
#include "inc/const.hh"
#include "process/RealTimeThread.hh"


/*!\ingroup scheduler
 * \ingroup realtime
 * \brief Thread scheduler which implements the earliest deadline first scheduling strategy.
 *
 * NOTE: This Scheduler will _ONLY_ work with RealTimeThreads. Do not ever try to schedule other
 * thread Types.
 *
 */
class EarliestDeadlineFirstThreadScheduler: public PriorityThreadScheduler {
public:
    /*! \brief Computes the priority for the given thread according to EDF rules.
     *
     * This method is used by the enter() method of this class to set the priority of a
     * thread each time it is scheduled. Furthermore it is used by the getNextTimerEvent()
     * method to decide on a time interval that minimizes the amount of necessary
     * context switches.
     */
   void computePriority( RealTimeThread* item );

    /*!
     *  \brief Enter method which adds an already existing DatabaseItem to the scheduler.
     *
     * 	This will enter the database item in accordance with it's priority in the list
     *  of scheduled items. For this purpose a priority will be computed each time in accordance
     *  with EDF priority rules. It also sets the arrival time of the Thread.
     */
    ErrorT enter( LinkedListDatabaseItem* item );

    /*! \brief Enter method which adds a thread to the scheduler for which no DatabaseItem already exists.
     *
     * Be careful not to use this method if a DatabaseItem already exists for the thread. Otherwise a
     * superflous DatabaseItem will be generated which results in a memory leak!
     */
    ErrorT enter( ScheduleableItem* item ) {
        return (this->enter( new LinkedListDatabaseItem( item ) ));
    }

    /*! \brief Method returning the amount of microseconds for next timer event.
     *
     * The timer event is computed by analyzing all threads in the sleep state (which are normally waiting for their next
     * instance depending on their period). It will be set, so that the running thread will only be interrupted, if a thread
     * with higher priority finishes 'sleeping'. For this purpose it needs to compute the future priority of the sleeping
     * threads, since their priority will change once they are scheduled. If a thread with lower future priority finishes no
     * interrupt will be set, since it cannnot preempt the current thread anyway (this will prevent unnecessary context switches).
     * This method is called by the dispatcher before it calls the getNext() method, so the priority of the 'running' thread
     * can be gotten by looking at the next thread in line (at the head of the scheduling queue). The return value is a 4 byte
     * integer, since the theOS->getTimerDevice()->setTimer() method is implemented to take an unint4 anyways (and we seldom
     * will want to run more than 70 minutes uninterrupted).
     */
    unint4 getNextTimerEvent(LinkedListDatabase* sleepList,unint4 dt);

};




#endif /*EARLIESTDEADLINEFIRSTTHREADSCHEDULER_H_*/
