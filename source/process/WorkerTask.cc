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
    Task() {

    LOG(KERNEL,INFO,(KERNEL,INFO,"WorkerTask(): creating %d Workerthreads", NUM_WORKERTHREADS));
    WorkerThread* pWThread;
    // create some worker threads
    for ( int i = 0; i < NUM_WORKERTHREADS; i++ ) {
        pWThread = new WorkerThread( this );
        // announce the workerthread to the dispatcher by blocking it
        pWThread->block();
    }

    LOG(KERNEL,INFO,(KERNEL,INFO,"WorkerTask(): Workerthread Ids: 1-%d", pWThread->getId()));

    this->memManager = theOS->getMemoryManager();

}

WorkerTask::~WorkerTask() {
}

WorkerThread* WorkerTask::addJob( unint1 id, unint1 pid, void* param, unint priority_param ) {
	LOG(PROCESS,DEBUG,(PROCESS,DEBUG,"WorkerTask::addJob(): job %d",id));

    // find a available workerthread and assign the job
    LinkedListDatabaseItem* litem = this->threadDb.getHead();

    // find a workerthread that is not assigned to a job yet
    // TODO: we can enhance this by using a non-working queue
    while ( (litem != 0) &&  (  ( (WorkerThread*) litem->getData() )->hasJob())  )
        litem = litem->getSucc();

    if ( litem != 0 ) {
        WorkerThread* pWThread = (WorkerThread*) litem->getData();
        pWThread->setJob( id, param );
        pWThread->setPID( pid );

        LOG(PROCESS,DEBUG,(PROCESS,DEBUG,"WorkerTask::addJob() assigned thread %d for job %d",pWThread->getId(),id));

        // get the current cycles
        unint8 currentCycles = theClock->getTimeSinceStartup();

#ifdef HAS_PRIORITY
    #ifndef REALTIME
        pWThread->setInitialPriority( priority_param );
        pWThread->setEffectivePriority( priority_param );
    #else
        // reset the instance to 1
        pWThread->instance = 1;

		#if CLOCK_RATE >= (1 MHZ)
        	 // set the relative deadline for EDF
             pWThread->relativeDeadline = priority_param * (CLOCK_RATE / 1000000);
             // set period for RM
             pWThread->period = priority_param * (CLOCK_RATE / 1000000)
		#else
			 pWThread->relativeDeadline = (unint8) ((priority_param * CLOCK_RATE) /  1000000U);
             pWThread->period = (unint8) ((priority_param * CLOCK_RATE) /  1000000U);
			//pWThread->relativeDeadline = (unint8) (((float)priority_param) * ((float) CLOCK_RATE / 1000000.0f));
			//pWThread->period = (unint8) (((float)priority_param) * ((float) CLOCK_RATE / 1000000.0f));
		#endif


        // set the arrival time to now!
        pWThread->arrivalTime = currentCycles;
    #endif
        theScheduler->computePriority(pWThread);
#endif

        if ( id == TimedFunctionCallJob || id == PeriodicFunctionCallJob ) {
            // set the sleeptime so the thread sleeps
            // until the the function can be called
            TimedFunctionCall* funcCall = (TimedFunctionCall*) param;
            // when are wo going to be called the first time? sleep until that point in time
            pWThread->sleepCycles = (unint4) (funcCall->time - currentCycles);
        }
        else
            pWThread->sleepCycles = 0;

        // unblock the workerthread
        // this will cause the thread
        // either to be scheduled directly
        // or send to sleep if the sleeptime is > 0
        pWThread->unblock();
        return (pWThread);
    }
    else
    {
        ERROR("No WorkerThread available!");

        // no available workerthread
        return (0);
    }
}
