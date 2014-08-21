/*
 * lwipTMR.cc
 *
 *  Created on: 14.07.2011
 *      Author: dbaldin
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

    if (count > 20)
    {
        ethar_tmr();
        count = 0;
    }

    comStackMutex->release();

    theOS->getMemoryManager()->service();

    /* iterate over all tasks and threads to check for the blocked state */
    LinkedList* llt = theOS->getTaskDatabase();

    for (LinkedListItem* lldi = llt->getHead(); lldi != 0; lldi = lldi->getSucc())
    {
        Task* task = (Task*) lldi->getData();

        LinkedList* lltask = task->getThreadDB();
        for (LinkedListItem* lldthread = lltask->getHead(); lldthread != 0; lldthread = lldthread->getSucc())
        {
            Thread* thread = (Thread*) lldthread->getData();
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

}
