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

	// read active interrupt number from register
	// register INTCPS_SIR_IRQ, address 0x4820 0040
	// active interrupt: bits 0 to 6
	// mask: 0x7F
/*	asm volatile(
		"LDR r1, [%1];"
		"AND %0, r1, #0x7F"
		: "=&r" (irqSrc)
		: "r" (MPU_INTCPS_SIR_IRQ)
		: "r0", "r1"
	);*/

	irqSrc = INW(MPU_INTCPS_SIR_IRQ) & 0x7f;

    return irqSrc;
}

void BeagleBoardInterruptController::clearIRQ( int num ) {

	// allow new interrupts MPU_INTCPS_CONTROL
	OUTW(MPU_INTCPS_CONTROL, 0x1);
}

void BeagleBoardInterruptController::enableIRQs() {
	_enableInterrupts();
}
