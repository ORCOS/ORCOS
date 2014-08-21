/*
 * handleIRQ.c
 *
 *  Created on: 06.04.2014
 *      Author: Daniel
 */

#include "kernel/Kernel.hh"

extern Kernel* theOS;

extern "C" void handleEXT_INT0() {
    LOG(ARCH,INFO,"EXTINT0");
}

extern "C" void handleEXT_INT1() {
    LOG(ARCH,INFO,"EXTINT1");
}

extern "C"  void handleWatchdog() {
    LOG(ARCH,INFO,"Watchdog");
}


extern "C" void handleTIM0() {
    //LOG(ARCH,INFO,"TIM0 irq");
    theOS->getTimerDevice()->tick();
}

extern "C" void handleTIM1() {
    LOG(ARCH,INFO,"TIM1 irq");
}

extern "C" void invalidIRQ() {
    LOG(ARCH,INFO,"invalid IRQ.");
}

extern "C" void handleTIM2() {
    LOG(ARCH,INFO,"woke up..");
	theOS->getTimerDevice()->tick();
}
