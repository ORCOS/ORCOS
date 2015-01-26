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
#include "assemblerFunctions.hh"

extern Kernel* theOS;
extern Kernel_ThreadCfdCl* pCurrentRunningThread;
extern Task* pCurrentRunningTask;

#ifndef USE_PIP
/* If not configured we use PIP */
#define USE_PIP 1
#endif

extern unint4 rescheduleCount;

/*****************************************************************************
 * Method: Mutex::Mutex()
 *
 * @description
 *   Mutex Constructor
 *
 * @params
 *
 * @returns
 *---------------------------------------------------------------------------*/
Mutex::Mutex() :
        m_locked(false),
        m_pThread(0),
        m_pRes(0),
        waitingThreads(0) {
    acquirePriority = 1;
}

/*****************************************************************************
 * Method: Mutex::~Mutex()
 *
 * @description
 *   Mutex Destructor
 *
 * @params
 *
 * @returns
 *---------------------------------------------------------------------------*/
Mutex::~Mutex() {
    // TODO: waiting threads?? should not happen
}

/*****************************************************************************
 * Method: Mutex::acquire( Resource* pRes, bool blocking )
 *
 * @description
 *  Tries to acquire the resource pRes using this mutex. If blocking is true
 *  and the resource is already owned the calling thread is blocked.
 *
 * @params
 *
 * @returns
 *---------------------------------------------------------------------------*/
ErrorT Mutex::acquire(Resource* pRes, bool blocking) {
    Kernel_ThreadCfdCl* pCallingThread = pCurrentRunningThread;

    DISABLE_IRQS(irqstatus);
    waitingThreads++;
    do  {
        _disableInterrupts();
        /* spinning lock acquisition!
         * if we can not get the lock dispatch and give the running thread the higher priority!
         * The OS scheduler then will take care of the order the threads spinning
         * here will get the lock */
        if (m_locked == 0) {
            waitingThreads--;

            /* successfully acquired mutex */
            if (pCallingThread != 0)
                acquirePriority = pCallingThread->effectivePriority;

            m_locked        = 1;
            m_pThread       = pCallingThread;
            m_pRes          = pRes;

            if (pCallingThread) {
                LOG(SYNCHRO, DEBUG, "Mutex::acquire() Thread %d acquired Mutex %x", pCallingThread->getId(), this);
            } else {
                LOG(SYNCHRO, DEBUG, "Mutex::acquire() Thread acquired Mutex %x", this);
            }

            RESTORE_IRQS(irqstatus);
            return (cOk );
        }

        if (pCallingThread != 0 && m_pThread != 0 &&
           m_pThread->effectivePriority <= pCallingThread->effectivePriority) {
           /* boost priority */
           m_pThread->effectivePriority = pCallingThread->effectivePriority + 1;
           m_pThread->getLinkedListItem()->remove();
           theOS->getCPUScheduler()->enter(m_pThread->getLinkedListItem());
        }

        /* in blocking mode we must dispatch here!
         * Do this by raising a timer IRQ to reschedule */
        if (blocking) {
            LOG(SYNCHRO, DEBUG, "Mutex::acquire() Thread %d blocked on Mutex %x", pCallingThread->getId(), this);
            rescheduleCount++;
            theOS->getTimerDevice()->setTimer(1);
        }

        _enableInterrupts();
        /* at this point we will be interrupted. */

        NOP;
    } while (blocking);

    RESTORE_IRQS(irqstatus);
    return (cError);
}

/*****************************************************************************
 * Method: Mutex::release(Thread* pThread)
 *
 * @description
 *
 *
 * @params
 *
 * @returns
 *---------------------------------------------------------------------------*/
ErrorT Mutex::release(Thread* pThread) {
    DISABLE_IRQS(irqstatus);

    /* no available thread to acquire the mutex */
      m_locked    = 0;
      m_pThread   = 0;
      m_pRes      = 0;

    LOG(SYNCHRO, DEBUG, "Mutex 0x%x released", this);

    Kernel_ThreadCfdCl* pCallingThread = static_cast<Kernel_ThreadCfdCl*>(pThread);
    /* reset the priority of the currentRunning thread as it might have been boosted by
     * higher priority waiting threads  */
    if (pCallingThread != 0) {
        pCallingThread->effectivePriority = acquirePriority;
    }

    RESTORE_IRQS(irqstatus);

    return (cOk );
}


/*
 * C Wrapper for the Mutex class
 *
 */

extern "C" void* createMutex() {
    return (new Mutex());
}

extern "C" void acquireMutex(void* mutex) {
    Mutex* m = reinterpret_cast<Mutex*>(mutex);
    m->acquire();
}

extern "C" void releaseMutex(void* mutex) {
    Mutex* m = reinterpret_cast<Mutex*>(mutex);
    m->release();
}

