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

#include "SingleCPUDispatcher.hh"

#include <assemblerFunctions.hh>
#include "kernel/Kernel.hh"
#include "sprintf.hh"

extern Kernel*                 theOS;
extern Board_TimerCfdCl*       theTimer;
extern Board_ClockCfdCl*       theClock;

/* Global Variables to speed up access to these objects */
Kernel_SchedulerCfdCl*  theScheduler;
Kernel_ThreadCfdCl*     pCurrentRunningThread   = 0;
Task*                   pCurrentRunningTask     = 0;
TimeT                   lastCycleStamp          = 0;
bool                    processChanged          = true;
unint4                  scheduleCount           = 0;
unint4                  rescheduleCount         = 0;
bool                    needReschedule          = false;

#ifndef ARCH_DELAY
#define ARCH_DELAY 3 MICROSECONDS
#endif


SingleCPUDispatcher::SingleCPUDispatcher() :
        blockedList(new LinkedList),
        sleepList(new LinkedList)
#ifdef ORCOS_SUPPORT_SIGNALS
        ,
        waitList(new LinkedList),
        irqwaitList(new LinkedList),
        condwaitList(new LinkedList)
#endif
{
    /* Initialize the scheduler */
    SchedulerCfd            = new NEW_Kernel_SchedulerCfd;
    theScheduler            = SchedulerCfd;
    pCurrentRunningThread   = 0;
    pCurrentRunningTask     = 0;
    scheduleCount           = 0;
    /* Set idle thread */
    idleThread              = new IdleThread();
}

SingleCPUDispatcher::~SingleCPUDispatcher() {
}


static inline unint4 rdtsc(void) {
    unint4 r = 0;
    asm volatile("mrc p15, 0, %0, c9, c13, 0; ISB;" : "=r"(r) );
    return r;
}

//static int4 minCycles = 99999999;
//static int4 maxCycles = 0;
//static int numBenchmarks = 0;

/*****************************************************************************
 * Method: SingleCPUDispatcher::dispatch()
 *
 * @description
 *  The main dispatcher method. Gets the next thread to be run
 *  from the configured scheduler and runs it.
 *
 *---------------------------------------------------------------------------*/
