/*
 * KernelServiceThread.cc
 *
 *  Created on: 22.01.2015
 *    Copyright &  Author: dbaldin
 */

#include <kernel/KernelServiceThread.hh>
#include "kernel/Kernel.hh"
#include "lwipopts.h"
#include "assemblerFunctions.hh"
#include "process/RealTimeThread.hh"

extern Kernel* theOS;
extern "C" Mutex* comStackMutex;

extern "C" void tcp_tmr();
extern "C" void ethar_tmr();
extern "C" void dhcp_fine_tmr();
extern "C" void dns_tmr();

#if (LWIP_NETIF_LOOPBACK || LWIP_HAVE_LOOPIF)
extern "C" void netif_poll_all(void);
#endif


KernelServiceThread::KernelServiceThread() {
    count = 0;
}

KernelServiceThread::~KernelServiceThread() {
}

const char* states[8] = {
        "NEW", // new
        "READY", // ready
        "BLOCKED", // blocked
        "TERM", // terminated
        "RES", // resource waiting
        "STOPPED", //stopped
        "SIGNAL", // signal waiting
        "DOTERM", // goint to terminate
};

const char* getStatusStr(unint4 status) {
    char* ret = "SLEEPING";
    for (int i = 0; i < 8; i++) {
        if (status & (1 << i))
            return (states[i]);
    }
    return (ret);
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

    /* count increases every 250 ms */
    count++;
#if LWIP_TCP
    /* call every 250 ms*/
    tcp_tmr();
#endif
#if (LWIP_NETIF_LOOPBACK || LWIP_HAVE_LOOPIF)
    netif_poll_all();
#endif

    if (count >= 16) {
        /* call every 4 s*/
        ethar_tmr();
        count = 0;
    }

#if LWIP_DHCP
    if (count & 1) {
        /* call every 500 ms*/
        dhcp_fine_tmr();
    }
#endif

#if LWIP_DNS
    /* call every 1 s*/
    if (count & 4)
        dns_tmr();
#endif

    comStackMutex->release();
#endif // ENABLE_NETWORKING


#ifdef HEARTBEAT
    if (count & 1) {
        /* heartbeat led */
        int value;
        unint4 length = 4;

        CharacterDevice* hbdev = (CharacterDevice*) theOS->getFileManager()->getResourceByNameandType(HEARTBEAT_DEV, cStreamDevice);
        if (hbdev) {
            hbdev->readBytes((char*) &value, length);
            value = value ^ (HEARTBEAT_VALUE);
            hbdev->writeBytes((char*) &value, 4);
        }
    }
#endif

    LOG(KERNEL, TRACE, "MemoryManager service!");
    theOS->getMemoryManager()->service();

    /* iterate over all tasks and threads to check for the blocked state */
    LinkedList* llt = theOS->getTaskDatabase();

    DISABLE_IRQS(irqstatus);

    // TODO this is not safe against concurrent modification!
    for (LinkedListItem* lldi = llt->getHead(); lldi != 0; lldi = lldi->getSucc()) {
        Task* task = static_cast<Task*>(lldi->getData());

        LinkedList* lltask = task->getThreadDB();
        for (LinkedListItem* lldthread = lltask->getHead(); lldthread != 0; lldthread = lldthread->getSucc()) {
            RealTimeThread* thread = static_cast<RealTimeThread*>(lldthread->getData());

/*            const char* statusStr = getStatusStr(thread->getStatus());

            printf("%2u %5u %8s\t%10u\t {%s}"LINEFEED
                   , task->getId(),thread->getId(),statusStr,(unint4)thread->effectivePriority,thread->getName());*/

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
    RESTORE_IRQS(irqstatus);

    LOG(KERNEL, TRACE, "Flushing Kernel Log!");
    /* Perform logger flush */
    theOS->getLogger()->flush();
    LOG(KERNEL, TRACE, "Services Thread done!");
}
