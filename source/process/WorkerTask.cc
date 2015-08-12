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

#include "WorkerTask.hh"
#include "kernel/Kernel.hh"

extern Board_ClockCfdCl* theClock;
extern Kernel* theOS;
extern Kernel_SchedulerCfdCl* theScheduler;

WorkerTask::WorkerTask() :
        Task(),
        nonWorkingThreads(NUM_WORKERTHREADS) {
    LOG(KERNEL, INFO, "WorkerTask(): Creating %d Workerthreads", NUM_WORKERTHREADS);
    /* create some worker threads */
    for (int i = 0; i < NUM_WORKERTHREADS; i++) {
        WorkerThread* pWThread = new WorkerThread(this);
        nonWorkingThreads.addHead(pWThread);
        /* announce the Workerthread to the dispatcher by blocking it */
        pWThread->block();
    }
}

WorkerTask::~WorkerTask() {
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
void WorkerTask::workFinished(WorkerThread* pwthread) {
    nonWorkingThreads.addHead(pwthread);
}

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
WorkerThread* WorkerTask::addJob(JOBType jobType, unint1 pid, void* param, unint priority_param) {
    LOG(PROCESS, DEBUG, "WorkerTask::addJob(): job %d", jobType);

    /* find a available workerthread and assign the job */
    ListItem* litem = this->nonWorkingThreads.removeHead();

    if (litem != 0) {
        WorkerThread* pWThread = static_cast<WorkerThread*>(litem);
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
