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

#ifndef ROUNDROBINTHREADSCHEDULER_HH_
#define ROUNDROBINTHREADSCHEDULER_HH_

#include "scheduler/ListScheduler.hh"
#include "hal/CallableObject.hh"

// The RR timeslice
#define RRTimeSlice 20 ms

/*!
 * \ingroup scheduler
 * \brief Thread scheduler which implements the RoundRobin policy.
 *
 */
class RoundRobinThreadScheduler: public ListScheduler {
public:
    RoundRobinThreadScheduler();

    ~RoundRobinThreadScheduler();

    /*****************************************************************************
     * Method: computePriority(Kernel_ThreadCfdCl* item)
     *
     * @description
     *  Does nothing as round robin does ignor priorities.
     *******************************************************************************/
    inline void computePriority(Kernel_ThreadCfdCl* item) {}

    /*****************************************************************************
     * Method: getNext()
     *
     * @description
     *  Returns the next item by round robin fashion.
     *
     * @returns
     *  ListItem         The next item
     *******************************************************************************/
    LinkedListItem* getNext();

    /*****************************************************************************
     * Method: startScheduling()
     *
     * @description
     *  Does nothing as round robin does not need precomputation.
     *******************************************************************************/
    inline void startScheduling() {}

    /****************************************************************************
     * Method: getNextTimerEvent( LinkedList* sleepList, TimeT currentTime )
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
    TimeT getNextTimerEvent(LinkedList* sleepList, TimeT currentTime);
};

#endif /*ROUNDROBINTHREADSCHEDULER_HH_*/
