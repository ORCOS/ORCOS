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

// includes
#include "RealTimeThread.hh"
#include "kernel/Kernel.hh"
#include "lib/defines.h"
#include "assembler.h"

// Kernel definition, needed for clock
extern Kernel* theOS;
extern unint8   lastCycleStamp;
extern Board_ClockCfdCl* theClock;
extern SingleCPUDispatcher_SchedulerCfdCl* theScheduler;

extern unint4 GPIO_VAL;

#define FAIL_ADDR          0xcb000000

RealTimeThread::RealTimeThread( void* startRoutinePointer, void* exitRoutinePointer, Task* owner,
        MemoryManagerCfdCl* memManager, unint4 stack_size, void* RTThreadAttributes, bool newThread ) :
    PriorityThread( startRoutinePointer, exitRoutinePointer, owner, memManager, stack_size, RTThreadAttributes,
            newThread ) {
    if ( RTThreadAttributes != 0 ) {
        // set the parameters and convert them from µs to cylces
    	thread_attr_t* attr = static_cast< thread_attr_t* > ( RTThreadAttributes );
		#if CLOCK_RATE >= (1 MHZ)
			this->relativeDeadline = ((unint8) attr->deadline) * (CLOCK_RATE / 1000000);
			this->executionTime = ((unint8) attr->executionTime) * (CLOCK_RATE / 1000000);
			this->period = ((unint8) attr->period) * (CLOCK_RATE / 1000000);
		#else
			this->relativeDeadline = (unint8) (((float)attr->deadline) * ((float) CLOCK_RATE / 1000000.0f));
			this->executionTime = (unint8) (((float)attr->executionTime) * ((float) CLOCK_RATE / 1000000.0f));
			this->period = (unint8) (((float)attr->period) * ((float) CLOCK_RATE / 1000000.0f));
		#endif

        LOG(PROCESS,WARN,( PROCESS, WARN, "RealtimeThread:() period: %d",this->period));

        unint8 currentCycles = theClock->getTimeSinceStartup();
        // set the new arrivaltime of this thread
        this->arrivalTime = currentCycles + attr->phase;
        this->absoluteDeadline = this->arrivalTime + this->relativeDeadline;
    }
    else {
        this->relativeDeadline = 0;
        this->executionTime = 0;
        this->period = 0;
        this->absoluteDeadline = 0;
    }
    this->instance = 1;
    this->arguments = (void*) instance;

#ifdef AIS
    lasterror = *((unsigned int*)FAIL_ADDR);
#endif

    theScheduler->computePriority(this);
}

RealTimeThread::~RealTimeThread() {
}


void RealTimeThread::terminate() {
    if ( period > 0 ) {
        // we need to disable interrupts since we will reset the thread context
        // and really shouldn't be interrupted afterwards (since there is no way to resume).
        #if ENABLE_NESTED_INTERRUPTS
            _disableInterrupts();
        #endif

       /*
        * This is for logging time points with an external logic analyzer
        *
        * GPIO_VAL = ((1UL << (29 + this->getId() )));
        *((volatile unsigned*) 0x90230000) = 	GPIO_VAL;
        *((volatile unsigned*) 0x90230000) = 	0;
        */

        // Reset the thread context. Fortunately it is this simple ^_^.
        threadStack.top = 0;

        // Set the status to new so that the call main method will be called instead of restoring the context.
        this->status.set( cNewFlag | cReadyFlag );

        // REMARK: we waste 4 bytes here since we could also use the arguments variable as a counter for the instance
        this->instance++;
        this->arguments = (void*) instance;

        #if USE_SAFE_KERNEL_STACKS
        // we use safe kernel stacks so we must free the stack slot now
           int2 myBucketIndex =  this->getKernelStackBucketIndex();
           FREE_KERNEL_STACK_SLOT(myBucketIndex);
        #endif

        unint8 currentCycles = theClock->getTimeSinceStartup();

        // Compute the remaining time left till the next instance of this thread is due to start.
        int8 sleepcycles = (int8) period - ((int8) currentCycles - (int8) arrivalTime);
        if (sleepcycles < 0) sleepcycles = 0;

#ifdef HAS_Kernel_LoggerCfd
        // check if we missed our deadline
        if(currentCycles > this->absoluteDeadline) {
        	theOS->getLogger()->log(PROCESS,FATAL,"RealTimeThread ID %d failed deadline! cur_cyles: %d, deadline: %d, sleep: %d", this->getId(), (unint4) currentCycles, (unint4) this->absoluteDeadline, (unint4) sleepcycles);
        	// omit serial console time for now .. this is not correct but good for debugging
        	currentCycles = theClock->getTimeSinceStartup();
        }
#endif

#ifdef DEBUG_EXECUTION_TIMES
        theOS->getLogger()->log(PROCESS,WARN,  "exec_time: %d", (unint4) (currentCycles - arrivalTime) / (CLOCK_RATE / 1000000));
        currentCycles = theClock->getTimeSinceStartup();
#endif
        // set the new arrivaltime of this thread
        this->arrivalTime += this->period; //= currentCycles + sleepcycles;
        this->absoluteDeadline = this->arrivalTime + this->relativeDeadline;

        // invoke the computePriority method of the scheduler
        theScheduler->computePriority(this);

        // sleep for the computed interval.
        this->sleep( (unint4) sleepcycles );
    }
    else {
        unint8 currentCycles = theClock->getTimeSinceStartup();
        LOG(PROCESS,WARN,( PROCESS, WARN, "RealtimeThread::terminate() lateness: %d",(int4) (currentCycles - this->absoluteDeadline) ));

#ifdef AIS_MIGRATE

      volatile unsigned int errorc = *((unsigned int*)FAIL_ADDR);
      volatile unsigned int diff = errorc - lasterror;
      lasterror = errorc;

      LOG(PROCESS,WARN,( PROCESS, WARN, "RealtimeThread::terminate() errors: %d",diff ));


      unint4 threshold = 1200;
      if ( diff >= threshold )
      {
    	  // threshold reached!
    	  // migrate now
          LOG(PROCESS,WARN,( PROCESS, WARN, "RealtimeThread::terminate() migrating whole task!" ));

          sockaddr destination;

          destination.port_data =   1;
          destination.sa_data =     IP4_ADDR(192,168,1,10);
          destination.name_data[0] = '\0';

          if (isOk(theOS->getMigrationManager()->migrateTask(this->getOwner(),&destination))) {
        	  //this->getOwner()->terminate();
          }
          else {
        	  LOG(PROCESS,WARN,( PROCESS, WARN, "RealtimeThread::terminate() could not replicate task to destination node!" ));
          }

      }
#endif

        Thread::terminate();
    }
}

