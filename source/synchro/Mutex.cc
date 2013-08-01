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
extern ThreadCfdCl*    pCurrentRunningThread;
extern Task*           pCurrentRunningTask;

Mutex::Mutex() :
    m_locked( false ), m_pThread( 0 ), m_stoppedThreads(5), m_pRes(0) {
#ifdef HAS_Mutex_SchedulerCfd
    SchedulerCfd = new Mutex_SchedulerCfdCl( );
#endif
}

Mutex::~Mutex() {
#ifdef HAS_Mutex_SchedulerCfd
    delete SchedulerCfd;
#endif
}

ErrorT Mutex::acquire( Resource* pRes, bool blocking ) {
    ThreadCfdCl* pCallingThread = pCurrentRunningThread;

#if ENABLE_NESTED_INTERRUPTS
    bool int_enabled;
    GET_INTERRUPT_ENABLE_BIT(int_enabled);
    _disableInterrupts();
#endif

    if (m_locked == false)
    {
        m_locked = true;
        // successfully aquired mutex
        m_pThread = pCallingThread;
        m_pRes = pRes;
        if ( m_pRes != 0) pCurrentRunningTask->aquiredResources.addTail( pRes );

        // reset the file position
        if (m_pRes->getType() == cFile) ((File*) m_pRes)->resetPosition();

        #if ENABLE_NESTED_INTERRUPTS
            if ( int_enabled )  _enableInterrupts();
        #endif

        return cOk;
    }
    else
    {
      if (!blocking) return cError;
      // mutex already aquired by some other thread!
    #ifdef HAS_Mutex_SchedulerCfd
        // Implementation of the Priority Inheritance Protocol (PIP)
        // See if the newly arrived Thread has a higher priority than the thread currently locking the semaphore.
        // If this is the case, set the priority of the currently owning Thread to that of the newly arrived one.
        // Save the original priority of the owner Thread so it can be set back afterwards.
        #ifdef HAS_PRIORITY
        #if USE_PIP
            if ( pCallingThread != 0 && m_pThread != 0 && m_pThread->effectivePriority
                    < pCallingThread->effectivePriority ) {
                m_pThread->effectivePriority = pCallingThread->effectivePriority ;
            }
        #endif
        #endif

        // enter thread into scheduler and reuse linkedlistdatabaseitems to avoid memory leaks!
        if ( m_unusedLinkedListDBItems.isEmpty() ) {
                   getScheduler()->enter( pCallingThread );
               }
               // we have an unused LLDB item, so we reuse it instead of creating a new one
               else {
                   LinkedListDatabaseItem* newItem = m_unusedLinkedListDBItems.removeHead();
                   newItem->setData( pCallingThread );
                   getScheduler()->enter( newItem );
               }

        // now block the calling thread!
        // non returning function call!
        pCallingThread->pBlockedMutex = this;
        pCallingThread->block();
        // TODO: how to handle unblocked threads if this mutex is deleted?
        // they will get here and reference invalid memory locations!

        return cOk;
    #else
        return cError;
    #endif

    }

   return cError;
}


ErrorT Mutex::release( ) {

#if ENABLE_NESTED_INTERRUPTS
    bool int_enabled;
    GET_INTERRUPT_ENABLE_BIT(int_enabled);
    _disableInterrupts();
#endif

    // no need to check this .. handle syscall already checked it!
//    if ( pCallingThread->getOwner() != m_pThread->getOwner() )
//           return cError;

      // remove the resource from the acquired list of the owner task
      if ( m_pRes != 0 )
          pCurrentRunningTask->aquiredResources.removeItem( m_pRes );

       LOG(SYNCHRO,TRACE,(SYNCHRO,TRACE,"Task released resource."));


#ifdef HAS_PRIORITY
#if USE_PIP
      // reset the priority of the currentRunning thread
      pCallingThread->effectivePriority = pCallingThread->initialPriority;
#endif
#endif

#ifdef HAS_Mutex_SchedulerCfd
      // get the next thread by the scheduler who is next to aquire the mutex
      LinkedListDatabaseItem* next = (LinkedListDatabaseItem*) getScheduler()->getNext();
      ThreadCfdCl* pSchedulerThread;
      // search the first not stopped thread
      while (next != 0) {
          pSchedulerThread = (ThreadCfdCl*) next->getData();
          if ( pSchedulerThread->isStopped() ) m_stoppedThreads.addTail( pSchedulerThread );
          else break;
          next = (LinkedListDatabaseItem*) getScheduler()->getNext();
      }
      if (next != 0)
      {
          // available thread found
          m_unusedLinkedListDBItems.addHead(next);
          m_pThread = pSchedulerThread;
          pSchedulerThread->pBlockedMutex = 0;
          if ( m_pRes != 0) pSchedulerThread->getOwner()->aquiredResources.addTail( m_pRes );
          // reset the file position
          if (m_pRes->getType() == cFile) ((File*) m_pRes)->resetPosition();

          pSchedulerThread->unblock();

      } else
#endif
      {
          // no available thread to aquire the mutex
          m_locked = false;
          m_pThread = 0;
          m_pRes = 0;
          return cOk;
      }


#if ENABLE_NESTED_INTERRUPTS
    if ( int_enabled )  _enableInterrupts();
#endif
    return cOk;

}

void Mutex::threadResume( ThreadCfdCl* pThread ) {

    m_stoppedThreads.removeItem( (DatabaseItem*) pThread );

    if (!m_locked)
    {
        // directly aquire the resource now!
        m_locked = true;
        m_pThread = pThread;
        pThread->pBlockedMutex = 0;
        if ( m_pRes != 0) pThread->getOwner()->aquiredResources.addTail( m_pRes );

        // reset the file position
        if (m_pRes->getType() == cFile) ((File*) m_pRes)->resetPosition();

        pThread->unblock();

    } else
    {
        // enter thread into scheduler and reuse linkedlistdatabaseitems to avoid memory leaks!
       if ( m_unusedLinkedListDBItems.isEmpty() ) {
                  getScheduler()->enter( pThread );
              }
              // we have an unused LLDB item, so we reuse it instead of creating a new one
              else {
                  LinkedListDatabaseItem* newItem = m_unusedLinkedListDBItems.removeHead();
                  newItem->setData( pThread );
                  getScheduler()->enter( newItem );
              }
    }


}

