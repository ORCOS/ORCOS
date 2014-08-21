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
#include "hal/CharacterDevice.hh"
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
class InterruptManager : public CharacterDevice {
public:
    using GenericDeviceDriver::handleIRQ;

    InterruptManager();

    ~InterruptManager();

    ErrorT handleIRQ(unint4 irq);

    ErrorT registerIRQ(unint4 irq, GenericDeviceDriver* driver, unint4 priority);

    /*
     * \brief Returns a human readable string containing information on the irq history
     */
    ErrorT readBytes(char* bytes, unint4& length);
};

#endif /* INTERRUPTMANAGER_HH_ */

