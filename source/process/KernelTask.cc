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

#include "KernelTask.hh"

#include "kernel/Kernel.hh"

extern Board_ClockCfdCl* theClock;
extern Kernel* theOS;
extern Kernel_SchedulerCfdCl* theScheduler;

KernelTask::KernelTask() :
        Task(),
        nonWorkingThreads(NUM_WORKERTHREADS) {
    LOG(KERNEL, INFO, "WorkerTask(): Creating %u Kernel Threads", NUM_WORKERTHREADS);
    /* create some worker threads */
    for (int i = 0; i < NUM_WORKERTHREADS; i++) {
        KernelThread* pWThread = new KernelThread(this);
        nonWorkingThreads.addHead(pWThread);
        /* announce the KernelThread to the dispatcher by blocking it */
        pWThread->block();
    }
}

KernelTask::~KernelTask() {
}


/*****************************************************************************
 * Method: WorkerTask::workFinished(WorkerThread* pwthread)
 *
 * @description
 *  Function to be called by workerthreads when they finish working.
 *
 * @params
 *  pwthread    The workerthread that finished its work
 *******************************************************************************/
void KernelTask::workFinished(KernelThread* pwthread) {
    nonWorkingThreads.addHead(pwthread);
}


KernelThread* KernelTask::getPeriodicThread(unint1 pid, CallableObject* obj, TimeT period, unint4 priority) {
    /* find a available workerthread and assign the job */
     ListItem* litem = this->nonWorkingThreads.removeHead();

     if (litem != 0) {
         KernelThread* pWThread = static_cast<KernelThread*>(litem);
         pWThread->jobid = PeriodicFunctionCallJob;
         pWThread->pid   = pid;
         pWThread->param.periodicCall.period = period;
         pWThread->param.periodicCall.functioncall.objectptr = obj;
         pWThread->param.periodicCall.functioncall.time = 0;

         // set the arrival time to now!
         pWThread->arrivalTime = 0;
         pWThread->instance    = 1;

#ifdef HAS_PRIORITY
         pWThread->initialPriority   = priority;
         pWThread->effectivePriority = priority;

 #ifdef REALTIME
         pWThread->relativeDeadline  = 0;
         /* if no priority was given calculate the priority */
         if (priority == 0) {
             pWThread->period            = pWThread->param.periodicCall.period;
             theScheduler->computePriority(pWThread);
             pWThread->period = 0;
         }
 #endif
#endif

          pWThread->sleepTime = 0;
         /* unblock the workerthread
          * this will cause the thread
          * either to be scheduled directly
          * or send to sleep if the sleeptime is > 0 */
         pWThread->unblock();
         return (pWThread);
     }
     return (0);
}

KernelThread* KernelTask::getCallbackThread(unint1 pid, CallableObject* obj, TimeT delay, unint4 priority) {
    /* find a available workerthread and assign the job */
    ListItem* litem = this->nonWorkingThreads.removeHead();

    if (litem != 0) {
        KernelThread* pWThread = static_cast<KernelThread*>(litem);
        TimeT now = theOS->getClock()->getClockCycles();

        pWThread->jobid = TimedFunctionCallJob;
        pWThread->pid   = pid;
        pWThread->param.timedCall.objectptr = obj;
        pWThread->param.timedCall.time      = now + delay;
        pWThread->param.timedCall.parameterptr = 0;

        pWThread->sleepTime = pWThread->param.timedCall.time;

#ifdef HAS_PRIORITY
        pWThread->initialPriority   = priority;
        pWThread->effectivePriority = priority;
#endif
        pWThread->arrivalTime  = now;
        pWThread->period       = 0;
        pWThread->instance     = 1;

       /* unblock the workerthread
        * this will cause the thread
        * either to be scheduled directly
        * or send to sleep if the sleeptime is > 0 */
       pWThread->unblock();
       return (pWThread);
    }
    return (0);
}

