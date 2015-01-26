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

#include "RateMonotonicThreadScheduler.hh"
#include "kernel/Kernel.hh"
#include "inc/const.hh"
#include "archtypes.h"

extern Kernel* theOS;

/*****************************************************************************
 * Method: RateMonotonicThreadScheduler::getNextTimerEvent( LinkedList* sleepList, TimeT currentTime )
 *
 * @description
 *  Returns the next timer event to be programmed for calling the scheduler again. This
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
TimeT RateMonotonicThreadScheduler::getNextTimerEvent(LinkedList* sleepList, TimeT currentTime) {
    /* it makes no sense to return an unint8 here, since theOS->getTimerDevice()->setTimer(nextevent) will take
     * a unint4 anyways (and the return value of this function is used to set the timer event). */
    TimeT sleeptime = MAX_UINT8;

    /* only return a value smaller than sleeptime if there is some other competing threads inside the sleeplist! */
    LinkedListItem* pDBSleepItem = sleepList->getHead();
    if (pDBSleepItem != 0) {
        LinkedListItem* pDBNextItem = database.getHead();

        /* set variables which are needed to compare to later on, so we do not need to set these for every
         * iteration of the while loop */
        TimeT nextPriority;

        if (pDBNextItem != 0) {
            nextPriority = (static_cast<RealTimeThread*>(pDBNextItem->getData()))->effectivePriority;
        } else {
            nextPriority = 0;
        }

        /* this is the actual computation of the needed interval. it compares the priority of the current
         thread with the future priorities of the threads in the sleeplist. The smallest time interval where
         the sleeping threads priority is higher than the current priority will be set as sleeptime.*/
        do {
            RealTimeThread* pSleepThread = static_cast<RealTimeThread*>(pDBSleepItem->getData());

            if (pSleepThread->sleepTime <= currentTime) {
                pSleepThread->status.setBits(cReadyFlag);
                LinkedListItem* litem2  = pDBSleepItem;
                pDBSleepItem            = pDBSleepItem->getSucc(); /* get the next item here is the next line will change the succ */
                litem2->remove();
                pSleepThread->sleepTime = 0;
                this->enter(litem2);
                /* get new head.. and use its priority */
                pDBNextItem = database.getHead();
                nextPriority = (static_cast<RealTimeThread*>(pDBNextItem->getData()))->effectivePriority;
            } else {
                if ((pSleepThread->getSleepTime() <= sleeptime) && (pSleepThread->effectivePriority >= nextPriority)) {
                    sleeptime = pSleepThread->getSleepTime();
                }

                pDBSleepItem            = pDBSleepItem->getSucc();
            }
        } while (pDBSleepItem != 0);
    }

    return (sleeptime);
}

/*****************************************************************************
 * Method: RateMonotonicThreadScheduler::computePriority(RealTimeThread* pRTThread)
 *
 * @description
 *  Computes and assignes the proiority of/to the given thread based on the the
 *  Rate Monotonic priority rule.
 *
 * @params
 *  pRTThread   Realtime Thread
 *******************************************************************************/
void RateMonotonicThreadScheduler::computePriority(RealTimeThread* pRTThread) {
    TimeT priority = pRTThread->initialPriority;

    /* This computation assures, that a smaller period will result in a higher priority
     * for periodic threads. non periodic thread priority will not be changed! */
    if (pRTThread->period != 0)
        priority = (MAX_UINT4 - pRTThread->period);

    pRTThread->initialPriority      = priority;
    pRTThread->effectivePriority    = priority;
}

/*****************************************************************************
 * Method: RateMonotonicThreadScheduler::enter(LinkedListItem* item)
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
ErrorT RateMonotonicThreadScheduler::enter(LinkedListItem* item) {
    RealTimeThread* pRTThread = static_cast<RealTimeThread*>(item->getData());

    LOG(SCHEDULER, DEBUG, "RateMonotonicThreadScheduler::enter() Thread %d , Priority: %x %x", pRTThread->getId(),
        (unint4) ((pRTThread->effectivePriority >> 32) & 0xffffffff),
        (unint4) ((pRTThread->effectivePriority) & 0xffffffff));

    /* set priority if it has not been set yet.*/
    if (pRTThread->instance == 1) {
        computePriority(pRTThread);

        /* If an arrivaltime has been specified put the thread to sleep until that time */
        if (pRTThread->sleepTime > 0) {
            LOG(SCHEDULER, DEBUG, "RateMonotonicThreadScheduler::enter: TimePoint: %x %x",
                (unint4) ((pRTThread->sleepTime >> 32) & 0xffffffff),
                (unint4) ((pRTThread->sleepTime) & 0xffffffff));
            pRTThread->sleep(pRTThread->sleepTime);
            return (cOk );
        }
    }

    /* enter Thread in the database in accordance with it's priority */
    return (PriorityThreadScheduler::enter(item));
}
