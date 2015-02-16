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

#include "process/PriorityThread.hh"
#include "kernel/Kernel.hh"
#include "assembler.h"
#include "assemblerFunctions.hh"

extern Kernel* theOS;
extern unint4 rescheduleCount;

PriorityThread::PriorityThread(void* p_startRoutinePointer,
                               void* p_exitRoutinePointer,
                               Task* p_owner,
                               Kernel_MemoryManagerCfdCl* memManager,
                               unint4 stack_size,
                               void* prioThreadAttributes,
                               bool newThread) :
                        Thread(p_startRoutinePointer,
                               p_exitRoutinePointer,
                               p_owner,
                               memManager,
                               stack_size,
                               prioThreadAttributes,
                               newThread) {
    if (prioThreadAttributes != 0) {
        thread_attr_t* attr = static_cast<thread_attr_t*>(prioThreadAttributes);
        this->initialPriority = attr->priority;
        this->effectivePriority = attr->priority;
    } else {
        this->initialPriority = cDefaultPriority;
        this->effectivePriority = cDefaultPriority;
    }

    memset(&priorities, 0, sizeof(InheritedPriorities_t));
}

/*****************************************************************************
 * Method: PriorityThread::pushPriority(TimeT newPriority, void* m)
 *
 * @description
 * Tries to increase the priority of the thread to the new priority. Only succeeds
 * if the current effective priority is lower. Used for e.g. PIP.
 *---------------------------------------------------------------------------*/
void PriorityThread::pushPriority(TimeT newPriority, void* m) {
    if (m == 0) {
        LOG(PROCESS, ERROR, "PriorityThread::pushPriority() m==0");
        return;
    }

    SMP_SPINLOCK_GET(m_priolock);
    if (newPriority <= this->effectivePriority) {
        /* just return */
        SMP_SPINLOCK_FREE(m_priolock);
        return;
    }

    for (int i = 0; i < MAX_PIP_PRIORITIES; i++) {
        if (priorities.ref[i] == 0) {
            priorities.ref[i] = m;
            priorities.priorities[i] = this->effectivePriority;
            goto out;
        }
    }
    /* if we get here no more slots are left */
    LOG(PROCESS, ERROR, "PriorityThread::pushPriority() out of PIP priorities..");

out:
    this->effectivePriority = newPriority;
    this->getLinkedListItem()->remove();
    theOS->getCPUScheduler()->enter(this->getLinkedListItem());
    SMP_SPINLOCK_FREE(m_priolock);
}

/*****************************************************************************
 * Method: PriorityThread::popPriority(void* m)
 *
 * @description
 * Reduces the priority if the priority associated with m is currently the highest.
 *---------------------------------------------------------------------------*/
void PriorityThread::popPriority(void* m) {
    if (m == 0) {
        LOG(PROCESS, ERROR, "PriorityThread::popPriority() m==0");
        return;
    }
    SMP_SPINLOCK_GET(m_priolock);

    TimeT highestPrio = this->initialPriority;
    int found = 0;
    for (int i = 0; i < MAX_PIP_PRIORITIES; i++) {
        if (priorities.ref[i] == m) {
            priorities.ref[i] = 0;
            priorities.priorities[i] = 0;
            found = 1;
        } else {
            if (priorities.ref[i] != 0) {
              if (priorities.priorities[i] > highestPrio) {
                  highestPrio = priorities.priorities[i];
              }
          }
        }
    }

    SMP_SPINLOCK_FREE(m_priolock);

    if (!found) {
        /* if we get here no higher priority thread pushed out priority on m */
        LOG(PROCESS, DEBUG, "PriorityThread::pop() m=%x not found..", m);
        return;
    }

    /* chance for interrupts to reduce latency */
    NOP;
    {
        SMP_SPINLOCK_GET(m_priolock);
        if (highestPrio != 0) {
            /* update ourself at the scheduler! */
             this->effectivePriority = highestPrio;
             rescheduleCount++;
             /* program hardware timer to dispatch now.. may internally use a soft irq */
             theOS->getTimerDevice()->setTimer(1);
        }
        LOG(PROCESS, DEBUG, "PriorityThread::pop() Thread %d reverted Priority", this->getId());
        SMP_SPINLOCK_FREE(m_priolock);
    }
}

