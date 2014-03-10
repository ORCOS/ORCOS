/*
 * InterruptManager.hh
 *
 *  Created on: 10.03.2014
 *      Author: dbaldin
 */

#ifndef INTERRUPTMANAGER_HH_
#define INTERRUPTMANAGER_HH_

#include "inc/types.hh"
#include "hal/GenericDeviceDriver.hh"

/**
 * The Interrupt Manager class provides the functionality
 * for device drivers to register interrupt handler for global interrupt
 * request numbers.
 *
 * Depending on the configuration of the kernel the interrupt dispatch automatically
 * dispatches the IRQ to the handler directly or scheduled using a workerthread.
 * Additionally the manager provides the functionality to allow user threads to wait for IRQ request to
 * happen.
 *
 */
class InterruptManager {
public:
	InterruptManager();

	virtual ~InterruptManager();

	ErrorT handleIRQ(unint4 irq);

	ErrorT registerIRQ(unint4 irq, GenericDeviceDriver* driver, unint4 priority);

};

#endif /* INTERRUPTMANAGER_HH_ */
