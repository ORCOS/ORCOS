#include "OPB_Interrupt_Controller.hh"
#include "inc/memio.h"

OPB_Interrupt_Controller::OPB_Interrupt_Controller() {
}

OPB_Interrupt_Controller::~OPB_Interrupt_Controller() {
}

int OPB_Interrupt_Controller::getIRQStatusVector() {
    int reg = INW(OPB_INTC_BASE + OPB_INTC_ISR_OFFSET);
    return reg;
}

void OPB_Interrupt_Controller::clearIRQ( int num ) {
    int clearval = 0;
    clearval |= num;
    OUTW(OPB_INTC_BASE + OPB_INTC_IAR_OFFSET,clearval);
}

void OPB_Interrupt_Controller::enableIRQs() {
    // unmask all interrupts
    OUTW(OPB_INTC_BASE + OPB_INTC_IER_OFFSET,0xFFFFFFFF);
    // enable interrupt requests of the interrupt controller
    OUTW(OPB_INTC_BASE + OPB_INTC_MER_OFFSET,0xFFFFFFFF);
}
