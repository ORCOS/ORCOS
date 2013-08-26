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

//! architecture dependend assembler functions include
#include <assemblerFunctions.hh>
#include "kernel/Kernel.hh"
#include <sprintf.hh>

// the minimal timer intervall the system can handle in microSec
// see context  switching times as a reference value
#define MINIMAL_TIMER_INTERVAL 20 * (CLOCK_RATE / 1000000)

extern Kernel* theOS;
extern Board_ClockCfdCl* theClock;
extern Board_TimerCfdCl* theTimer;

// Global Variables to speed up access to these objects
LinkedListDatabaseItem* pRunningThreadDbItem;
SingleCPUDispatcher_SchedulerCfdCl* theScheduler;
ThreadCfdCl*    pCurrentRunningThread = 0;
Task*           pCurrentRunningTask = 0;
unint8          lastCycleStamp = 0;

SingleCPUDispatcher::SingleCPUDispatcher() :
    blockedList( new LinkedListDatabase ), sleepList( new LinkedListDatabase ), waitList( new LinkedListDatabase ) {
    // initialize the scheduler with 0, needed for the new Operator!
#ifdef HAS_SingleCPUDispatcher_SchedulerCfd
    SchedulerCfd = new NEW_SingleCPUDispatcher_SchedulerCfd;
#endif

    theScheduler = SchedulerCfd;
    pRunningThreadDbItem = 0;
    pCurrentRunningThread = 0;
    pCurrentRunningTask = 0;
}

SingleCPUDispatcher::~SingleCPUDispatcher() {
}

void SingleCPUDispatcher::dispatch( unint4 dt ) {
     // first be sure that interrupts are disabled
    _disableInterrupts();


    // check whether the idle thread was currently running or not
    // the idle thread would have pRunningThreadDbItem = 0
    if ( pRunningThreadDbItem != 0 ) {
        this->SchedulerCfd->enter( pRunningThreadDbItem );
    }

    // get the next timer event the scheduler wants to be called
    int nextevent = this->SchedulerCfd->getNextTimerEvent(sleepList,dt);
    LOG(SCHEDULER,TRACE,(SCHEDULER,TRACE,"SingleCPUDispatcher: next Timer %d", nextevent));
    theTimer->setTimer( nextevent );

    // get the next ready thread
    // Be sure to call getNextTimerEvent() before this, since depending on the scheduler the returned
    // timeslice might depend on the Threads in the scheduler! (e.g. RateMonotonicThreadScheduler)
    LinkedListDatabaseItem* nextThreadDbItem = (LinkedListDatabaseItem*) SchedulerCfd->getNext();

     // set the time stamp
    lastCycleStamp = theClock->getTimeSinceStartup();

    if ( nextThreadDbItem != 0 ) {
        pCurrentRunningThread = (ThreadCfdCl*) nextThreadDbItem->getData();
        pCurrentRunningTask   = pCurrentRunningThread->getOwner();

        ASSERT(pCurrentRunningThread);
        ASSERT(pCurrentRunningTask);

#ifdef HAS_Kernel_LoggerCfd
        int tid = pCurrentRunningThread->getId();
#endif
        pRunningThreadDbItem = nextThreadDbItem;

        if ( pCurrentRunningThread->isNew() ) {

            LOG(SCHEDULER,DEBUG,(SCHEDULER,DEBUG,"SingleCPUDispatcher: start Thread %d at %x", tid, pCurrentRunningThread));
#if USE_TRACE
            theOS->getTrace()->addExecutionTrace(tid,lastCycleStamp);
#endif

            pCurrentRunningThread->callMain();
        }
        else {

            LOG(SCHEDULER,DEBUG,(SCHEDULER,DEBUG,"SingleCPUDispatcher: resume Thread %d at %x",tid, pCurrentRunningThread));
#if USE_TRACE
            theOS->getTrace()->addExecutionTrace(tid,lastCycleStamp);
#endif

            assembler::restoreContext( pCurrentRunningThread );
        }
    }
    else {
        // reset the stack pointer to the context address since we don't want to waste memory
        //void* sp = pCurrentRunningThread->threadStack.stackptrs[pCurrentRunningThread->threadStack.top];
        pCurrentRunningThread = 0;
        pCurrentRunningTask = 0;
        pRunningThreadDbItem = 0;

        //LOG(KERNEL,DEBUG,(KERNEL,DEBUG,"SingleCPUDispatcher: Idle Thread running"));
#if USE_TRACE
        theOS->getTrace()->addExecutionTrace(0,lastCycleStamp);
#endif

        // reset back to the virtual memory of the kernel

        // non returning run()
        idleThread->run();
    }


}