void SingleCPUDispatcher::dispatch() {
    /* first be sure that interrupts are disabled */
    _disableInterrupts();
    needReschedule = false;
    /* the time at dispatch point */
    TimeT currentTime = theClock->getClockCycles() + ARCH_DELAY;
    /* set the time stamp */
    lastCycleStamp = currentTime;
    scheduleCount++;
    LOG(SCHEDULER, DEBUG, "SingleCPUDispatcher: Dispatching!");


    /* check whether the idle thread was currently running or not
     the idle thread would have pCurrentRunningThread == 0 */
    if (pCurrentRunningThread != 0) {
        LinkedListItem* lItem = pCurrentRunningThread->getLinkedListItem();
        LOG(SCHEDULER, DEBUG, "Thread %d preempted.. Reinserting", pCurrentRunningThread->getId());
        pCurrentRunningThread = 0;
        this->SchedulerCfd->enter(lItem);
    }

    /* get the next timer event the scheduler wants to be called */
    TimeT nextevent = this->SchedulerCfd->getNextTimerEvent(sleepList, currentTime);
    LOG(SCHEDULER, DEBUG, "SingleCPUDispatcher: next Timer %x %x", (unint4) ((nextevent >> 32) & 0xffffffff), (unint4) ((nextevent) & 0xffffffff));
    theTimer->setTimer(nextevent);

    /*
     * Get the next ready thread to be executed.
     * Be sure to call getNextTimerEvent() before this, since depending on the scheduler the returned
     * timeslice might depend on the Threads in the scheduler! (e.g. RateMonotonicThreadScheduler) */
    LinkedListItem* nextThreadDbItem = static_cast<LinkedListItem*>(SchedulerCfd->getNext());

    /* Any ready thread?  */
    if (likely(nextThreadDbItem != 0)) {
        pCurrentRunningThread   = static_cast<Kernel_ThreadCfdCl*>(nextThreadDbItem->getData());
        pCurrentRunningTask     = pCurrentRunningThread->getOwner();

        ASSERT(pCurrentRunningThread);
        ASSERT(pCurrentRunningTask);

        int tid = pCurrentRunningThread->getId();

        TRACE_THREAD_START(tid, pCurrentRunningThread->getId());

        pCurrentRunningThread->instance++;

        /* Is this thread new (has no context)?*/
        if (unlikely(pCurrentRunningThread->isNew())) {
            LOG(SCHEDULER, DEBUG, "SingleCPUDispatcher: start Thread %d at %x, stack [0x%x - 0x%x]", tid, pCurrentRunningThread, pCurrentRunningThread->threadStack.startAddr, pCurrentRunningThread->threadStack.endAddr);
            pCurrentRunningThread->callMain();
        } else {
            LOG(SCHEDULER, DEBUG, "SingleCPUDispatcher: resume Thread %d at %x", tid, pCurrentRunningThread);
            assembler::restoreContext(pCurrentRunningThread);
        }
    } else {
        pCurrentRunningThread   = 0;
        pCurrentRunningTask     = 0;

        LOG(KERNEL, DEBUG, "SingleCPUDispatcher: Idle Thread running");

#ifdef DEBUG_IDLE
        if (nextevent == MAX_UINT8) {
            LOG(KERNEL, WARN, "SingleCPUDispatcher: Idle without preemption programmed...");
            LOG(KERNEL, WARN, "SingleCPUDispatcher: SleepList size     : %u", sleepList->getSize());
            LOG(KERNEL, WARN, "SingleCPUDispatcher: BlockedList size   : %u", blockedList->getSize());
            LOG(KERNEL, WARN, "SingleCPUDispatcher: WaitList size      : %u", waitList->getSize());
            LOG(KERNEL, WARN, "SingleCPUDispatcher: IRQWaitList size   : %u", irqwaitList->getSize());
            LOG(KERNEL, WARN, "SingleCPUDispatcher: CondwaitList size  : %u", condwaitList->getSize());
            this->SchedulerCfd->printQueue();
            LinkedList* ltasks = theOS->getTaskManager()->getTaskDatabase();
            for (LinkedListItem* lldi = ltasks->getHead(); lldi != 0; lldi = lldi->getSucc()) {
                  Task* task = static_cast<Task*>(lldi->getData());
                  LOG(KERNEL, WARN, "SingleCPUDispatcher: Task : %u", task->getId());
                  LinkedList* lthreads = task->getThreadDB();
                  for (LinkedListItem* lldthread = lthreads->getHead(); lldthread != 0; lldthread = lldthread->getSucc()) {
                      Thread* thread = (Thread*) lldthread->getData();
                      LOG(KERNEL, WARN, "SingleCPUDispatcher:   Thread %u (%s): State %x", thread->getId(), thread->getName(), thread->getStatus().getBits());
                  }
              }
        }
#endif
        /* non returning run() */
        idleThread->run();
    }
    __builtin_unreachable();
}

/*****************************************************************************
 * Method: SingleCPUDispatcher::sleep(LinkedListItem* pSleepDbItem)
 *
 * @description
 *  Places a thread into the sleep list using its own linked list item.
 *
 * @params
 *  pSleepDbItem:     Pointer to the threads linked list item
 *
 *---------------------------------------------------------------------------*/
void SingleCPUDispatcher::sleep(Thread* thread) {
    /* be sure that this critical area cant be interrupted */
    int irqstatus;
    DISABLE_IRQS(irqstatus);

    /* place into sleeplist at correct location. sorted on sleeptime. */
    LinkedListItem* sItem = sleepList->getTail();
    TimeT sleepTime = thread->getSleepTime();
    while (sItem != 0) {
         if (static_cast<PriorityThread*>(sItem->getData())->getSleepTime() <= sleepTime) {
             sleepList->insertAfter(thread->getLinkedListItem(), sItem);
             goto out;
         }
         sItem = sItem->getPred();
    }

    /* if this statement is reached, no thread with a bigger priority then the pRTThread was found,
     so we add it at the very front of the queue. */
    sleepList->addHead(thread->getLinkedListItem());

out:
    if (thread == pCurrentRunningThread) {
        pCurrentRunningThread = 0;
        dispatch();
        __builtin_unreachable();
    }

    RESTORE_IRQS(irqstatus);
}

