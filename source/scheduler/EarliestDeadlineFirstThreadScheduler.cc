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

#include "EarliestDeadlineFirstThreadScheduler.hh"
#include "kernel/Kernel.hh"
#include "inc/const.hh"

extern Kernel* theOS;

ErrorT EarliestDeadlineFirstThreadScheduler::enter( LinkedListItem* item ) {
    ASSERT(item);
    RealTimeThread* pRTThread = static_cast< RealTimeThread* > ( item->getData() );

    /* If a phase has been specified, put the thread to sleep for the specified duration. */
    if ( pRTThread->phase > 0 ) {
        pRTThread->sleep( (unint4) pRTThread->phase, item );
        pRTThread->phase = 0;
        return (cOk);
    }

    /* enter Thread in the database in accordance with it's priority */
    return (PriorityThreadScheduler::enter( item ));

}

TimeT EarliestDeadlineFirstThreadScheduler::getNextTimerEvent( LinkedList* sleepList,TimeT currentTime ) {
    ASSERT(sleepList);
    /* it makes no sense to return an unint8 here, since theOS->getTimerDevice()->setTimer(nextevent) will take
       a unint4 anyways (and the return value of this function is used to set the timer event). */
    TimeT sleeptime = MAX_UINT8;

    /* only return a value smaller than sleeptime if there is some other competing threads inside the sleeplist! */
    LinkedListItem* pDBSleepItem = sleepList->getHead();
    if ( pDBSleepItem != 0 ) {

        /* first update the sleeptime and wake up waiting/sleeping threads */
        do {
            RealTimeThread* pSleepThread = static_cast< RealTimeThread*> ( pDBSleepItem->getData() );

            if ( pSleepThread->sleepTime <= currentTime ) {
				pSleepThread->status.setBits( cReadyFlag );
				LinkedListItem* litem2 = pDBSleepItem;
				pDBSleepItem = pDBSleepItem->getSucc();
				pSleepThread->sleepTime = 0;
				/* This thread is active again. Enter the queue again. If it has the highest
				 * priority it will be chosen next. */
				this->enter( litem2 );
            } else
                pDBSleepItem = pDBSleepItem->getSucc();
        } while ( pDBSleepItem != 0 );

        /* set variables which are needed to compare to later on, so we do not need to set these for every
           iteration of the while loop */
        TimeT nextPriority = 0;
        LinkedListItem* pDBNextItem = database.getHead();

        if ( pDBNextItem != 0 )
            nextPriority = (static_cast< RealTimeThread* > ( pDBNextItem->getData() ))->effectivePriority;

        pDBSleepItem = sleepList->getHead();

        /* this is the actual computation of the needed interval. it compares the priority of the current
           thread with the future priorities of the threads in the sleeplist. The smallest time intervall where
           the sleeping threads priority is higher than the current priority will be set as sleeptime in
           order to preempt the running thread at that time point */
        while ( pDBSleepItem != 0 ) {
            RealTimeThread* pSleepThread = static_cast< RealTimeThread*> ( pDBSleepItem->getData() );
			if ( ( pSleepThread->getSleepTime() < sleeptime ) && ( pSleepThread->effectivePriority > nextPriority ) )
					 sleeptime = pSleepThread->getSleepTime();

			pDBSleepItem = pDBSleepItem->getSucc();
        }

    }

    return (sleeptime);

}

 void EarliestDeadlineFirstThreadScheduler::computePriority( RealTimeThread* item ) {

	 /* for this we first compute the absolute deadline according to the following formula:
        arrival time + relative deadline (arrival time contains the time the current instance of this thread has arrived) */
	if (item->relativeDeadline != 0)
	{
		item->absoluteDeadline     = item->arrivalTime + item->relativeDeadline;
		item->effectivePriority    = MAX_UINT8 - item->absoluteDeadline;
		item->initialPriority      = item->effectivePriority;
	}
	/* only change the priority if it has not been set by the prioritythread */
	else if (item->initialPriority == cDefaultPriority)
	{
		item->initialPriority      = 1;
		item->absoluteDeadline     = 0;
		item->effectivePriority    = 1;
	}

    LOG(SCHEDULER,DEBUG,"EDF: Thread %d Priority=%x%x",
        item->getId(),
        (unint4) ((item->effectivePriority >> 32) & 0xffffffff),
        (unint4) ((item->effectivePriority) & 0xffffffff));

 }
