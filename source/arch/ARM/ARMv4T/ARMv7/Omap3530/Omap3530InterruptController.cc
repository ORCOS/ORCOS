/*
 *
 *   Copyright &  Author: dbaldin
 */

#include "Omap3530InterruptController.hh"
#include "assemblerFunctions.hh"
#include "OMAP3530.h"
#include "inc/memio.h"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

Omap3530InterruptController::Omap3530InterruptController(T_Omap3530InterruptController_Init* init) {
    baseAddr = init->Address;
    OUTW(baseAddr + INTCPS_SYSCONFIG, 0x0);

    int revision = INW(baseAddr + INTCPS_REVISION);
    LOG(ARCH,INFO,"InterruptController Revision %u.%u", revision >> 4, revision & 0xf)
}

Omap3530InterruptController::~Omap3530InterruptController() {
}

/*****************************************************************************
 * Method: BeagleBoardInterruptController::unmaskIRQ(unint4 irq_number)
 *
 * @description
 *  Unmask the spcific IRQ which allows the IRQ to be raised
 *******************************************************************************/
ErrorT Omap3530InterruptController::unmaskIRQ(unint4 irq_number) {
    if (irq_number < 96) {
        /* enable interrupt: (int no. irq_number: (irq_number mod 32)): position of bit */
        int reg = irq_number / 32;
        int bit = (irq_number - reg * 32);
        OUTW(baseAddr + INTCPS_MIR_CLEAR(reg), (1 << bit));
        return (cOk);
    }
    return (cError );
}

/*****************************************************************************
 * Method: BeagleBoardInterruptController::maskIRQ(unint4 irq_number)
 *
 * @description
 *  Masks the spcific IRQ which prevents the IRQ from being raised
 *******************************************************************************/
ErrorT Omap3530InterruptController::maskIRQ(unint4 irq_number) {
    if (irq_number < 96) {
        /* enable interrupt: (int no. irq_number: (irq_number mod 32)): position of bit */
        int reg = irq_number / 32;
        int bit = (irq_number - reg * 32);
        OUTW(baseAddr + INTCPS_MIR_SET(reg), (1 << bit));
        return (cOk);
    }
    return (cError );
}

ErrorT Omap3530InterruptController::raiseIRQ(unint4 irq_number) {
    if (irq_number < 96) {
        /* enable interrupt: (int no. irq_number: (irq_number mod 32)): position of bit */
        int reg = irq_number / 32;
        int bit = (irq_number - reg * 32);
        OUTW(baseAddr + INTCPS_ISR_SET(reg), (1 << bit));
        OUTW(baseAddr + INTCPS_ISR_CLEAR(reg), (1 << bit));
        return (cOk);
    }
    return (cError );
}

/*****************************************************************************
 * Method: BeagleBoardInterruptController::setIRQPriority(unint4 irq_number, unint4 priority)
 *
 * @description
 * Sets the hardware priority of the IRQ source with priority 63 being the highest
 *  priority. 0 beeing the lowest priority.
 *******************************************************************************/
ErrorT Omap3530InterruptController::setIRQPriority(unint4 irq_number, unint4 priority) {
    if (irq_number < 96) {
        /* range check */
        if (priority > 63)
            priority = 63;

        /* invert as this is programmed into hardware */
        int prioval = 63 - priority;
        prioval = (prioval << 2) & 0xFC;

        OUTW(baseAddr + INTCPS_ILR(irq_number), ((INW(baseAddr + INTCPS_ILR(irq_number)) & ~0xFC) | prioval));   // priority 0 (highest)
        return (cOk );
    }

    return (cError );
}

/*****************************************************************************
 * Method: BeagleBoardInterruptController::enableIRQs()
 *
 * @description
 *
 *******************************************************************************/
void Omap3530InterruptController::enableIRQs() {
   // _enableInterrupts();
}
