/*
 * KernelServiceThread.cc
 *
 *  Created on: 22.01.2015
 *    Copyright &  Author: dbaldin
 */

#include <kernel/KernelServiceThread.hh>
#include "kernel/Kernel.hh"
#include "lwipopts.h"

extern Kernel* theOS;
extern "C" Mutex* comStackMutex;

extern "C" void tcp_tmr();
extern "C" void ethar_tmr();

#if (LWIP_NETIF_LOOPBACK || LWIP_HAVE_LOOPIF)
extern "C" void netif_poll_all(void);
#endif


KernelServiceThread::KernelServiceThread() {
    count = 0;
}

KernelServiceThread::~KernelServiceThread() {
}

/*****************************************************************************
 * Method: KernelServiceThread::callbackFunc(void* param)
 *
 * @description
 *  Performs generic service operations that need to be done
 *  periodically. However, it is not necessary that this is done
 *  deterministically. To ensure correct operation this method
 *  must just be called sometimes.
 *
 * @params
 *
 *******************************************************************************/
void KernelServiceThread::callbackFunc(void* param) {
    LOG(KERNEL, TRACE, "KernelServiceThread called!");

#if ENABLE_NETWORKING
    comStackMutex->acquire();

    count++;
#if LWIP_TCP
    tcp_tmr();
#endif
#if (LWIP_NETIF_LOOPBACK || LWIP_HAVE_LOOPIF)
    netif_poll_all();
#endif

    if (count > 20) {
        ethar_tmr();
        count = 0;
    }

    comStackMutex->release();
#endif


    LOG(KERNEL, TRACE, "MemoryManager service!");
    theOS->getMemoryManager()->service();

    /* iterate over all tasks and threads to check for the blocked state */
    LinkedList* llt = theOS->getTaskDatabase();

    for (LinkedListItem* lldi = llt->getHead(); lldi != 0; lldi = lldi->getSucc()) {
        Task* task = static_cast<Task*>(lldi->getData());

        LinkedList* lltask = task->getThreadDB();
        for (LinkedListItem* lldthread = lltask->getHead(); lldthread != 0; lldthread = lldthread->getSucc()) {
            Thread* thread = static_cast<Thread*>(lldthread->getData());
            if (thread->isBlocked() && thread->blockTimeout > 0) {
                if (thread->blockTimeout <= 250 ms) {
                    // unblock the thread
                    thread->blockTimeout = 0;
                    thread->unblock();
                } else {
                    thread->blockTimeout -= 250 ms;
                }
            }
        }
    }

    LOG(KERNEL, TRACE, "Flushing Kernel Log!");
    /* Perform logger flush */
    theOS->getLogger()->flush();
    LOG(KERNEL, TRACE, "Services Thread done!");
}
