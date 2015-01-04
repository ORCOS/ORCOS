/*
 * UIC_Interrupt_Controller.cc
 *
 *  Created on: 19.01.2012
 *      Author: kgilles
 */

#include "UIC_Interrupt_Controller.hh"
#include <assembler.h>
#include "inc/memio.h"

UIC_Interrupt_Controller::UIC_Interrupt_Controller() {
}

UIC_Interrupt_Controller::~UIC_Interrupt_Controller() {
}

int UIC_Interrupt_Controller::getIRQStatusVector() {
    // get masked status register
    int reg = INW(UIC_INTC_BASE + UIC_MSR_OFFSET);
    return reg;
}

void UIC_Interrupt_Controller::clearIRQ(int num) {
    int clearval = 0;
    clearval |= num;
    OUTW(UIC_INTC_BASE + UIC_SR_OFFSET, clearval);
}

void UIC_Interrupt_Controller::enableIRQs() {
    // enable all interrupt requests of the interrupt controller
    OUTW(UIC_INTC_BASE + UIC_ER_OFFSET, 0xFFFFFFFF);
}
