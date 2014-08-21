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
 *	This scheduler will not assign or change prioritys. Only use this scheduler with priority or
 *  realtime threads... if the basic thread class is configured (which has no prioritys) bad
 *  things will occur (yes really).
 */
class PriorityThreadScheduler: public ListScheduler {

public:

    /*!
     *  \brief Enter method which adds an already existing DatabaseItem to the scheduler.
     *
     *  This will enter the database item in accordance with it's priority in the list
     *  of scheduled items.
     */
    ErrorT enter( LinkedListItem* item );

    /*!
     * \brief Enter method which adds a thread to the scheduler for which no DatabaseItem already exists.
     *
     * Be careful not to use this method if a DatabaseItem already exists for the thread. Otherwise a
     * superflous DatabaseItem will be generated which results in a memory leak!
     */
    ErrorT enter( ScheduleableItem* item ) {
        return (this->enter( new LinkedListItem( item ) ));
    }

    /*!
     * \brief This method returns the next thread to be executed.
     *
     * Since the actual scheduling logic is implemented in the enter method, this will always return the first
     * item of the internal database of threads ready to run.
     */
    ListItem* getNext();

    /*!
     * \brief The initialisation method called just before scheduling begins, enables the timer device.
     *
     */
    void startScheduling();

    /*!
     * \brief Method returning the amount of microseconds for next timer event.
     *
     */
#ifndef REALTIME
    int getNextTimerEvent(LinkedList* sleepList,unint4 dt);
#endif

};

#endif /*PRIORITYSCHEDULER_HH_*/