void SingleCPUDispatcher::dispatch() {
	dispatch(theClock->getTimeSinceStartup() - lastCycleStamp);
}

void SingleCPUDispatcher::sleep( int cycles, LinkedListDatabaseItem* pSleepDbItem ) {
    // be sure that this critical area cant be interrupted
#if ENABLE_NESTED_INTERRUPTS
    bool int_enabled;
    GET_INTERRUPT_ENABLE_BIT(int_enabled);

    _disableInterrupts();
#endif

    if ( cycles > 0 ) {
       // LOG(KERNEL,INFO,(KERNEL,INFO,"SingleCPUDispatcher::sleep(): %d",cycles));
        // the specified thread wants to sleep
        this->sleepList->addTail( (LinkedListDatabaseItem*) pSleepDbItem );
    }
    else {
        // dont sleep. be sure ready flag is set.
        //LOG(SCHEDULER,TRACE,(SCHEDULER,TRACE,"SingleCPUDispatcher::sleep() Thread sleepcycles <= 0 (%d) putting in ready queue.",cycles));
        ( (ThreadCfdCl*) pSleepDbItem->getData() )->status.setBits( cReadyFlag );
        // announce thread to scheduler again
        this->SchedulerCfd->enter( (LinkedListDatabaseItem*) pSleepDbItem );
    }

    if ( pSleepDbItem == pRunningThreadDbItem ) {
        pRunningThreadDbItem = 0;
#if ENABLE_NESTED_INTERRUPTS
        pCurrentRunningThread->executinginthandler = false;
#endif
        dispatch( theClock->getTimeSinceStartup() - lastCycleStamp );
    }

#if ENABLE_NESTED_INTERRUPTS
        if ( int_enabled ) {
            _enableInterrupts();
        }
#endif

}

void SingleCPUDispatcher::block( Thread* thread ) {
    // be sure that this critical area cant be interrupted
#if ENABLE_NESTED_INTERRUPTS
    bool int_enabled;
    GET_INTERRUPT_ENABLE_BIT(int_enabled);

    _disableInterrupts();
#endif
    LOG(SCHEDULER,TRACE,(SCHEDULER,TRACE,"SingleCPUDispatcher::block() blocking Thead %d", thread->getId()));

    if ( thread == pCurrentRunningThread ) {
        this->blockedList->addTail( pRunningThreadDbItem );
        pRunningThreadDbItem = 0;
#if ENABLE_NESTED_INTERRUPTS
        pCurrentRunningThread->executinginthandler = false;
#endif
       // LOG(SCHEDULER,INFO,(SCHEDULER,INFO,"SingleCPUDispatcher::block() dispatching! %d",this->SchedulerCfd->database.getSize()));
        dispatch( theClock->getTimeSinceStartup() - lastCycleStamp );
    }
    else {
        // find the databaseitem of this thread and remove it from the scheduler
        LinkedListDatabaseItem* litem;

        // if the thread is currently sleeping remove it from the sleeplist
        if ( thread->sleepCycles > 0 ) {
            litem = this->sleepList->getItem( thread );
        }
        else
            litem = this->SchedulerCfd->remove( thread );

        if ( litem == 0 ) {
            // thread must be new since it is not
            // referenced anywhere so lets create
            // a new databaseitem to hold the reference
            // from now on
            litem = new LinkedListDatabaseItem( thread );
        }

        this->blockedList->addTail( litem );

    }

#if ENABLE_NESTED_INTERRUPTS
    if ( int_enabled ) {
        _enableInterrupts();
    }
#endif

}

void SingleCPUDispatcher::unblock( Thread* thread ) {
    // be sure that this critical area cant be interrupted
#if ENABLE_NESTED_INTERRUPTS
    bool int_enabled;
    GET_INTERRUPT_ENABLE_BIT(int_enabled);

    _disableInterrupts();

#endif

    LinkedListDatabaseItem* litem = this->blockedList->getItem( thread );

    if ( litem != 0 ) {
        Thread* pThread = (Thread*) litem->getData();
        // check if there is still some remaining sleep time on this thread
        if ( pThread->sleepCycles > 0 )
            this->sleepList->addTail( litem );
        else {
            // TODO Dispatch! since we may have priorities!!!!!!
            this->SchedulerCfd->enter( litem );
        }

    }

#if ENABLE_NESTED_INTERRUPTS
    if ( int_enabled ) {
        _enableInterrupts();
    }

#endif
}

