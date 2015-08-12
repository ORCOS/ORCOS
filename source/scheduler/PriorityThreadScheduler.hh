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

#ifndef PRIORITYTHREADSCHEDULER_HH_
#define PRIORITYTHREADSCHEDULER_HH_

// includes
#include "scheduler/ListScheduler.hh"
#include "hal/CallableObject.hh"

/*!\ingroup scheduler
 * \brief Simple thread scheduler which schedules threads according to their priority.
 *
 *    This scheduler will not assign or change prioritys. Only use this scheduler with priority or
 *  realtime threads... if the basic thread class is configured (which has no prioritys) bad
 *  things will occur (yes really).
 */
class PriorityThreadScheduler: public ListScheduler {
public:
    /*****************************************************************************
     * Method: enter(LinkedListItem* item)
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
    ErrorT enter(LinkedListItem* item);

    /*****************************************************************************
     * Method: enter(LinkedListItem* item)
     *
     * @description
     *  Enter method which adds a thread to the scheduler for which no DatabaseItem already exists.
     *  Be careful not to use this method if a DatabaseItem already exists for the thread. Otherwise a
     *  superflous DatabaseItem will be generated which results in a memory leak!
     *
     * @params
     *  item        Linkedlist item of the priority thread to be inserted
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT enter(ScheduleableItem* item) {
        return (this->enter(new LinkedListItem(item)));
    }

    /*****************************************************************************
     * Method: getNext()
     *
     * @description
     *  Returns and removes the head of the priority queue.
     *
     * @returns
     *  ListItem*         The head of the priority queue or null
     *******************************************************************************/
    LinkedListItem* getNext();

    /*****************************************************************************
     * Method: startScheduling()
     *
     * @description
     *  Starts scheduling. As this is a dynamic scheduler nothing has to be
     *  computed beforehand.
     *******************************************************************************/
    void startScheduling();

#if 1
    void printQueue();
#endif

#ifndef REALTIME
    /*****************************************************************************
     * Method: getNextTimerEvent(LinkedListDatabase* sleepList, unint4 dt)
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
    int getNextTimerEvent(LinkedList* sleepList, unint4 dt);
#endif
};

#endif /*PRIORITYSCHEDULER_HH_*/
