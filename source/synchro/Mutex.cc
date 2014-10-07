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


#include "Mutex.hh"
#include "kernel/Kernel.hh"
#include "process/Task.hh"
#include "filesystem/File.hh"

extern Kernel* theOS;
extern Kernel_ThreadCfdCl* pCurrentRunningThread;
extern Task*  pCurrentRunningTask;

#ifndef USE_PIP
/* If not configured we use PIP */
#define USE_PIP 1
#endif

Mutex::Mutex() :
    m_locked( false ), m_pThread( 0 ), m_stoppedThreads(4), m_pRes(0) {
    SchedulerCfd = new Kernel_SchedulerCfdCl( );
}

Mutex::~Mutex() {
    delete SchedulerCfd;

    // TODO: waiting threads?? should not happen

#if MEM_NO_FREE
    LinkedListItem* item = m_unusedLinkedListDBItems.removeHead();
    while (item) {
        delete item;
        item = m_unusedLinkedListDBItems.removeHead();
    }
#endif
}

ErrorT Mutex::acquire( Resource* pRes, bool blocking ) {
	Kernel_ThreadCfdCl* pCallingThread = pCurrentRunningThread;

    DISABLE_IRQS(irqstatus);

    if (m_locked == false)
    {
        /* successfully acquired mutex */
        m_locked 	= true;
        m_pThread 	= pCallingThread;
        m_pRes 		= pRes;

        if ( m_pRes != 0) {
        	 if (pCurrentRunningTask != 0)
        		 pCurrentRunningTask->aquiredResources.addTail( pRes );

        	 /* reset the file position on newly acquired files */
        	 if (m_pRes->getType() == cFile) ((File*) m_pRes)->resetPosition();
        }

       RESTORE_IRQS(irqstatus);
       return (cOk);
    }
    else
    {
      if (!blocking) {
          RESTORE_IRQS(irqstatus);
          return (cError);
      }
        /* Mutex already acquired by some other thread! */

        /* Implementation of the Priority Inheritance Protocol (PIP)
           See if the newly arrived Thread has a higher priority than the thread currently locking the semaphore.
           If this is the case, set the priority of the currently owning Thread to that of the newly arrived one.
        */
        #ifdef HAS_PRIORITY
			#if USE_PIP
				if ( pCallingThread != 0 && m_pThread != 0 && m_pThread->effectivePriority < pCallingThread->effectivePriority ) {
					/* boost priority */
					m_pThread->effectivePriority = pCallingThread->effectivePriority ;
				}
			#endif
        #endif

#if MEM_NO_FREE
        /* enter thread into scheduler and reuse linkedlistdatabaseitems to avoid memory leaks! */
        if ( m_unusedLinkedListDBItems.isEmpty() ) {
#endif
		   getScheduler()->enter( pCallingThread );
#if MEM_NO_FREE
	   }
	   else {
		   /* we have an unused LLDB item, so we reuse it instead of creating a new one */
		   LinkedListItem* newItem = m_unusedLinkedListDBItems.removeHead();
		   newItem->setData( pCallingThread );
		   getScheduler()->enter( newItem );
	   }
#endif

        /* now block the calling thread! */
        pCallingThread->pBlockedMutex = this;
        pCallingThread->block();

        // TODO: how to handle unblocked threads if this mutex is deleted?
        // they will get here and reference invalid memory locations!
        // solution to be implemented: delayed removal of mutexes and resources!
        // mark as invalid resource and wake up all thread => then remove
        RESTORE_IRQS(irqstatus);
        return (cOk);
    }

   RESTORE_IRQS(irqstatus);
   return (cError);
}


