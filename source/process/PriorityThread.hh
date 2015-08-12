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

#ifndef PRIORITYTHREAD_HH
#define PRIORITYTHREAD_HH
#define HAS_PRIORITY

#include "process/Thread.hh"
#include "inc/const.hh"

#define MAX_PIP_PRIORITIES 5

typedef struct {
    TimeT  priorities[MAX_PIP_PRIORITIES];
    void*  ref[MAX_PIP_PRIORITIES];
} InheritedPriorities_t;


/*!
 * \brief A thread class for multithreaded environments with priorities
 * \ingroup process
 *
 * This class adds priorities to threads which may be needed in some systems.
 * Unless the cpu scheduler is not configured to be some scheduler regarding priorities
 * this class is useless and should not be used to save space.
 */
class PriorityThread: public Thread {
    friend class Task;

    /* internal lock to get exclusive access on modification */
    unint4          m_priolock;

    InheritedPriorities_t priorities;
public:
    /*!
     *  The initial priority.
     */
    TimeT initialPriority;

    /*!
     *  The actual/effective priority.
     */
    TimeT effectivePriority;

    /*!
     *  The Constructor of the PriorityThread, the prioThreadAttributes pointer has to point to a prioThreadAttrs structure.
     */
    PriorityThread(void* startRoutinePointer,
                   void* exitRoutinePointer,
                   Task* owner,
                   unint4 stack_size = DEFAULT_USER_STACK_SIZE,
                   void* prioThreadAttributes = 0);

    /*****************************************************************************
      * Method: pushPriority(TimeT newPriority, void* m)
      *
      * @description
      * Tries to increase the priority of the thread to the new priority. Only succeeds
      * if the current effective priority is lower. Used for e.g. PIP.
      *---------------------------------------------------------------------------*/
    void pushPriority(TimeT newPriority, void* m);

    /*****************************************************************************
      * Method: popPriority(void* m)
      *
      * @description
      * Reduces the priority if the priority associated with m is currently the highest.
      *---------------------------------------------------------------------------*/
    void popPriority(void* m);
};

#endif /*PRIORITYTHREAD_HH*/

