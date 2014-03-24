/*
 * InterruptManager.cc
 *
 *  Created on: 10.03.2014
 *      Author: dbaldin
 */

#include "InterruptManager.hh"
#include "inc/error.hh"
#include "inc/memtools.hh"
#include "kernel/Kernel.hh"

#define IRQ_MAX 95

extern Kernel* theOS;

typedef struct{
	GenericDeviceDriver* driver;
	unint4 priority;
} irqHandler;

irqHandler irqTable[IRQ_MAX];


InterruptManager::InterruptManager() {
	memset(irqTable,0,sizeof(irqTable));
}

InterruptManager::~InterruptManager() {

}

ErrorT InterruptManager::handleIRQ(unint4 irq) {

	ErrorT ret = cError;

	if (irq > IRQ_MAX) return (cError);
	TRACE_IRQ_ENTRY(irq);

#if USE_WORKERTASK
#ifdef ORCOS_SUPPORT_SIGNALS
	/* schedule the irq using a workerthread */
	theOS->getCPUDispatcher()->signal((void*) (irq << 24),cOk);
#endif
	/* do wa have an irq handler for this irq number? */
	if (irqTable[irq].driver != 0){

		if (irqTable[irq].driver->hasAssignedWorkerThread){
			LOG(HAL,WARN,(HAL,WARN,"InterruptManager::handleIRQ(): IRQ %x occurred again before workerthread finished.",irq));
			ret = cError;
		} else {

			/* try to schedule the irq using a workerthread*/
			WorkerThread* wthread = theOS->getWorkerTask()->addJob(IRQJob,0,irqTable[irq].driver,irqTable[irq].priority);

			if (wthread == 0) {
			 	/* on error log it and execute the irq handler directly..
				 * this way the system stays functional, however the realtime
				 * capabilities will be seriously reduced..
				 * */
				LOG(HAL,DEBUG,(HAL,DEBUG,"InterruptManager::handleIRQ(): could not schedule IRQ %x Executing directly.",irq));
				irqTable[irq].driver->handleIRQ();
				irqTable[irq].driver->hasAssignedWorkerThread = false;
				irqTable[irq].driver->interruptPending = false;
			} else {

				/* success the irq handler will be executed inside a workerthread */
				irqTable[irq].driver->hasAssignedWorkerThread = true;
				irqTable[irq].driver->interruptPending = true;
				/* disable further irqs on this source and continue executing */
				irqTable[irq].driver->disableIRQ();

			}
			/* finally clear and ack the IRQ as it will be handled either way. */
			irqTable[irq].driver->clearIRQ();
			/* also clear the IRQ at the interrupt controller */
			theOS->getBoard()->getInterruptController()->clearIRQ(irq);

		}
		ret = cOk;
	} else {
		LOG(HAL,ERROR,(HAL,ERROR,"InterruptManager::handleIRQ(): unknown IRQ %x.",irq));
	}
#else
	// directly execute the interrupt handler
	if (irqTable[irq].driver != 0){
		ErrorT result = irqTable[irq].driver->handleIRQ();
		irqTable[irq].driver->clearIRQ();
		theOS->getBoard()->getInterruptController()->clearIRQ(irq);
		return (result);
	} else {
		LOG(HAL,ERROR,(HAL,ERROR,"InterruptManager::handleIRQ(): unknown IRQ %x.",irq));
	}
#endif

	TRACE_IRQ_EXIT(irq);
	return (ret);
}

ErrorT InterruptManager::registerIRQ(unint4 irq, GenericDeviceDriver* driver, unint4 priority) {

	if (irq > IRQ_MAX) return (cError);

	// allow overwriting of handlers for irq
	irqTable[irq].driver   = driver;
	irqTable[irq].priority = priority;

	TRACE_ADD_SOURCE(0,irq,driver->getName());

	return (cError);
}
