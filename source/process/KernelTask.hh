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

#ifndef WORKERTASK_HH_
#define WORKERTASK_HH_

#include "KernelThread.hh"
#include "Task.hh"

/*!
 * \brief Worker task which holds a certain amount of workeör threads.
 * \ingroup process
 *
 * A WorkerTask is a kernel task that is used to schedule undeterministcally
 * arriving jobs. Therefore a workerthread is assigned to the job which is then scheduled
 * like any other realtime thread.
 */
class KernelTask: public Task {
    friend class KernelThread;

private:
    /*!
     * \brief The list of threads currently executing no job.
     */
    ArrayList nonWorkingThreads;


    /*****************************************************************************
     * Method: workFinished(WorkerThread* pwthread)
     *
     * @description
     *  Function to be called by workerthreads when they finish working.
     *
     * @params
     *  pwthread    The workerthread that finished its work
     *******************************************************************************/
    void    workFinished(KernelThread* pwthread);

public:
    KernelTask();

    ~KernelTask();


    /*****************************************************************************
     * Method: addJob(JOBType jobType, unint1 pid, void* param, unint priority_param)
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
   // KernelThread* addJob(JOBType id, unint1 pid, void* param, unint priority_param);


    KernelThread* getPeriodicThread(unint1 pid, CallableObject* obj, TimeT period, unint4 priority);

    KernelThread* getCallbackThread(unint1 pid, CallableObject* obj, TimeT delay, unint4 priority);

    KernelThread* getIRQThread(unint1 pid, GenericDeviceDriver* driver, int irq, unint4 priority);

};

#endif /*WORKERTASK_HH_*/