/*****************************************************************************
 * Method: SingleCPUDispatcher::block(Thread* thread)
 *
 * @description
 *  Blocks the given thread, thereby placing it on the blocked list until
 *  it is removed by calling unblock.
 *
 * @params
 *  thread:     Pointer to the threads to be placed on the blocked list.
 *
 *---------------------------------------------------------------------------*/
void SingleCPUDispatcher::block(Thread* thread) {
    /* be sure that this critical area can not be interrupted */
    int irqstatus;
    DISABLE_IRQS(irqstatus);
    LOG(SCHEDULER, DEBUG, "SingleCPUDispatcher::block() blocking Thead %d", thread->getId());

    TRACE_THREAD_STOP(thread->getOwner()->getId(), thread->getId());

    this->blockedList->addTail(thread->getLinkedListItem());

    if (thread == pCurrentRunningThread) {
        pCurrentRunningThread = 0;
        LOG(SCHEDULER, INFO, "SingleCPUDispatcher::block() dispatching! %d", this->SchedulerCfd->database.getSize());
        dispatch();
        __builtin_unreachable();
    }

    RESTORE_IRQS(irqstatus);
}

/*****************************************************************************
 * Method: SingleCPUDispatcher::unblock(Thread* thread)
 *
 * @description
 *  Removes the given thread from the blocked list inserting it into the
 *  sleepList if it has some remaining sleeptime or into the ready queue.
 *
 * @params
 *  thread:     The thread to be removed from the blocked list
 *---------------------------------------------------------------------------*/
void SingleCPUDispatcher::unblock(Thread* thread) {
    int irqstatus;
    DISABLE_IRQS(irqstatus);

    LOG(SCHEDULER, DEBUG, "SingleCPUDispatcher::unblock() unblocking Thead %d", thread->getId());
    thread->getLinkedListItem()->remove();

    /* check if there is still some remaining sleep time on this thread */
    if (thread->sleepTime > 0) {
        this->sleepList->addTail(thread->getLinkedListItem());
    } else {
        this->SchedulerCfd->enter(thread->getLinkedListItem());

        Kernel_ThreadCfdCl *pThread = static_cast<Kernel_ThreadCfdCl *>(thread);
        /*  Ensure Dispatch if no thread is running or the unblocked threads has
         * a higher priority!  */
        if (pCurrentRunningThread == 0 || pThread->effectivePriority > pCurrentRunningThread->effectivePriority) {
            /* issue rescheduling interrupt */
            needReschedule = true;
            rescheduleCount++;
            theOS->getTimerDevice()->setTimer(1);
        }
    }

    RESTORE_IRQS(irqstatus);
}

#ifdef ORCOS_SUPPORT_SIGNALS

/*****************************************************************************
 * Method: SingleCPUDispatcher::sigwait(Thread* thread)
 *
 * @description
 *  Places the given thread on the wait list.
 *
 * @params
 *  thread:     The thread that waits for a signal
 *---------------------------------------------------------------------------*/
void SingleCPUDispatcher::sigwait(SignalType signaltype, Thread* thread) {
    int irqstatus;
    DISABLE_IRQS(irqstatus);

    LOG(SCHEDULER, TRACE, "SingleCPUDispatcher::sigwait() adding Thread");

    switch (signaltype) {
    case (SIGNAL_GENERIC) : {
       waitList->addTail(thread->getLinkedListItem());
       break;
       }
    case (SIGNAL_COND) : {
       condwaitList->addTail(thread->getLinkedListItem());
       break;
       }
    }

    if (thread == pCurrentRunningThread) {
        pCurrentRunningThread = 0;
        dispatch();
        __builtin_unreachable();
    }

    RESTORE_IRQS(irqstatus);
}

