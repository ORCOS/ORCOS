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

#ifndef LISTSCHEDULER_HH_
#define LISTSCHEDULER_HH_

#include "db/LinkedListItem.hh"
#include "db/LinkedList.hh"
#include "scheduler/Scheduler.hh"

/*!
 * \ingroup scheduler
 * \brief Base class which uses lists to store threads.
 *
 */
class ListScheduler {
public:

    /* the list of schedulableitems that will be scheduled */
    LinkedList database;

public:
    ListScheduler() {
    }

    ~ListScheduler() {
    }

    /*!
     *  \brief Adds a new thread or reactivated service to this scheduler
     */
    ErrorT enter(ScheduleableItem* item) {
        ASSERT(item);
        return (this->database.addTail(item));
    }


    /*!
     *  \brief Enter method which adds an already existing DatabaseItem to the scheduler.
     *
     *  Implemented to avoid reallocating memory for scheduleableitems if reentering the scheduler.
     */
    ErrorT enter(LinkedListItem* item) {
        ASSERT(item);
        return (this->database.addTail(item));
    }


    /*!
     * \brief Removes the item from the scheduler
     */
    LinkedListItem* remove(ScheduleableItem* it) {
        LinkedListItem* item = (LinkedListItem*) this->database.getItem(it);
        if (item != 0) {
            item->remove();
            return (item);
        }
        return (0);
    }

    /*!
     * \brief Returns true if at least one element is in the queue.
     */
    bool isEmpty() {
        return (this->database.isEmpty());
    }

};

#endif /*LISTSCHEDULER_HH_*/
