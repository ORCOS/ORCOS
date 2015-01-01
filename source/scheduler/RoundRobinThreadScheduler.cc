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

#include "scheduler/RoundRobinThreadScheduler.hh"
#include "process/Thread.hh"
#include "inc/const.hh"
#include "kernel/Kernel.hh"

// the kernel object
extern Kernel* theOS;

RoundRobinThreadScheduler::RoundRobinThreadScheduler() {
}

RoundRobinThreadScheduler::~RoundRobinThreadScheduler() {
}

/*****************************************************************************
 * Method: RoundRobinThreadScheduler::getNext()
 *
 * @description
 *  Returns the next item by round robin fashion.
 *
 * @returns
 *  ListItem         The next item
 *******************************************************************************/
LinkedListItem* RoundRobinThreadScheduler::getNext() {
    LinkedListItem* nextItem = this->database.removeHead();

    /* this implies that this thread will be the active thread then */
    return (nextItem);
}


/****************************************************************************
 * Method: RoundRobinThreadScheduler::getNextTimerEvent( LinkedList* sleepList, TimeT currentTime )
 *
 * @description
 *  Returns the next timer event to be programmed for calling the scheduler again. This
 *  may be a preemption point. This is the round robin time slice.
 *
 * @params
 *  sleepList       The list of sleeping threads which are updated.
 *  currentTime     Current system time.
 * @returns
 *  TimeT           The next absolute time point the scheduler has to be called again
 *******************************************************************************/
TimeT RoundRobinThreadScheduler::getNextTimerEvent(LinkedList* sleepList, TimeT currentTime) {
    ASSERT(sleepList);
    TimeT shortest_sleeptime = 0x0fffffffffffffffll;//MAX_UINT8;

    /* update sleeplist */
    LinkedListItem* pDBSleepItem = sleepList->getHead();
    if (pDBSleepItem != 0) {
        do {
            Thread* pSleepThread = static_cast<Thread*>(pDBSleepItem->getData());

            if (pSleepThread->sleepTime <= currentTime) {
                pSleepThread->status.setBits(cReadyFlag);
                LinkedListItem* litem2 = pDBSleepItem;
                pDBSleepItem = pDBSleepItem->getSucc();
                pSleepThread->sleepTime = 0;
                this->enter(litem2);
            } else {
                if (pSleepThread->getSleepTime() < shortest_sleeptime)
                    shortest_sleeptime = pSleepThread->getSleepTime();
                pDBSleepItem = pDBSleepItem->getSucc();
            }
        } while (pDBSleepItem != 0);
    }

    if (database.isEmpty())
        return (shortest_sleeptime);
    else
        return (RRTimeSlice);
}
