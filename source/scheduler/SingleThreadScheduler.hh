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
	LinkedListItem wrappingItem;
public:
    SingleThreadScheduler();
    ~SingleThreadScheduler();

    inline void computePriority( Kernel_ThreadCfdCl* item ) {};

    /*!
     * Due this Scheduler is for singlethreading, only one thread could be set.
     */
    ErrorT enter( LinkedListItem* item );

    ErrorT enter( ListItem* item ) {
		wrappingItem.setData(item);
		return (this->enter(&wrappingItem));
    }

    LinkedListItem* remove(ListItem* item);

     /*!
     * Start the scheduling.
     */
    void startScheduling() {};

    TimeT getNextTimerEvent(LinkedList* sleepList, TimeT currentTime );

    LinkedListItem* getNext();

};

#endif /*SINGLETHREADSCHEDULER_H_*/