ErrorT Mutex::release(Thread* pThread ) {

    DISABLE_IRQS(irqstatus);

    /* remove the resource from the acquired list of the owner task */
    if ( m_pRes != 0 && pThread != 0 ) {
        ListItem* removedItem = pThread->getOwner()->aquiredResources.removeItem( m_pRes );
         if (removedItem == 0) {
             RESTORE_IRQS(irqstatus);
             LOG(SYNCHRO,ERROR,"Mutex::release() Could not remove %s from thread %d",m_pRes->getName(),pThread->getId());
             return (cElementNotInDatabase);
         }
    }

    LOG(SYNCHRO,TRACE,"Mutex 0x%x released",this);


#ifdef HAS_PRIORITY
#if USE_PIP
    Kernel_ThreadCfdCl* pCallingThread = (Kernel_ThreadCfdCl*) pThread;
    /* reset the priority of the currentRunning thread */
    if (pCallingThread != 0)
    	pCallingThread->effectivePriority = pCallingThread->initialPriority;
#endif
#endif

      /* get the next thread by the scheduler who is next to acquire the mutex */
      LinkedListItem* next = (LinkedListItem*) getScheduler()->getNext();
      Kernel_ThreadCfdCl* pSchedulerThread;

      /* search the first not stopped thread */
      while (next != 0) {
          pSchedulerThread = (Kernel_ThreadCfdCl*) next->getData();
          if ( pSchedulerThread->isStopped() ) m_stoppedThreads.addTail( pSchedulerThread );
          else {
        	  /* available thread found */

#if MEM_NO_FREE
			   m_unusedLinkedListDBItems.addHead(next);
#else
			   delete next;
#endif
			   m_pThread = pSchedulerThread;
			   pSchedulerThread->pBlockedMutex = 0;

			   if ( m_pRes != 0) {
				   pSchedulerThread->getOwner()->aquiredResources.addTail( m_pRes );
				   /* reset the file position */
				   if (m_pRes->getType() == cFile) ((File*) m_pRes)->resetPosition();
			   }

			   /* unblock the waiting thread */
			   pSchedulerThread->unblock();

			   RESTORE_IRQS(irqstatus);

			   return (cOk);
          }
          next = (LinkedListItem*) getScheduler()->getNext();
      }

    /* no available thread to aquire the mutex */
    m_locked 	= false;
    m_pThread   = 0;
    m_pRes 	    = 0;

    RESTORE_IRQS(irqstatus);

    return (cOk);

}

void Mutex::threadResume( Kernel_ThreadCfdCl* pThread ) {

    m_stoppedThreads.removeItem( (ListItem*) pThread );

    if (!m_locked)
    {
        /* directly acquire the resource now */
        m_locked 	= true;
        m_pThread 	= pThread;
        pThread->pBlockedMutex = 0;
        if ( m_pRes != 0) {
        	pThread->getOwner()->aquiredResources.addTail( m_pRes );
            /* reset the file position */
            if (m_pRes->getType() == cFile) ((File*) m_pRes)->resetPosition();
        }

        pThread->unblock();

    } else
    {
#if MEM_NO_FREE
       /* enter thread into scheduler and reuse linkedlistdatabaseitems to avoid memory leaks! */
       if ( m_unusedLinkedListDBItems.isEmpty() ) {
#endif
		  getScheduler()->enter( pThread );
#if MEM_NO_FREE
	  }
	  /* we have an unused LLDB item, so we reuse it instead of creating a new one */
	  else {
		  LinkedListItem* newItem = m_unusedLinkedListDBItems.removeHead();
		  newItem->setData( pThread );
		  getScheduler()->enter( newItem );
	  }
#endif
    }



}

void Mutex::threadRemove(Thread* pThread) {
    getScheduler()->remove(pThread);
}

/*
 * C Wrapper for the Mutex class
 *
 */

extern "C" void* createMutex() {
	return (new Mutex());
}

extern "C" void acquireMutex(void* mutex) {
	Mutex* m = (Mutex*) mutex;
 	m->acquire();
}

extern "C" void releaseMutex(void* mutex) {
	 Mutex* m = (Mutex*) mutex;
	 m->release();
}



