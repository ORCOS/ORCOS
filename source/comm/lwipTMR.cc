/*
 * lwipTMR.cc
 *
 *  Created on: 14.07.2011
 *    Copyright &  Author: dbaldin
 */

#include "lwipTMR.hh"
#include "kernel/Kernel.hh"
#include "lwipopts.h"

extern Kernel* theOS;
extern "C" Mutex* comStackMutex;

extern "C" void tcp_tmr();
extern "C" void ethar_tmr();
#if (LWIP_NETIF_LOOPBACK || LWIP_HAVE_LOOPIF)
extern "C" void netif_poll_all(void);
#endif

static int count;

lwipTMR::lwipTMR() {
    count = 0;
}

lwipTMR::~lwipTMR() {
}

/*****************************************************************************
 * Method: lwipTMR::callbackFunc(void* param)
 *
 * @description
 *  TODO: Rename this to a generic service routine
 * @params
 *
 *******************************************************************************/
void lwipTMR::callbackFunc(void* param) {
    LOG(COMM, TRACE, "Kernel: lwipTMR called!");

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

    /* Perform logger flush */
    theOS->getLogger()->flush();
}
