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

#ifndef SINGLETHREADSCHEDULER_H_
#define SINGLETHREADSCHEDULER_H_

#include <SCLConfig.hh>
#include Kernel_Thread_hh

/*!
 * \ingroup scheduler
 * \brief Scheduler only schedules one single thread. Special Class.
 *
 */
class SingleThreadScheduler {
    LinkedListItem* singleThread;
    LinkedListItem  wrappingItem;
public:
    SingleThreadScheduler();
    ~SingleThreadScheduler();

    /*****************************************************************************
    * Method: computePriority(Kernel_ThreadCfdCl* item)
    *
    * @description
    *  Dummy implementation as no priority needed with a single thread.
    *---------------------------------------------------------------------------*/
    inline void computePriority(Kernel_ThreadCfdCl* item) {
    }


     /*****************************************************************************
     * Method: enter(LinkedListItem* item)
     *
     * @description
     *  Registers the single thread.
     *---------------------------------------------------------------------------*/
    ErrorT enter(LinkedListItem* item);

    /*****************************************************************************
    * Method: enter(ListItem* item)
    *
    * @description
    *  Registers the single thread.
    *---------------------------------------------------------------------------*/
    ErrorT enter(ListItem* item) {
        wrappingItem.setData(item);
        return (this->enter(&wrappingItem));
    }

    /*****************************************************************************
    * Method: remove(ListItem* item)
    *
    * @description
    *  Removes the single thread if its list item is given.
    *---------------------------------------------------------------------------*/
    LinkedListItem* remove(ListItem* item);

    /*****************************************************************************
     * Method: startScheduling()
     *
     * @description
     *   Empty.
     *---------------------------------------------------------------------------*/
    void startScheduling() {
    }

    /*****************************************************************************
     * Method: getNextTimerEvent(LinkedList* sleepList, TimeT currentTime)
     *
     * @description
     *   Returns the next timer event for scheduling.
     *---------------------------------------------------------------------------*/
    TimeT getNextTimerEvent(LinkedList* sleepList, TimeT currentTime);

    /*****************************************************************************
     * Method: getNext()
     *
     * @description
     *   Returns the next Thread to be activated.
     *---------------------------------------------------------------------------*/
    LinkedListItem* getNext();
};

#endif /*SINGLETHREADSCHEDULER_H_*/

