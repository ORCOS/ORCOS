#include "BeagleBoardInterruptController.hh"
#include "assemblerFunctions.hh"
#include "OMAP3530.h"
#include "inc/memio.h"

BeagleBoardInterruptController::BeagleBoardInterruptController() {
}

BeagleBoardInterruptController::~BeagleBoardInterruptController() {
}

int BeagleBoardInterruptController::getIRQStatusVector() {

	int irqSrc = 100; //initialize with invalid interrupt number (only valid: 0 to 95)

	irqSrc = INW(MPU_INTCPS_SIR_IRQ) & 0x7f;

    return (irqSrc);
}

void BeagleBoardInterruptController::clearIRQ( int num ) {

	// allow new interrupts MPU_INTCPS_CONTROL
	OUTW(MPU_INTCPS_CONTROL, 0x1);
}

void BeagleBoardInterruptController::enableIRQs() {
	_enableInterrupts();
}
