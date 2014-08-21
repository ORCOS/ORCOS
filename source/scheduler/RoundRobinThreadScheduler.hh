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
#define RRTimeSlice 200 ms

/*!
 * \ingroup scheduler
 * \brief Thread scheduler which implements the RoundRobin policy.
 *
 */
class RoundRobinThreadScheduler: public ListScheduler {
public:
    RoundRobinThreadScheduler();

    ~RoundRobinThreadScheduler();

    inline void computePriority( Kernel_ThreadCfdCl* item ) {};
    /*!
     * \brief The scheduling policy.
     *
     * This method returns the the next thread to be executed. The old thread
     * will be sorted back into the lists so it may be rescheduled later on.
     */
    ListItem* getNext();

    /*!
     * \brief The initialisation method called just before scheduling begins
     *
     * This method sets the timer device timeslice.
     */
    void startScheduling();

    /*!
     * \brief Method returning the amount of microseconds for next timer event.
     *
     * This may be 0 if no elements are in the list.
     */
    TimeT getNextTimerEvent( LinkedList* sleepList,TimeT currentTime  );
};

#endif /*ROUNDROBINTHREADSCHEDULER_HH_*/