void SingleCPUDispatcher::sigwait( Thread* thread ) {
    // be sure that this critical area cant be interrupted
#if ENABLE_NESTED_INTERRUPTS
    bool int_enabled;
    GET_INTERRUPT_ENABLE_BIT(int_enabled);

    _disableInterrupts();

#endif

    LOG(SCHEDULER,TRACE,(SCHEDULER,TRACE,"SingleCPUDispatcher::sigwait() adding Thread to waitlist"));

    if (thread == pCurrentRunningThread) {
        waitList->addTail(pRunningThreadDbItem);
        pRunningThreadDbItem = 0;
#if ENABLE_NESTED_INTERRUPTS
        pCurrentRunningThread->executinginthandler = false;
#endif
        dispatch( theClock->getTimeSinceStartup() - lastCycleStamp );
    }
    else {
        // find the databaseitem of this thread and remove it from the scheduler
        LinkedListDatabaseItem* litem = 0;

        // if the thread is currently sleeping remove it from the sleeplist
        if ( thread->sleepCycles > 0 ) {
            litem = this->sleepList->getItem( thread );
        }
        else if ( ((Thread*)litem->getData())->status.areSet(cBlockedFlag) ){
            litem = this->blockedList->getItem( thread );
        }
        else {
            litem = this->SchedulerCfd->remove( thread );
        }

        if ( litem == 0 ) {
            // thread must be new since it is not
            // referenced anywhere so lets create
            // a new databaseitem to hold the reference
            // from now on
            litem = new LinkedListDatabaseItem( thread );
        }

        this->waitList->addTail( litem );
    }
#if ENABLE_NESTED_INTERRUPTS
    if ( int_enabled ) {
        _enableInterrupts();
    }

#endif
}

void SingleCPUDispatcher::signal( void* sig ) {
    // be sure that this critical area cant be interrupted
#if ENABLE_NESTED_INTERRUPTS
    bool int_enabled;
    GET_INTERRUPT_ENABLE_BIT(int_enabled);

    _disableInterrupts();

#endif

    LOG(SCHEDULER,TRACE,(SCHEDULER,TRACE,"SingleCPUDispatcher::signal() signal: %d",sig));

    LinkedListDatabaseItem* litem = this->waitList->getHead();

    for ( unint i = 0; i < waitList->getSize(); i++ ) {
        LinkedListDatabaseItem* tmpLitem = litem->getSucc();
        Thread* pThread = (Thread*) litem->getData();

        if ( litem != 0 ) {
            if ( pThread->signal == sig ) {
                LOG(SCHEDULER,TRACE,(SCHEDULER,TRACE,"SingleCPUDispatcher::signal() signal to thread %d", pThread->getId()));

                pThread->status.clearBits( cSignalFlag );
                if ( !pThread->status.areSet( cStopped ) ) {

                    if ( pThread->status.areSet( cBlockedFlag ) ) {
                        this->blockedList->addTail( litem );
                    }
                    // check if there is still some remaining sleep time on this thread
                    else if ( pThread->sleepCycles > 0 ) {
                        this->sleepList->addTail( litem );
                    }
                    else {
                        this->SchedulerCfd->enter( litem );
                    }
                }
            }
        }

        litem = tmpLitem;
    }

#if ENABLE_NESTED_INTERRUPTS
    if ( int_enabled ) {
        _enableInterrupts();
    }

#endif
}

void SingleCPUDispatcher::terminate_thread( Thread* thread ) {
    if ( pCurrentRunningThread == thread ) {

#if ENABLE_NESTED_INTERRUPTS
        _disableInterrupts();
        // at this time we are not executing the interrupt handler any more!
        pCurrentRunningThread->executinginthandler = false;
#endif

        // TODO: maybe keep the thread as zombie
        // delete thread;

        pRunningThreadDbItem = 0;

        dispatch( theClock->getTimeSinceStartup() - lastCycleStamp );
    }
    else {
   	     	    // if the thread is currently sleeping remove it from the sleeplist
		if ( thread->sleepCycles > 0 )
			this->sleepList->remove( thread );
		else if (thread->isBlocked())
			this->blockedList->remove( thread );
		else
			this->SchedulerCfd->remove( thread );

    }
}
