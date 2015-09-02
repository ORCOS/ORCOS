/*
 * InterruptManager.hh
 *
 *  Created on: 10.03.2014
 *    Copyright &  Author: dbaldin
 */

#ifndef INTERRUPTMANAGER_HH_
#define INTERRUPTMANAGER_HH_

#include "inc/types.hh"
#include "hal/GenericDeviceDriver.hh"
#include "hal/CharacterDevice.hh"


 /* Forces the interrupt handler to be executed directly inside the interrupt context. The IRQ
  * is not scheduled using a workerthread. Usefull to ensure very low latencies for interrupt handlers.
  * However, increases latency for other IRQS such as the timer IRQ.
  * */
#define IRQ_NOTHREAD    (1 << 1)

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

    /*****************************************************************************
     * Method: handleIRQ(unint4 irq)
     *
     * @description
     *
     *******************************************************************************/
    ErrorT handleInterruptIRQ(unint4 irq);

    /*****************************************************************************
     * Method: registerIRQ(unint4 irq, GenericDeviceDriver* driver, unint4 priority)
     *
     * @description
     *  Register an interrupt handler for a given IRQ number.
     *  The driver->handleInterrupt method will be called whenever
     *  the given IRQ is raised. If the IRQ is to be scheduled using workerthreads
     *  (IRQ_NOTHREAD flag is NOT set) the priority will be used to schedule
     *  the interupt handler thread. If IRQ_NOTHREAD is set inside the flags bitmap
     *  the interrupt handler will directory be executed inside the interrupt context
     *  which assures the fastest IRQ latency, however may intrdoce additional latencies
     *  to other IRQs.
     *
     *  If a handler is already registered for this IRQ this method will overwrite the
     *  registered handler.
     *
     * @params
     *  irq     :   The IRQ number to be handled
     *  driver  :   The driver object which handles the IRQ
     *  priority:   IRQ Priority in case of a scheduled IRQ handler
     *  flags   :   Bitmap containing flags:
     *                  IRQ_NOTHREAD: Do not use a thread to execute the IRQ handler.
     *******************************************************************************/
    ErrorT registerIRQ(unint4 irq, GenericDeviceDriver* driver, unint4 priority, unint4 flags = 0);


    ErrorT waitIRQ(unint4 irq, Thread* t);

    /*****************************************************************************
     * Method: unregisterIRQ(unint4 irq)
     *
     * @description
     *  Unregisters the IRQ handler for the given IRQ.
     *******************************************************************************/
    ErrorT   unregisterIRQ(unint4 irq);

    /*****************************************************************************
     * Method: readBytes(char* bytes, unint4& length)
     *
     * @description
     *  Returns a human readable string containing information on the irq history
     *******************************************************************************/
    ErrorT readBytes(char* bytes, unint4& length);
};

#endif /* INTERRUPTMANAGER_HH_ */