KernelThread* KernelTask::getIRQThread(unint1 pid, GenericDeviceDriver* driver, int irq, unint4 priority) {
    /* find a available workerthread and assign the job */
    ListItem* litem = this->nonWorkingThreads.removeHead();

    if (litem != 0) {
        KernelThread* pWThread = static_cast<KernelThread*>(litem);

        pWThread->jobid = IRQJob;
        pWThread->pid   = pid;
        pWThread->arrivalTime = 0;
        pWThread->irq = irq;
#ifdef HAS_PRIORITY
        pWThread->initialPriority   = priority;
        pWThread->effectivePriority = priority;
#endif
        pWThread->param.driver = driver;
        pWThread->sleepTime    = 0;
        pWThread->period       = 0;
        pWThread->instance     = 1;

        /* unblock the workerthread
         * this will cause the thread
         * either to be scheduled directly
         * or send to sleep if the sleeptime is > 0 */
         pWThread->unblock();
         return (pWThread);
    }
    return (0);
}


#if 0
/*****************************************************************************
 * Method: WorkerTask::addJob(JOBType jobType, unint1 pid, void* param, unint priority_param)
 *
 * @description
 *  Adds a new job to the workertash which is executed by a dedicated worker thread.
 *
 * @params
 *  jobType    The type of job
 *  pid        The address space id the thread shall be executed in. Running
 *             in the address space of another task allows the workerthread
 *             to access the data of that task
 *  param      Parameter of the job
 * @returns
 *  WorkerThread*  Pointer to the workerthread assigned to the job or null if no
 *                 workerthread could be assigned.
 *******************************************************************************/
KernelThread* KernelTask::addJob(JOBType jobType, unint1 pid, void* param, unint priority_param) {
    LOG(PROCESS, DEBUG, "WorkerTask::addJob(): job %d", jobType);

    /* find a available workerthread and assign the job */
    ListItem* litem = this->nonWorkingThreads.removeHead();

    if (litem != 0) {
        KernelThread* pWThread = static_cast<KernelThread*>(litem);
        pWThread->setJob(jobType, param);
        pWThread->setPID(pid);

        LOG(PROCESS, DEBUG, "WorkerTask::addJob() assigned thread %d for job %d", pWThread->getId(), jobType);

#ifdef HAS_PRIORITY
    #ifndef REALTIME
            pWThread->setInitialPriority(priority_param);
            pWThread->setEffectivePriority(priority_param);
    #else
            // reset the instance to 1
            pWThread->instance          = 1;
            pWThread->initialPriority   = priority_param;
            pWThread->period            = 0;
            pWThread->relativeDeadline  = 0;

            if (jobType == PeriodicFunctionCallJob) {
                PeriodicFunctionCall* pcall = reinterpret_cast<PeriodicFunctionCall*>(param);
            #if (CLOCK_RATE >= (1 MHZ))
                    pWThread->relativeDeadline = pcall->period * (CLOCK_RATE / 1000000);
                    pWThread->period           = pcall->period * (CLOCK_RATE / 1000000);
            #else
                    pWThread->relativeDeadline = (TimeT) ((pcall->period * CLOCK_RATE) / 1000000U);
                    pWThread->period           = (TimeT) ((pcall->period * CLOCK_RATE) / 1000000U);
            #endif
            }

            // set the arrival time to now!
            pWThread->arrivalTime = 0;
    #endif

        theScheduler->computePriority(pWThread);
        pWThread->period = 0;
#endif

        if (jobType == TimedFunctionCallJob || jobType == PeriodicFunctionCallJob) {
            /* set the sleeptime so the thread sleeps
             * until the the function can be called */
            TimedFunctionCall* funcCall = reinterpret_cast<TimedFunctionCall*>(param);
            /* when are we going to be called the first time? sleep until that point in time */
            pWThread->sleepTime = funcCall->time;
        } else {
            pWThread->sleepTime = 0;
        }

        /* unblock the workerthread
         * this will cause the thread
         * either to be scheduled directly
         * or send to sleep if the sleeptime is > 0 */
        pWThread->unblock();
        return (pWThread);
    } else {
        return (0);
    }
}
#endif
