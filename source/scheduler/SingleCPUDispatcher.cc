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
#include <sprintf.hh>

extern Kernel* theOS;
extern Board_TimerCfdCl* theTimer;

/* Global Variables to speed up access to these objects */
LinkedListDatabaseItem* pRunningThreadDbItem;
Kernel_SchedulerCfdCl* 	theScheduler;
Kernel_ThreadCfdCl*    	pCurrentRunningThread = 0;
Task*          			pCurrentRunningTask = 0;
TimeT           		lastCycleStamp = 0;
bool					processChanged = true;


SingleCPUDispatcher::SingleCPUDispatcher() :
  blockedList( new LinkedListDatabase )
, sleepList( new LinkedListDatabase )
#ifdef ORCOS_SUPPORT_SIGNALS
,waitList( new LinkedListDatabase )
#endif
{
	/* Initialize the scheduler */
    SchedulerCfd 			= new NEW_Kernel_SchedulerCfd;
    theScheduler 			= SchedulerCfd;
    pRunningThreadDbItem 	= 0;
    pCurrentRunningThread 	= 0;
    pCurrentRunningTask 	= 0;
    /* Set idle thread */
    idleThread 				= new IdleThread();
}

SingleCPUDispatcher::~SingleCPUDispatcher() {
}

void SingleCPUDispatcher::dispatch() {
     /* first be sure that interrupts are disabled */
    _disableInterrupts();

    TimeT currentTime = theOS->getClock()->getTimeSinceStartup();
    /* set the time stamp */
    lastCycleStamp = currentTime;

    /* check whether the idle thread was currently running or not
       the idle thread would have pRunningThreadDbItem = 0 */
    if ( pRunningThreadDbItem != 0 ) {
        this->SchedulerCfd->enter( pRunningThreadDbItem );
    }

    /* get the next timer event the scheduler wants to be called */
    TimeT nextevent = this->SchedulerCfd->getNextTimerEvent(sleepList,currentTime);
    LOG(SCHEDULER,DEBUG,(SCHEDULER,DEBUG,"SingleCPUDispatcher: next Timer %x %x", (unint4) ((nextevent >> 32) & 0xffffffff),  (unint4) ((nextevent) & 0xffffffff)));
    theTimer->setTimer( nextevent );

    /* get the next ready thread
      Be sure to call getNextTimerEvent() before this, since depending on the scheduler the returned
      timeslice might depend on the Threads in the scheduler! (e.g. RateMonotonicThreadScheduler) */
    LinkedListDatabaseItem* nextThreadDbItem = (LinkedListDatabaseItem*) SchedulerCfd->getNext();

    /* Any read thread?  */
    if ( nextThreadDbItem != 0 ) {
        pCurrentRunningThread = (Kernel_ThreadCfdCl*) nextThreadDbItem->getData();
        pCurrentRunningTask   = pCurrentRunningThread->getOwner();

        ASSERT(pCurrentRunningThread);
        ASSERT(pCurrentRunningTask);

        int tid = pCurrentRunningThread->getId();
        pRunningThreadDbItem = nextThreadDbItem;

        TRACE_THREAD_START(tid,pCurrentRunningThread->getId());

        /* Is this thread new (has no context)?*/
        if ( pCurrentRunningThread->isNew() ) {
            LOG(SCHEDULER,DEBUG,(SCHEDULER,DEBUG,"SingleCPUDispatcher: start Thread %d at %x, stack [0x%x - 0x%x]", tid, pCurrentRunningThread,pCurrentRunningThread->threadStack.startAddr,pCurrentRunningThread->threadStack.endAddr));
            pCurrentRunningThread->callMain();
        }
        else {
            LOG(SCHEDULER,DEBUG,(SCHEDULER,DEBUG,"SingleCPUDispatcher: resume Thread %d at %x",tid, pCurrentRunningThread));
            assembler::restoreContext( pCurrentRunningThread );
        }
    }
    else {
        pCurrentRunningThread = 0;
        pCurrentRunningTask = 0;
        pRunningThreadDbItem = 0;

        LOG(KERNEL,DEBUG,(KERNEL,DEBUG,"SingleCPUDispatcher: Idle Thread running"));
        /* non returning run() */
        idleThread->run();
    }


}




void SingleCPUDispatcher::sleep( LinkedListDatabaseItem* pSleepDbItem ) {
  /* be sure that this critical area cant be interrupted */
  DISABLE_IRQS(irqstatus);

  /* place into sleeplist */
   this->sleepList->addTail( (LinkedListDatabaseItem*) pSleepDbItem );

    if ( pSleepDbItem == pRunningThreadDbItem ) {
        pRunningThreadDbItem = 0;
        dispatch();
    }

    RESTORE_IRQS(irqstatus);

}