/*****************************************************************************
 * Method: SingleCPUDispatcher::signal(LinkedList* list, void* sig, int signalvalue = cOk)
 *
 * @description
 *  Indicates that the given signal was raised. Wakes all threads currently
 *  waiting for that signal and reschedules them.
 *
 * @params
 *  sig:      The signal raised.
 *  sigvalue: The value of the signal passed as return code to the thread.
 *---------------------------------------------------------------------------*/
void SingleCPUDispatcher::signal(LinkedList* list, void* sig, int sigvalue) {
    /* be sure that this critical area cannot be interrupted */
    int irqstatus;
    DISABLE_IRQS(irqstatus);
    LOG(SCHEDULER, TRACE, "SingleCPUDispatcher::signal() signal: %d", sig);

    LinkedListItem* litem = list->getHead();

    while (litem != 0) {
        LinkedListItem* tmpLitem = litem->getSucc();
        Thread* pThread = static_cast<Thread*>(litem->getData());

        if (pThread->signal == sig) {
            LOG(SCHEDULER, TRACE, "SingleCPUDispatcher::signal() signal to thread %d", pThread->getId());

            pThread->status.clearBits(cSignalFlag);
            /* Set the signal return value. The restore Context must handle passing this value
             * to the return register. It should reset the signal to 0 to indicate that the thread is not
             * waiting for a signal any more and ignore the signalvalue field afterwards.
             *  */
            pThread->signalvalue = sigvalue;

            /* stopped threads are ignored */
            if (!pThread->status.areSet(cStopped)) {
                if (pThread->isBlocked()) {
                    /* woke up blocked thread.. put on blocked list*/
                    this->blockedList->addTail(litem);
                } else if (pThread->sleepTime > 0) {
                    /* some sleep time left.. put on sleeplist*/
                    this->sleepList->addTail(litem);
                } else {
                    /* put on run queue */
                    this->SchedulerCfd->enter(litem);
                }
            }
        } /* if pThread->signal == sig*/

        litem = tmpLitem;
    }

    RESTORE_IRQS(irqstatus);
}


/*****************************************************************************
  * Method: SingleCPUDispatcher::signalCond(void* phyAddr, int signalvalue = cOk)
  *
  * @description
  *  Indicates that the given condition variable (phy address) was raised.
  *  Wakes all threads currently waiting for that condition and reschedules them.
  *
  * @params
  *  sig:      The physical address of the condition raised.
  *  sigvalue: The value of the signal passed as return code to the thread.
  *---------------------------------------------------------------------------*/
void SingleCPUDispatcher::signalCond(void* phyAddr, int signalvalue) {
    this->signal(condwaitList, phyAddr, signalvalue);
}

/*****************************************************************************
  * Method: SingleCPUDispatcher::signal(void* sig, int signalvalue = cOk)
  *
  * @description
  *  Indicates that the given generic signal was raised.
  *  Wakes all threads currently waiting for that condition and reschedules them.
  *
  * @params
  *  sig:      The global signal raised.
  *  sigvalue: The value of the signal passed as return code to the thread.
  *---------------------------------------------------------------------------*/
void SingleCPUDispatcher::signal(void* sig, int signalvalue) {
    this->signal(waitList, sig, signalvalue);
}

#endif

/*****************************************************************************
 * Method: SingleCPUDispatcher::terminate_thread(Thread* thread)
 *
 * @description
 *  Tells the disptacher that the given thread has terminated. The thread is
 *  removed from all lists.
 *
 * @params
 *  thread:     The thread that terminated
 *---------------------------------------------------------------------------*/
void SingleCPUDispatcher::terminate_thread(Thread* thread) {
    int irqstatus;
    DISABLE_IRQS(irqstatus);
    thread->getLinkedListItem()->remove();

    if (pCurrentRunningThread == thread) {
        TRACE_THREAD_STOP(pCurrentRunningTask->getId(), pCurrentRunningThread->getId());

        pCurrentRunningThread = 0;

        /* Delete thread object.
         * Beware: It must have been removed from the task! */
        delete thread;
        dispatch();
        __builtin_unreachable();
    }

    RESTORE_IRQS(irqstatus);
}
