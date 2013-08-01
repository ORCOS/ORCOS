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

#ifndef SCHEDULER_HH_
#define SCHEDULER_HH_

#include "inc/const.hh"
#include "inc/types.hh"

#include "scheduler/ScheduleableItem.hh"

/*!
 * \ingroup scheduler
 * \brief The base Scheduler class defining a set of methods to be implemented by derived classes.
 *
 *
 * The Scheduler is the mainly used to schedule the execution of threads on the cpu but may also be used
 * used by synrchonisation objects.
 *
 */
class Scheduler {
public:

    /*!
     * \brief The schedule policy implemented in this method.
     */
    DatabaseItem* getNext() {
        return 0;
    }
    ;

    /*!
     * \brief Announces a new item to the Scheduler, that should be scheduled
     */
    ErrorT enter( ScheduleableItem* ) {
        return cNotImplemented;
    }

    /*!
     * \brief Removes the item from the scheduler
     */
    ErrorT remove( ScheduleableItem* ) {
        return cNotImplemented;
    }

    /*!
     * \brief Returns true if at least one element is in the queue.
     */
    bool isEmpty() {
        return cNotImplemented;
    }

};

#endif /*SCHEDULER_HH_*/