void SingleCPUDispatcher::block( Thread* thread ) {
    /* be sure that this critical area cant be interrupted */
	DISABLE_IRQS(irqstatus);

    LOG(SCHEDULER,DEBUG,(SCHEDULER,DEBUG,"SingleCPUDispatcher::block() blocking Thead %d", thread->getId()));

   	TRACE_THREAD_STOP(thread->getOwner()->getId(),thread->getId());

    if ( thread == pCurrentRunningThread ) {
        this->blockedList->addTail( pRunningThreadDbItem );
        pRunningThreadDbItem = 0;

        LOG(SCHEDULER,INFO,(SCHEDULER,INFO,"SingleCPUDispatcher::block() dispatching! %d",this->SchedulerCfd->database.getSize()));
        dispatch( );
    }
    else {
        // find the databaseitem of this thread and remove it from the scheduler
        LinkedListDatabaseItem* litem;

        // if the thread is currently sleeping remove it from the sleeplist
        if ( thread->sleepTime > 0 ) {
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

    RESTORE_IRQS(irqstatus);
}

void SingleCPUDispatcher::unblock( Thread* thread ) {
    // be sure that this critical area cant be interrupted
	DISABLE_IRQS(irqstatus);

    LOG(SCHEDULER,DEBUG,(SCHEDULER,DEBUG,"SingleCPUDispatcher::unblock() unblocking Thead %d", thread->getId()));
    LinkedListDatabaseItem* litem = this->blockedList->getItem( thread );

    if ( litem != 0 ) {
        Thread* pThread = (Thread*) litem->getData();
        // check if there is still some remaining sleep time on this thread
        if ( pThread->sleepTime > 0 )
            this->sleepList->addTail( litem );
        else {
            /* TODO Ensure Dispatch! since we may have priorities!!!!!!
             * Currently IRQ/syscall handler has to call dispatch for now.
             * */
            this->SchedulerCfd->enter( litem );
        }

    }

    RESTORE_IRQS(irqstatus);
}


#ifdef ORCOS_SUPPORT_SIGNALS

void SingleCPUDispatcher::sigwait( Thread* thread ) {
    // be sure that this critical area cant be interrupted
	DISABLE_IRQS(irqstatus);

    LOG(SCHEDULER,TRACE,(SCHEDULER,TRACE,"SingleCPUDispatcher::sigwait() adding Thread to waitlist"));

    if (thread == pCurrentRunningThread) {
        waitList->addTail(pRunningThreadDbItem);
        pRunningThreadDbItem = 0;

        dispatch( );
    }
    else {
        // find the databaseitem of this thread and remove it from the scheduler
        LinkedListDatabaseItem* litem = 0;

        // if the thread is currently sleeping remove it from the sleeplist
        if ( thread->sleepTime > 0 ) {
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
            // a new linkedlist databaseitem to hold the reference
            // from now on
            litem = new LinkedListDatabaseItem( thread );
        }

        this->waitList->addTail( litem );
    }

    RESTORE_IRQS(irqstatus);
}

void SingleCPUDispatcher::signal( void* sig, int sigvalue ) {
    /* be sure that this critical area cannot be interrupted */
	DISABLE_IRQS(irqstatus);

    LOG(SCHEDULER,TRACE,(SCHEDULER,TRACE,"SingleCPUDispatcher::signal() signal: %d",sig));

    LinkedListDatabaseItem* litem = this->waitList->getHead();

    for ( unint i = 0; i < waitList->getSize(); i++ ) {
        LinkedListDatabaseItem* tmpLitem = litem->getSucc();
        Thread* pThread = (Thread*) litem->getData();

        if ( litem != 0 ) {
            if ( pThread->signal == sig ) {
                LOG(SCHEDULER,TRACE,(SCHEDULER,TRACE,"SingleCPUDispatcher::signal() signal to thread %d", pThread->getId()));

                pThread->status.clearBits( cSignalFlag );
                /* Set the signal return value. The restore Context must handle passing this value
                 * to the return register. It should reset the signal to 0 to indicate that the thread is not
                 * waiting for a signal any more and ignore the signalvalue field afterwards.
                 *  */
                pThread->signalvalue = sigvalue;

                if ( !pThread->status.areSet( cStopped ) ) {

                    if ( pThread->isBlocked() ) {
                        this->blockedList->addTail( litem );
                    }
                    /* check if there is still some remaining sleep time on this thread */
                    else if (pThread->sleepTime > 0 ) {
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

    RESTORE_IRQS(irqstatus);
}

#endif

void SingleCPUDispatcher::terminate_thread( Thread* thread ) {
    if ( pCurrentRunningThread == thread ) {
    	TRACE_THREAD_STOP(pCurrentRunningTask->getId(),pCurrentRunningThread->getId());

    	DISABLE_IRQS(irqstatus);

        /* TODO: maybe keep the thread as zombie if we add scheduleability tests */
        if (thread->owner->getThreadDB()->getSize() == 0)
        	theOS->getTaskManager()->removeTask(thread->owner);

        pRunningThreadDbItem = 0;

        dispatch();
    }
    else {
   	    /* if the thread is currently sleeping remove it from the sleeplist */
		if ( thread->sleepTime > 0 )
			this->sleepList->remove( thread );
		else if (thread->isBlocked())
			this->blockedList->remove( thread );
		else
			this->SchedulerCfd->remove( thread );

    }
}
