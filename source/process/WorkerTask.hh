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

#include "Task.hh"
#include "process/WorkerThread.hh"

/*!
 * \brief Worker task which holds a certain amount of workeör threads.
 * \ingroup process
 *
 * A WorkerTask is a kernel task that is used to schedule undeterministcally
 * arriving jobs. Therefore a workerthread is assigned to the job which is then scheduled
 * like any other realtime thread.
 */
class WorkerTask: public Task {

    friend class WorkerThread;

private:
    /*!
     * \brief The list of threads currently executing no job.
     */
    ArrayList nonWorkingThreads;


    /*!
     * \brief Callback for the workerthread to indicate work has finished.
     */
    void    workFinished(WorkerThread* pwthread);

public:

    WorkerTask();

    ~WorkerTask();

    /*!
     * \brief Adds another job to the worker task
     *
     * This will activate one of the available worker threads to take over this job
     *
     * \param   id      the jobid of the job added
     *          pid     the pid the workerthread shall work with
     *          param   parameters passed to the job
     *          priority_param  the priority/deadline of the workerthread in a priority/realtime based system
     */
    WorkerThread* addJob(JOBType id, unint1 pid, void* param, unint priority_param);


};

#endif /*WORKERTASK_HH_*/
