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

#include "PriorityThreadScheduler.hh"
#include "kernel/Kernel.hh"
#include "inc/const.hh"
#include "process/RealTimeThread.hh"
#include "assemblerFunctions.hh"

extern Kernel* theOS;

/*****************************************************************************
 * Method: PriorityThreadScheduler::startScheduling()
 *
 * @description
 *  Starts scheduling. As this is a dynamic scheduler nothing has to be
 *  computed beforehand.
 *******************************************************************************/
void PriorityThreadScheduler::startScheduling() {
    /* nothing to do */
}

#ifndef REALTIME
/*****************************************************************************
 * Method: PriorityThreadScheduler::getNextTimerEvent(LinkedListDatabase* sleepList, unint4 dt)
 *
 * @description
*  Returns the next timer event to be programmed for calling the scheduler again. This
 *  may be a preemption point. ONLY used if no realtime scheduler is configured as this
 *  method is overloaded then.
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
int PriorityThreadScheduler::getNextTimerEvent(LinkedListDatabase* sleepList, unint4 dt) {
    /* it makes no sense to return an unint8 here, since theOS->getTimerDevice()->setTimer(nextevent) will take
     * a unint4 anyways (and the return value of this function is used to set the timer event). */
    int4 sleeptime = MAX_INT4;

    // only return a value smaller than sleeptime if there is some other competing threads inside the sleeplist!
    LinkedListItem* pDBSleepItem = sleepList->getHead();
    if (pDBSleepItem != 0) {
        LinkedListItem* pDBNextItem = database.getHead();

        // set variables which are needed to compare to later on, so we do not need to set these for every
        // iteration of the while loop
        TimeT nextPriority;

        if (pDBNextItem != 0) {
            nextPriority = (static_cast<PriorityThread*>(pDBNextItem->getData()))->effectivePriority;
        } else {
            nextPriority = 0;
        }

        // this is the actual computation of the needed intervall. it compares the priority of the current
        // thread with the future prioritys of the threads in the sleeplist. The smallest time intervall where
        // the sleeping threads priority is higher than the current priority will be set as sleeptime.
        do {
            PriorityThread* pSleepThread = static_cast<PriorityThread*>(pDBSleepItem->getData());

            pSleepThread->sleepCycles -= dt;
            if (pSleepThread->sleepCycles <= 0) {
                pSleepThread->status.setBits(cReadyFlag);
                LinkedListItem* litem2 = pDBSleepItem;
                pDBSleepItem = pDBSleepItem->getSucc();

                this->enter(litem2);
            } else {
                if ((pSleepThread->getSleepTime() < sleeptime) && (pSleepThread->effectivePriority > nextPriority))
                    sleeptime = pSleepThread->getSleepTime();

                pDBSleepItem = pDBSleepItem->getSucc();
            }
        } while (pDBSleepItem != 0);
    }

    return sleeptime;
}
#endif

/*****************************************************************************
 * Method: PriorityThreadScheduler::getNext()
 *
 * @description
 *  Returns and removes the head of the priority queue.
 *
 * @returns
 *  ListItem*         The head of the priority queue or null
 *******************************************************************************/
LinkedListItem* PriorityThreadScheduler::getNext() {
    /* we can just return the head since the queue is kept sorted by priority by the
     enter method of this class. */
    LOG(SCHEDULER, INFO, "PriorityThreadScheduler: Queue Size: %d", database.getSize());

#if 0
    printQueue();
#endif
    return (this->database.removeHead());
}

#if 1
void PriorityThreadScheduler::printQueue() {

    printf("Queue: ");
    LinkedListItem* litem = this->database.getHead();
    for (; litem; litem = litem->getSucc()) {
        Thread* thread = static_cast<Thread*>(litem->getData());
        printf(" %d", thread->getId());
    }
    if (this->database.getTail()) {
        Thread* thread = static_cast<Thread*>(this->database.getTail()->getData());
        printf(" Tail: %d", thread->getId());
    }
    printf("\n");
}
#endif

/*****************************************************************************
 * Method: PriorityThreadScheduler::enter(LinkedListItem* item)
 *
 * @description
 *  Inserts the given priority thread into the priority queue based on its
 *  priority
 *
 * @params
 *  item        Linkedlist item of the priority thread to be inserted
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT PriorityThreadScheduler::enter(LinkedListItem* item) {
    ASSERT(item);

    PriorityThread* pPThread = static_cast<PriorityThread*>(item->getData());

    int status;
    DISABLE_IRQS(status);
    /* Enter Thread in the database in accordance with it's priority.
     * The order will keep threads with the same priority in a round robin fashion due to ">="
     **/
    LinkedListItem* sItem = database.getTail();
    while (sItem != 0) {
        if (static_cast<PriorityThread*>(sItem->getData())->effectivePriority >= pPThread->effectivePriority) {
            ErrorT ret = database.insertAfter(item, sItem);
            RESTORE_IRQS(status);
            return (ret);
        }
        sItem = sItem->getPred();
    }

    /* if this statement is reached, no thread with a higher priority than the pRTThread was found,
     so we add it at the very front of the queue. */
    ErrorT ret = database.addHead(item);
    RESTORE_IRQS(status);
    return (ret);
}
