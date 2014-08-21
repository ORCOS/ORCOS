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

#ifndef SINGLECPUDISPATCHER_H_
#define SINGLECPUDISPATCHER_H_

#include "process/IdleThread.hh"
#include "db/LinkedList.hh"
#include "process/Task.hh"
#include Kernel_Scheduler_hh
#include "process/WorkerThread.hh"

/*!
 * \brief Class managing the execution of threads on a single cpu.
 *
 *
 * This class is a management object which is responsible of starting/stopping
 * threads on a single cpu. For this a ThreadScheduler is needed which decides
 * which thread is the next thread to run on the cpu by its implemented scheduling
 * policy in the Scheduler::getNext() method.
 *
 * The CPUManager restores also threads that have a context (thus have at least run one time)
 * by calling the restoreContext(Thread*) method which needs to be implemented for the architecture
 * the operating systems is supposed to run on.
 */
class SingleCPUDispatcher {

    //! scheduler maintaining the ready list.
DEF_Kernel_SchedulerCfd

private    :
    //! list containing the currently blocked threads
    LinkedList* blockedList;

    /*
     * List containing the currently sleeping threads.
     * The ORCOS tasks waiting for their next period to begin
     * are treated as sleeping threads. This list
     * is parsed by the scheduler to determine the next
     * preemption point -> the arrival time of a higher priority
     * thread.
     */
    LinkedList* sleepList;

#ifdef ORCOS_SUPPORT_SIGNALS
    //! List containing the threads currently waiting for a signal
    LinkedList* waitList;
#endif


    IdleThread* idleThread;

public:
    SingleCPUDispatcher();


    ~SingleCPUDispatcher();

    //! Returns the current number of elements in the dispatchers blockedList
    unint getSizeOfBlockedList() {return (blockedList->getSize());}

    //! Returns the current number of elements in the dispatchers sleepList
    unint getSizeOfSleepList() {return (sleepList->getSize());}


    inline LinkedList* getSleeplist() {   return (this->sleepList);}

    /*!
     * \brief The dispatch function triggered by the TimerDevice on timer interrupt
     *
     * Calling this functison results in rescheduling the cpu. Thus the next
     * thread is taken from the cpu and assigned to the cpu (started/resumed).
     * This might be the currently running thread again depending on the schedulers
     * policy.
     *
     *
     */
    void dispatch();

    /*!
     * \brief sleep method which sends the given / current thread to sleep mode
     */
    void sleep(LinkedListItem* pSleepDbItem = pRunningThreadDbItem);

    /*!
     * \brief block method which sends the current thread to blocked mode
     */
    void block(Thread* thread);

    /*!
     *  \brief Unblocks the given thread
     */
    void unblock(Thread* thread);

#ifdef ORCOS_SUPPORT_SIGNALS
    /*!
     *  \brief Indicates to the dispatch that the given thread is waiting on a signal
     *
     *  The signal the thread is waiting on is given inside thread->signal
     *  This signal itself can be a memory location, a define SIG_xx or anything else.
     */
    void sigwait(Thread* thread);


    /*!
     * \brief Resumes all threads waiting for the given signal. The corresponding signal_value
     * will be set as return value.
     */
    void signal(void* sig, int signalvalue = cOk);
#endif

    /*!
     * \brief Terminates the given thread
     */
    void terminate_thread(Thread* thread);
};

#endif /*SingleCPUDispatcher_H_*/
