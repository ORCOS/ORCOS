/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2010 University of Paderborn

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <kernel/Kernel.hh>
#include "syscalls/handle_syscalls.hh"
#include "inc/error.hh"
#include "OMAP3530.h"
#include "BeagleBoardxM.hh"

#define STACK_CONTENT(sp_int,offset) *( (long*) ( ( (long) sp_int) + offset)  )

extern "C" void handleTimerInterrupt(void* sp_int);

extern Kernel* theOS;
extern Board_TimerCfdCl* theTimer;
extern Task* pCurrentRunningTask;

extern "C" void handleDataAbort(int addr, int instr) {
	LOG(ARCH,ERROR,(ARCH,ERROR,"Data Abort at address: 0x%x , instr: 0x%x",addr,instr));

	int tid = 0;
	if (pCurrentRunningTask != 0)
		tid = pCurrentRunningTask->getId();

	LOG(ARCH,ERROR,(ARCH,ERROR,"TID: %d",tid));

	#ifdef HAS_MemoryManager_HatLayerCfd

		int asid = -1;
		unint4 tbb0 = 0;
		unint4 paget =	((unint4)(&__PageTableSec_start)) + tid*0x4000;

		asm (
			"MRC p15,0,%0,c13,c0,1;" // ; Read CP15 Context ID Register
			"MRC p15,0,%1,c2,c0,0;" // ; Read CP15 Translation Table Base Register 0
			: "=&r" (asid), "=&r" (tbb0)
			:
			:
		);

		LOG(ARCH,ERROR,(ARCH,ERROR,"ASID: %d, TBB0: 0x%x, Task PT: 0x%x",asid,tbb0,paget));

		theOS->getHatLayer()->dumpPageTable(tid);

		// dump TLB
	#endif

	SETPID(0);

	// handle the error
	theOS->getErrorHandler()->handleError();

	while (true) {};
}

extern "C" void handleUndefinedIRQ(int addr, int spsr) {
	LOG(ARCH,ERROR,(ARCH,ERROR,"Undefined IRQ, LR: 0x%x, SPSR: 0x%x",addr,spsr));
	while (true) {};
}

extern "C" void handleFIQ() {
	LOG(ARCH,ERROR,(ARCH,ERROR,"FIQ.."));
	while (true) {};
}

extern "C" void handlePrefetchAbort(int instr) {
	LOG(ARCH,ERROR,(ARCH,ERROR,"Prefetch Abort IRQ. instr: 0x%x",instr));
	while (true) {};
}

extern "C"void dispatchIRQ(void* sp_int, int mode)
{
	if (pCurrentRunningThread != 0){
		ASSERT(isOk(pCurrentRunningThread->pushStackPointer(sp_int)));
		ASSERT(isOk(pCurrentRunningThread->pushStackPointer((void*)mode)));
	}

	int irqSrc;
	//BoardCfdCl* board;
	//board = theOS->getBoard();

	irqSrc = theOS->getBoard()->getInterruptController()->getIRQStatusVector();
	LOG(HAL,TRACE,(HAL,TRACE,"IRQ number: %d, sp_int %x, mode: %d",irqSrc, sp_int, mode));

	// jump to interrupt handler according to active interrupt
	switch (irqSrc)
	    {
			// General Purpose Timer interrupt
			case GPT1_IRQ:
			case GPT2_IRQ:
			{
				handleTimerInterrupt(sp_int);
				break;
			}
			// UART Module 1 interrupt
			case UART1_IRQ:
			{
				LOG(ARCH,INFO,(ARCH,INFO,"UART IRQ: No handler implemented. Returning."));
				break;
			}
			case UART2_IRQ:
			{
				LOG(ARCH,INFO,(ARCH,INFO,"UART 2 IRQ: No handler implemented. Returning."));
				break;
			}
			case UART3_IRQ:
			{
				// the BeagleBoard uses OMAP3530 UART 3 device as serial console
				LOG(ARCH,INFO,(ARCH,INFO,"UART 3 IRQ"));
				#if HAS_Board_UARTCfd
					register CommDeviceDriver* uart = theOS->getBoard()->getUART();
					uart->clearIRQ();
					uart->disableIRQ();
					uart->interruptPending = true;
					uart->recv();
					break;
				#endif
			}
			case EHCI_IRQ:
			{
				((BeagleBoardxM*) theOS->getBoard())->getUSB_HC()->handleInterrupt();
				break;
			}
			default:
			{
				LOG(ARCH,ERROR,(ARCH,ERROR,"Unhandled IRQ Request.. IRQ Number: %d",irqSrc));
				while(true) { }
				break;
			}
	    }


	theOS->getBoard()->getInterruptController()->clearIRQ(irqSrc);

	if (pCurrentRunningThread != 0)
		assembler::restoreContext(pCurrentRunningThread);
	else
		theOS->getCPUDispatcher()->dispatch( theOS->getClock()->getTimeSinceStartup() - lastCycleStamp );

	// we should never get here
	//while(1);
}

extern "C"void dispatchSWI(void* sp_int, int mode)
{

#if ENABLE_NESTED_INTERRUPTS
    // we want nested interrupts so enable interrupts again
    _enableInterrupts();
#endif
	LOG(HAL,TRACE,(HAL,TRACE,"SWI: stack 0x%x mode 0x%x",sp_int,mode));

	ASSERT(isOk(pCurrentRunningThread->pushStackPointer(sp_int)));
	ASSERT(isOk(pCurrentRunningThread->pushStackPointer((void*)mode)));

	handleSyscall( (unint4) sp_int);
}

extern "C" void handleTimerInterrupt(void* sp_int)
{
	ASSERT(theTimer);
	// call Timer
	theTimer->tick();
}
