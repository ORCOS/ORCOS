/*
 * syscalls.cc
 *
 *  Created on: 07.04.2014
 *      Author: Daniel
 */

#include "syscalls/handle_syscalls.hh"
#include "assemblerFunctions.hh"
#include "avr/io.h"
#include "inc/memio.h"
#include <kernel/Kernel.hh>
#include "avr/common.h"

extern "C" int fopen(const char* filename, int blocking)
{
	sc_param1 = (unint4) filename;
	sc_param2 = blocking;
	fopenSyscall(0);
	return (sc_return_value);
}

extern "C"int fputc(short c, int stream) {
	sc_param1 = (unint4) c;
	sc_param2 = stream;
	fputcSyscall(0);
	return (sc_return_value);
}

extern "C"int fgetc(int stream) {
	sc_param1 = (unint4) stream;
	fgetcSyscall(0);
	return (sc_return_value);
}


extern "C" void sleep(int millis)
{
     // give system to perform spi and uart transfer on current clock before slowing down!
    for (volatile int i = 0; i < 100; i++);

    CLKPR = 128; // allow clock change!
    CLKPR = 7;   // divide by 128!

    theOS->getTimerDevice()->setTimer(millis MILLISECONDS);
	_enableInterrupts();

	SMCR = 7; // power save mode and allow sleep

	// sleep will set enter sleep mode
	// on interrupt (timer2) the cpu will store the return address on stack
	// enter the interrupt handler
	// the return instruction will return to the instruction after sleep
	asm volatile(
	   "sleep; "
		: // no output variables
		:
		:
	);

	_disableInterrupts();

    CLKPR = 128; // allow clock change!
    CLKPR = 0;   // divide by 2!

    // wait until clock frequency is settled
    for (volatile int i = 0; i < 100; i++);
    TCCR2B = 0;   // divisor = 1024



}

