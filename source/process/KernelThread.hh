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

#ifndef WORKERTHREAD_HH_
#define WORKERTHREAD_HH_

#include "SCLConfig.hh"
#include "db/ArrayList.hh"
#include Kernel_Thread_hh

typedef enum {
    None, IRQJob, TimedFunctionCallJob, PeriodicFunctionCallJob, IdleJob
} JOBType;

/*!
 * \brief Worker Thread (belonging to WorkerTask), execute jobs for other threads
 * \ingroup process
 *
 * The worker thread class which takes jobs to complete for other threads. This may e.g be threads waiting for a packet reception on one of the
 * communication devices. This from the priority thread derived class makes it possible to schedule those activities like any other thread
 * and thus making it possible to work in realtime environments.
 *
 *
 * jobids: <BR>
 *
 * ExternalDeviceJob          : packet reception on comm_device <BR>
 * TimedFunctionCallJob       : call a function after a period of time <BR>
 * PeriodicFunctionCallJob    : call a function periodically <BR>
 *
 */
class WorkerThread: public Kernel_ThreadCfdCl {
private:
    //! The job table of the worker thread. Layout : jobid | parameters ...
    // ArrayDatabase* job;
    void* param;

    // the jobid
    unint1 jobid;

    /*!
     * \brief the pid the worker thread is currently running with
     * this pid can change during the execution of the worker thread
     */
    unint1 pid;

public:
    /*****************************************************************************
     * Method: WorkerThread(Task* owner)
     *
     * @description
     *  Constructor
     *******************************************************************************/
    explicit WorkerThread(Task* owner);


    ~WorkerThread();

    /*****************************************************************************
     * Method: callMain()
     *
     * @description
     *  overloaded callMain method which starts this thread
     *******************************************************************************/
    void callMain();

    /*****************************************************************************
     * Method: work()
     *
     * @description
     *  the working routine
     *******************************************************************************/
    void work() __attribute__((used));

    /*****************************************************************************
     * Method: setJob(JOBType id, void* params)
     *
     * @description
     *  Set the job of this workerthread
     *******************************************************************************/
    inline void setJob(JOBType id, void* params) {
        jobid = id;
        param = params;
    }


    /*****************************************************************************
     * Method: setPID(unint1 thread_pid)
     *
     * @description
     *   Set the PID the workerthread shall work with
     *******************************************************************************/
    inline void setPID(unint1 thread_pid) {
        this->pid = thread_pid;
    }

    /*****************************************************************************
     * Method: stop()
     *
     * @description
     *  Stops the execution of this workerthread. Removes its current job
     *  and makes the thread available for new jobs again.
     *******************************************************************************/
    void stop();

    /*****************************************************************************
     * Method: hasJob()
     *
     * @description
     *
     *******************************************************************************/
    inline bool hasJob() {
        return (jobid != None);
    }

    /*****************************************************************************
     * Method: getPID()
     *
     * @description
     *   Get the PID the workerthread is currently working with
     *******************************************************************************/
    inline unint1 getPID() {
        return (pid);
    }
};

#endif /*WORKERTHREAD_HH_*/
