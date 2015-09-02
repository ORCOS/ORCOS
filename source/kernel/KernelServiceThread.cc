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
    if ((count & 7) == 0)
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

    /* Perform unblock on timeout operation */
    LinkedList* llt = theOS->getDispatcher()->getBlockedlist();
    llt->lock();
    for (LinkedListItem* lldi = llt->getHead(); lldi != 0; lldi = lldi->getSucc()) {
        Thread* thread = static_cast<Thread*>(lldi->getData());
        if (thread->isBlocked() && thread->blockTimeout > 0) {
            if (thread->blockTimeout <= 250 ms) {
                /* unblock the thread */
                thread->blockTimeout = 0;
                thread->unblock();
            } else {
                thread->blockTimeout -= 250 ms;
            }
        }
    }
    llt->unlock();

    LOG(KERNEL, TRACE, "Flushing Kernel Log!");
    /* Perform logger flush */
    theOS->getLogger()->flush();
    LOG(KERNEL, TRACE, "Services Thread done!");
}
