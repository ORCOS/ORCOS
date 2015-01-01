/*
 *
 *   Copyright &  Author: dbaldin
 */

#include "BeagleBoardInterruptController.hh"
#include "assemblerFunctions.hh"
#include "OMAP3530.h"
#include "inc/memio.h"

BeagleBoardInterruptController::BeagleBoardInterruptController() {
    OUTW(MPU_INTCPS_SYSCONFIG, 0x1);
}

BeagleBoardInterruptController::~BeagleBoardInterruptController() {
}

/*****************************************************************************
 * Method: BeagleBoardInterruptController::unmaskIRQ(unint4 irq_number)
 *
 * @description
 *  Unmask the spcific IRQ which allows the IRQ to be raised
 *******************************************************************************/
ErrorT BeagleBoardInterruptController::unmaskIRQ(unint4 irq_number) {
    if (irq_number < 96) {
        /* enable interrupt: (int no. irq_number: (irq_number mod 32)): position of bit */
        int reg = irq_number / 32;
        int bit = (irq_number - reg * 32);
        OUTW(MPU_INTCPS_MIR_CLEAR(reg), (1 << bit));
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
ErrorT BeagleBoardInterruptController::maskIRQ(unint4 irq_number) {
    if (irq_number < 96) {
        /* enable interrupt: (int no. irq_number: (irq_number mod 32)): position of bit */
        int reg = irq_number / 32;
        int bit = (irq_number - reg * 32);
        OUTW(MPU_INTCPS_MIR_SET(reg), (1 << bit));
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
ErrorT BeagleBoardInterruptController::setIRQPriority(unint4 irq_number, unint4 priority) {
    if (irq_number < 96) {
        /* range check */
        if (priority > 63)
            priority = 63;

        /* invert as this is programmed into hardware */
        int prioval = 63 - priority;
        prioval = (prioval << 2) & 0xFC;

        OUTW(MPU_INTCPS_ILR(irq_number), ((INW(MPU_INTCPS_ILR(irq_number)) & ~0xFC) | prioval));   // priority 0 (highest)
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
void BeagleBoardInterruptController::enableIRQs() {
   // _enableInterrupts();
}
