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

// includes
#include "RealTimeThread.hh"
#include "kernel/Kernel.hh"
#include "inc/defines.h"
#include "assembler.h"
#include "assemblerFunctions.hh"

extern Kernel* theOS;
extern Board_ClockCfdCl* theClock;
extern Kernel_SchedulerCfdCl* theScheduler;

#define FAIL_ADDR          0xcb000000

RealTimeThread::RealTimeThread(void* p_startRoutinePointer,
                               void* p_exitRoutinePointer,
                               Task* p_owner,
                               unint4 stack_size,
                               void* RTThreadAttributes) :
                               PriorityThread(p_startRoutinePointer,
                                              p_exitRoutinePointer,
                                              p_owner,
                                              stack_size,
                                              RTThreadAttributes) {
    if (RTThreadAttributes != 0) {
        /* set the parameters and convert them from us to cylces */
        thread_attr_t* attr = static_cast<thread_attr_t*>(RTThreadAttributes);

        /* convert to clock ticks */
#if CLOCK_RATE >= (1 MHZ)
        this->relativeDeadline  = ((TimeT) attr->deadline)      * (CLOCK_RATE / 1000000);
        this->executionTime     = ((TimeT) attr->executionTime) * (CLOCK_RATE / 1000000);
        this->period            = ((TimeT) attr->period)        * (CLOCK_RATE / 1000000);
#else
        this->relativeDeadline  = (TimeT) ((attr->deadline * CLOCK_RATE) / 1000000U);
        this->executionTime     = (TimeT) ((attr->executionTime * CLOCK_RATE) / 1000000U);
        this->period            = (TimeT) ((attr->period * CLOCK_RATE) / 1000000U);
#endif

        LOG(PROCESS, INFO, "RealtimeThread:() period: %d", this->period);

        TimeT currentCycles =  theClock->getClockCycles();
        if (currentCycles < attr->arrivaltime) {
            /* set the absolute start time of this thread */
            this->arrivalTime       = attr->arrivaltime;
            /* put the thread to sleep on scheduler::enter until arrival time */
            this->sleepTime         = attr->arrivaltime;
        } else {
            this->arrivalTime       = currentCycles;
            this->sleepTime         = 0;
        }

        this->absoluteDeadline  = this->arrivalTime + this->relativeDeadline;
    } else {
        this->relativeDeadline  = 0;
        this->executionTime     = 0;
        this->period            = 0;
        this->sleepTime         = 0;
        this->absoluteDeadline  = 0;
    }
    this->instance  = 1;
    this->arguments = 0;

    theScheduler->computePriority(this);
}

RealTimeThread::~RealTimeThread() {
   /* nothing to do here */
}

/*****************************************************************************
 * Method: RealTimeThread::terminate()
 *
 * @description
 *
 *******************************************************************************/
void RealTimeThread::terminate() {
    /* are we periodic and are not about to be
     * terminated? */
    if (period > 0 && !this->status.areSet(cDoTermFlag)) {
        /* we need to disable interrupts since we will reset the thread context
           and really shouldn't be interrupted afterwards (since there is no way to resume). */
        _disableInterrupts();

        /* Reset the thread context. Fortunately it is this simple ^_^. */
        threadStack.top = 0;

        /* Set the status to new so that the call main method will be called instead of restoring the context. */
        this->status.set(cNewFlag | cReadyFlag);

#if USE_SAFE_KERNEL_STACKS
        // we use safe kernel stacks so we must free the stack slot now
        int2 myBucketIndex = this->getKernelStackBucketIndex();
        FREE_KERNEL_STACK_SLOT(myBucketIndex);
#endif


#ifdef DEBUG_EXECUTION_TIMES
        TimeT currentCycles = theClock->getTimeSinceStartup();
        /* check if we missed our deadline */
        if (currentCycles > this->absoluteDeadline) {
            /* TODO: add some additional handling/ user warning here? */
            LOG(PROCESS, FATAL, "RealTimeThread ID %d failed deadline! cur_cyles: %d, deadline: %d, sleep: %d", this->getId(), (unint4) currentCycles, (unint4) this->absoluteDeadline, (unint4) sleepcycles);
            /* omit serial console time for now .. this is not correct but good for debugging */
        }

        LOG(PROCESS, WARN, "exec_time: %d", (unint4) (currentCycles - arrivalTime) / (CLOCK_RATE / 1000000));
#endif

        /* set the new arrival time of this thread */
        this->arrivalTime       = this->arrivalTime + this->period;
        this->absoluteDeadline  = this->arrivalTime + this->relativeDeadline;

        /* invoke the computePriority method of the scheduler with its new deadline
         * EDF will recalculate the priority based on the absolute deadline.
         * RM will keep the same as before.
         * */
        theScheduler->computePriority(this);

        /* sleep until next arrivalTime */
        this->sleep(this->arrivalTime);
    } else {
        /* not periodic or soft terminate flag was set */
        if (this->relativeDeadline != 0) {
            int4 lateness = theClock->getClockCycles() - this->absoluteDeadline;
            if (lateness > 0) {
                LOG(PROCESS, WARN, "RealtimeThread::terminate() lateness: %d", lateness);
            }
        }

        /* terminate now */
        Thread::terminate();
    }
}

