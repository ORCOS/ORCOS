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
#include "inc/signals.hh"
#include "../process/KernelThread.hh"


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
    /*
     *  List containing the threads currently waiting for a generic (global) signal
     */
    LinkedList* waitList;

    /*
     * List of threads waiting for specific IRQ numbers
     */
    LinkedList* irqwaitList;

    /*
     * List of threads waiting for specific condition variables
     * (given as physical address on signal_wait/signal).
     */
    LinkedList* condwaitList;
#endif

    /*****************************************************************************
     * Method: signal(LinkedList* list, void* sig, int signalvalue = cOk)
     *
     * @description
     *  Wakes all threads on the given list waiting for the signal
     *
     * @returns
     *
     *---------------------------------------------------------------------------*/
    void signal(LinkedList* list, void* sig, int signalvalue = cOk);

public:
    SingleCPUDispatcher();


    ~SingleCPUDispatcher();

    /*****************************************************************************
     * Method: getSizeOfBlockedList()
     *
     * @description
     *  Returns the current number of threads on the blocked list.
     *
     * @returns
     *  unint   The current number of threads on the blocked list
     *---------------------------------------------------------------------------*/
    inline unint getSizeOfBlockedList() const {return (blockedList->getSize());}

    /*****************************************************************************
     * Method: getSizeOfSleepList()
     *
     * @description
     *  Returns the current number of threads on the sleep list.
     *
     * @returns
     *  unint   The current number of threads on the sleep list
     *---------------------------------------------------------------------------*/
    inline unint getSizeOfSleepList() const {return (sleepList->getSize());}

    /*****************************************************************************
     * Method: getSleeplist()
     *
     * @description
     *  Returns the sleep lists
     *
     * @returns
     *  unint   The sleep list
     *---------------------------------------------------------------------------*/
    inline LinkedList* getSleeplist() const {   return (this->sleepList);}

    /*****************************************************************************
     * Method: getBlockedlist()
     *
     * @description
     *  Returns the blocked lists
     *
     * @returns
     *  unint   The blocked list
     *---------------------------------------------------------------------------*/
    inline LinkedList* getBlockedlist() const {   return (this->blockedList);}

    /*****************************************************************************
     * Method: dispatch()
     *
     * @description
     *  Calling this function results in rescheduling the cpu. Thus the next
     *  thread is taken from the schdulers list and assigned to the cpu (started/resumed).
     *  This might be the currently running thread again depending on the schedulers
     *  policy.
     *
     *---------------------------------------------------------------------------*/
    void dispatch()  __attribute__((noreturn));

    /*****************************************************************************
     * Method: sleep(LinkedListItem* pSleepDbItem = pRunningThreadDbItem)
     *
     * @description
     *  Places a thread into the sleep list using its own linked list item.
     *
     * @params
     *  pSleepDbItem:     Pointer to the threads linked list item
     *
     *---------------------------------------------------------------------------*/
    void sleep(Thread* thread);

    /*****************************************************************************
     * Method: block(Thread* thread)
     *
     * @description
     *  Blocks the given thread, thereby placing it on the blocked list until
     *  it is removed by calling unblock.
     *
     * @params
     *  thread:     Pointer to the threads to be placed on the blocked list.
     *
     *---------------------------------------------------------------------------*/
    void block(Thread* thread);

    /*****************************************************************************
     * Method: unblock(Thread* thread)
     *
     * @description
     *  Removes the given thread from the blocked list inserting it into the
     *  sleepList if it has some remaining sleeptime or into the ready queue.
     *
     * @params
     *  thread:     The thread to be removed from the blocked list
     *---------------------------------------------------------------------------*/
    void unblock(Thread* thread);

#ifdef ORCOS_SUPPORT_SIGNALS
    /*****************************************************************************
     * Method: sigwait(Thread* thread)
     *
     * @description
     *  Places the given thread on the wait list.
     *
     * @params
     *  thread:     The thread that waits for a signal
     *---------------------------------------------------------------------------*/
    void sigwait(SignalType signaltype, Thread* thread);


    /*****************************************************************************
     * Method: signal(void* sig, int sigvalue)
     *
     * @description
     *  Indicates that the given generic (global) signal was raised.
     *  Wakes all threads currently waiting for that signal and reschedules them.
     *
     * @params
     *  sig:      The signal raised.
     *  sigvalue: The value of the signal passed as return code to the thread.
     *---------------------------------------------------------------------------*/
    void signal(void* sig, int signalvalue = cOk);

    /*****************************************************************************
      * Method: signalCond(void* phyAddr, int signalvalue = cOk)
      *
      * @description
      *  Indicates that the given condition variable (phy address) was raised.
      *  Wakes all threads currently waiting for that condition and reschedules them.
      *
      * @params
      *  sig:      The physical address of the condition raised.
      *  sigvalue: The value of the signal passed as return code to the thread.
      *---------------------------------------------------------------------------*/
    void signalCond(void* phyAddr, int signalvalue = cOk);
#endif

    /*****************************************************************************
     * Method: terminate_thread(Thread* thread)
     *
     * @description
     *  Tells the disptacher that the given thread has terminated. The thread is
     *  removed from all lists.
     *
     * @params
     *  thread:     The thread that terminated
     *---------------------------------------------------------------------------*/
    void terminate_thread(Thread* thread);
};

#endif /*SingleCPUDispatcher_H_*/
