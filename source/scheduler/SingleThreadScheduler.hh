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
#include ThreadCfd_hh

/*!
 * \ingroup scheduler
 * \brief Scheduler only schedules one single thread. Special Class.
 *
 */
class SingleThreadScheduler {
    ThreadCfdCl* singleThread;
public:
    SingleThreadScheduler();
    ~SingleThreadScheduler();

    inline void computePriority( ThreadCfdCl* item ) {};

    /*!
     * Due this Scheduler is for singlethreading, only one thread could be set.
     */
    ErrorT enter( ScheduleableItem* item );

     /*!
     * Start the scheduling.
     */
    void startScheduling() {};

    int getNextTimerEvent(LinkedListDatabase* sleepList,unint4 dt );

    DatabaseItem* getNext();

};

#endif /*SINGLETHREADSCHEDULER_H_*/

