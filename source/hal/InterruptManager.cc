/*
 * InterruptManager.cc
 *
 *  Created on: 10.03.2014
 *    Copyright &  Author: dbaldin
 */

#include "InterruptManager.hh"
#include "inc/error.hh"
#include "inc/memtools.hh"
#include "kernel/Kernel.hh"

#define IRQ_MAX 95

extern Kernel* theOS;
extern unint4 scheduleCount;
extern unint4 rescheduleCount;

typedef struct {
    GenericDeviceDriver*    driver;
    unint4                  priority;
    unint4                  triggerCount;
    unint4                  flags;
    Thread*                 pWThread; /* Thread to execute on IRQ */
} irqHandler;

irqHandler irqTable[IRQ_MAX];

InterruptManager::InterruptManager() :
        CharacterDevice(false, "irq") {
    memset(irqTable, 0, sizeof(irqTable));
}

InterruptManager::~InterruptManager() {
}

/*****************************************************************************
 * Method: InterruptManager::handleIRQ(unint4 irq)
 *
 * @description
 *
 *******************************************************************************/
ErrorT InterruptManager::handleIRQ(unint4 irq) {
    ErrorT ret = cError;

    if (irq >= IRQ_MAX) {
        return (cError);
    }

    TRACE_IRQ_ENTRY(irq);
    irqTable[irq].triggerCount++;

    LOG(HAL, TRACE, "InterruptManager::handleIRQ(): IRQ %d.", irq);
#ifdef ORCOS_SUPPORT_SIGNALS
    /* signal waiting threads */
    theOS->getDispatcher()->signal(reinterpret_cast<void*>(irq << 24), cOk);
#endif


#if USE_WORKERTASK
    /* do we have an IRQ handler for this IRQ number? */
    if (irqTable[irq].driver == 0) {
        LOG(HAL, DEBUG, "InterruptManager::handleIRQ(): unknown IRQ %d.", irq);
        ret = cError;
        goto out;
    }

    /* disable further IRQs on this source and continue executing */
    irqTable[irq].driver->disableIRQ();
    irqTable[irq].driver->clearIRQ();
    theOS->getBoard()->getInterruptController()->clearIRQ(irq);

    /* IRQ handler shall be directly executed? */
    if (irqTable[irq].flags & IRQ_NOTHREAD) {
        irqTable[irq].driver->handleIRQ();
        ret = cOk;
        goto out;
   } else {
       /* IRQ handler still processing?? Would be a bug.*/
        if (irqTable[irq].driver->hasAssignedWorkerThread) {
          LOG(HAL, WARN, "InterruptManager::handleIRQ(): IRQ %d occurred again before workerthread finished.", irq);
          ret = cError;
          goto out;
       }

       irqTable[irq].pWThread->unblock();
       irqTable[irq].driver->hasAssignedWorkerThread   = true;
       irqTable[irq].driver->interruptPending          = true;
       ret = cOk;
    }

#else
    /* directly execute the interrupt handler as we can not schedule it any way*/
    if (irqTable[irq].driver != 0) {
        ErrorT result = irqTable[irq].driver->handleIRQ();
        irqTable[irq].driver->clearIRQ();
        ret = cOk;
    } else {
        LOG(HAL, ERROR, "InterruptManager::handleIRQ(): unknown IRQ %d.", irq);
        ret = cError;
    }
#endif

out:
    TRACE_IRQ_EXIT(irq);
    return (ret);
}

/*****************************************************************************
 * Method: InterruptManager::registerIRQ(unint4 irq, GenericDeviceDriver* driver, unint4 priority)
 *
 * @description
 *
 *******************************************************************************/
ErrorT InterruptManager::registerIRQ(unint4 irq, GenericDeviceDriver* driver, unint4 priority, unint4 flags) {
    if (irq > IRQ_MAX)
        return (cError );

    /* allow overwriting of handlers for irq */
    irqTable[irq].driver        = driver;
    irqTable[irq].priority      = priority;
    irqTable[irq].triggerCount  = 0;
    irqTable[irq].flags         = flags;
    irqTable[irq].driver->hasAssignedWorkerThread = false;

#if USE_WORKERTASK
    if (!(flags & IRQ_NOTHREAD)) {
        /* try to schedule the IRQ using a workerthread*/
        irqTable[irq].pWThread = theOS->getWorkerTask()->addJob(IRQJob, 0, irqTable[irq].driver, irqTable[irq].priority);
        if (!irqTable[irq].pWThread) {
            LOG(HAL, WARN, "InterruptManager::registerIRQ(): No Workerthread available for IRQ %d. Executing directly.", irq);
            /* remove IRQ_NOTHREAD flag as we must execute the irq handler directly */
            irqTable[irq].flags  =  irqTable[irq].flags & ~IRQ_NOTHREAD;
        } else {
            irqTable[irq].pWThread->block();
            irqTable[irq].pWThread->setName(const_cast<char*>(driver->getName()));
        }
    }
#endif

    TRACE_ADD_SOURCE(0, irq, driver->getName());
    return (cOk);
}


/*****************************************************************************
 * Method: InterruptManager::unregisterIRQ(unint4 irq)
 *
 * @description
 *
 *******************************************************************************/
ErrorT InterruptManager::unregisterIRQ(unint4 irq) {
    if (irq > IRQ_MAX)
        return (cError );

    /* remove handler */
    irqTable[irq].driver        = 0;
    irqTable[irq].priority      = 0;
    irqTable[irq].triggerCount  = 0;
    irqTable[irq].flags         = 0;
    irqTable[irq].pWThread      = 0;

    return (cOk);
}

/*****************************************************************************
 * Method: InterruptManager::readBytes(char* bytes, unint4& length)
 *
 * @description
 *
 *******************************************************************************/
ErrorT InterruptManager::readBytes(char* bytes, unint4& length) {
    unint4 len = 0;

    if (len + 33 > length) {
        goto out;
    }

    sprintf(bytes, " IRQ      COUNT  PRIORITY  NAME"LINEFEED);
    len += 33;
    bytes += 33;

    if (len + (16 + 6 + 12) > length) {
        goto out;
    }

    sprintf(bytes, " %3u %10u   HIGHEST  %s"LINEFEED, 0, scheduleCount, "Timer");
    len += 16 + 6 + 12;
    bytes += 16 + 6 + 12;

    if (len + (27 + 22) > length) {
        goto out;
    }

    sprintf(bytes, " %3u %10u   HIGHEST  %s"LINEFEED, 0, rescheduleCount, "Reschedule Interrupts");
    len += 27 + 23;
    bytes += 27 + 23;

    for (int i = 0; i < IRQ_MAX; i++) {
        if (irqTable[i].triggerCount > 0 || irqTable[i].driver != 0) {
            const char* name = "Unknown";
            int namelen = 7;
            if (irqTable[i].driver != 0 && irqTable[i].driver->getName() != 0) {
                name = irqTable[i].driver->getName();
                namelen = strlen(name);
            }

            if (len + (16 + 12 + namelen) > length) {
                goto out;
            }

            sprintf(bytes, " %3d %10u   %7u  %s"LINEFEED, i, irqTable[i].triggerCount, irqTable[i].priority, name);
            len += 16 + namelen + 13;
            bytes += 16 + namelen + 13;
        }
    }

    out: length = len;
    return (cOk );
}

