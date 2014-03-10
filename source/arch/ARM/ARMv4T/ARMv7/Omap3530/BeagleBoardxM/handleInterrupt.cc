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


extern "C" void dumpContext(void* sp_context) {

	printf("Context at 0x%x:\r", (unint4) sp_context);
	printf("PSR: 0x%x\r", ((unint4*) sp_context)[0]);

	for (int i = 1; i < 13; i++) {
		printf("r%d : %x\r",i-1,((unint4*) sp_context)[i]);
	}

	printf("PC: 0x%x\r", ((unint4*) sp_context)[14]);
	printf("LR: 0x%x\r", ((unint4*) sp_context)[15]);
	printf("SP: 0x%x\r", ((unint4*) sp_context)[16]);
	printf("FP: 0x%x\r", ((unint4*) sp_context)[17]);

}


extern "C" void handleDataAbort(int addr, int instr, int context, int spsr) {
	LOG(ARCH,ERROR,(ARCH,ERROR,"Data Abort at address: 0x%x , instr: 0x%x, SPSR: 0x%x",addr,instr,spsr));

	for (int i = -1; i <= 13; i++) {
			LOG(ARCH,ERROR,(ARCH,ERROR,"r%d : %x",i,((unint4*) context)[i]));
		}

	//memdump(instr-16,8);

	int tid = 0;
	if (pCurrentRunningTask != 0)
		tid = pCurrentRunningTask->getId();


	int dfsr;
	asm (
		"MRC p15,0,%0,c5,c0,0;"
		: "=&r" (dfsr)
		:
		:
		);


	LOG(ARCH,ERROR,(ARCH,ERROR,"TID: %d, DFSR: %x",tid, dfsr));

	#ifdef HAS_Board_HatLayerCfd

		int asid = -1;
		unint4 tbb0 = 0;
		unint4 paget =	((unint4)(&__PageTableSec_start)) + tid*0x4000;

		asm (
		#ifdef ARM_THUMB
			".align 4;"
			"mov    r0,pc;"
			"bx     r0;"
			".code 32;"
		#endif

			"MRC p15,0,%0,c13,c0,1;" // ; Read CP15 Context ID Register
			"MRC p15,0,%1,c2,c0,0;" // ; Read CP15 Translation Table Base Register 0

		#ifdef ARM_THUMB
			"add r0, pc,#1;"
			"bx  r0;"
			".code 16;"
		#endif
			: "=&r" (asid), "=&r" (tbb0)
			:
			: "r0"
		);

		LOG(ARCH,ERROR,(ARCH,ERROR,"ASID: %d, TBB0: 0x%x, Task PT: 0x%x",asid,tbb0,paget));

		theOS->getHatLayer()->dumpPageTable(tid);

		// dump TLB
	#endif

	//SETPID(0);

	// handle the error
	theOS->getErrorHandler()->handleError();

	while (true) {};
}

extern "C" void handleUndefinedIRQ(int addr, int spsr, int context) {
	LOG(ARCH,ERROR,(ARCH,ERROR,"Undefined Instruction IRQ, Addr: 0x%x, SPSR: 0x%x",addr,spsr));

	for (int i = 0; i < 12; i++) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"r%d : %x",i,((unint4*) context)[i]));
	}

	memdump(addr-16,8);

	int tid = 0;
	if (pCurrentRunningTask != 0)
		tid = pCurrentRunningTask->getId();

	LOG(ARCH,ERROR,(ARCH,ERROR,"TID: %d",tid));


	#ifdef HAS_Board_HatLayerCfd

		int asid = -1;
		unint4 tbb0 = 0;
		unint4 paget =	((unint4)(&__PageTableSec_start)) + tid*0x4000;

		asm (
		#ifdef ARM_THUMB
			".align 4;"
			"mov    r0,pc;"
			"bx     r0;"
			".code 32;"
		#endif

			"MRC p15,0,%0,c13,c0,1;" // ; Read CP15 Context ID Register
			"MRC p15,0,%1,c2,c0,0;" // ; Read CP15 Translation Table Base Register 0

		#ifdef ARM_THUMB
			"add r0, pc,#1;"
			"bx  r0;"
			".code 16;"
		#endif
			: "=&r" (asid), "=&r" (tbb0)
			:
			: "r0"
		);

		LOG(ARCH,ERROR,(ARCH,ERROR,"ASID: %d %d, TBB0: 0x%x, Task PT: 0x%x",asid >> 8, asid & 0xff,tbb0,paget));

		theOS->getHatLayer()->dumpPageTable(tid);

		// dump TLB
	#endif

	//SETPID(0);

	// handle the error
	theOS->getErrorHandler()->handleError();
}

extern "C" void handleFIQ() {
	LOG(ARCH,ERROR,(ARCH,ERROR,"FIQ.."));
	while (true) {};
}

extern "C" void handlePrefetchAbort(int instr, int context) {
	LOG(ARCH,ERROR,(ARCH,ERROR,"Prefetch Abort IRQ. instr: 0x%x",instr));

	for (int i = -1; i <= 13; i++) {
		LOG(ARCH,ERROR,(ARCH,ERROR,"r%d : %x",i,((unint4*) context)[i]));
	}

	int tid = 0;
	if (pCurrentRunningTask != 0)
		tid = pCurrentRunningTask->getId();

	int ifar,ifsr;
	asm (
		"MRC p15,0,%0,c6,c0,2;"
	    "MRC p15,0,%1,c5,c0,1;"
		: "=&r" (ifar), "=&r" (ifsr)
		:
		:
		);

	LOG(ARCH,ERROR,(ARCH,ERROR,"TID: %d, IFAR: %x, IFSR: %x",tid, ifar, ifsr));

	/*taskTable* tt = pCurrentRunningTask->getTaskTable();
	unint4 crc = crc32((char*) tt->task_start_addr,tt->task_data_end- tt->task_start_addr);
	LOG(ARCH,ERROR,(ARCH,ERROR,"CRC32: 0x%x",crc));*/


	#ifdef HAS_Board_HatLayerCfd

		int asid = -1;
		unint4 tbb0 = 0;
		unint4 paget =	((unint4)(&__PageTableSec_start)) + tid*0x4000;

		asm (
		#ifdef ARM_THUMB
			".align 4;"
			"mov    r0,pc;"
			"bx     r0;"
			".code 32;"
		#endif

			"MRC p15,0,%0,c13,c0,1;" // ; Read CP15 Context ID Register
			"MRC p15,0,%1,c2,c0,0;" // ; Read CP15 Translation Table Base Register 0

		#ifdef ARM_THUMB
			"add r0, pc,#1;"
			"bx  r0;"
			".code 16;"
		#endif
			: "=&r" (asid), "=&r" (tbb0)
			:
			: "r0"
		);

		LOG(ARCH,ERROR,(ARCH,ERROR,"ASID: %d %d, TBB0: 0x%x, Task PT: 0x%x",asid >> 8, asid & 0xff,tbb0,paget));

		theOS->getHatLayer()->dumpPageTable(tid);

		// dump TLB
	#endif


	theOS->getErrorHandler()->handleError();
}

/*
 * This method takes care of the low level IRQ functionality:
 * - registering the stack pointer and IRQ return mode
 * - getting the irq number
 * - dedicated interrupt dispatching for timer interrupts
 * - forwarding of all other irqs to the generic interrupt manager
 */
extern "C"void dispatchIRQ(void* sp_int, int mode)
{
	if (pCurrentRunningThread != 0){
		ASSERT(isOk(pCurrentRunningThread->pushStackPointer(sp_int)));
		ASSERT(isOk(pCurrentRunningThread->pushStackPointer((void*)mode)));
	}

	int irqSrc;

	irqSrc = theOS->getBoard()->getInterruptController()->getIRQStatusVector();
	//LOG(HAL,WARN,(HAL,WARN,"IRQ number: %d, sp_int %x, mode: %d",irqSrc, sp_int, mode));
	//dumpContext(sp_int);

	// jump to interrupt handler according to active interrupt
	switch (irqSrc)
	    {
			// General Purpose Timer interrupt
			case GPT1_IRQ:
			case GPT2_IRQ:
			{
				/* non returning irq ..*/
				theOS->getBoard()->getInterruptController()->clearIRQ(irqSrc);
				handleTimerInterrupt(sp_int);
				break;
			}
			default:
			{
				ErrorT result = theOS->getInterruptManager()->handleIRQ(irqSrc);
				theOS->getBoard()->getInterruptController()->clearIRQ(irqSrc);
			}
	    }

	/* Dispatch directly as a blocked thread might have been unblock by this irq handling */
	/* If the same thread is resumed we unfortunately lost some time, by not calling
	 * assemblerfunctions::resumeThread directly */
	theOS->getCPUDispatcher()->dispatch( (unint4) (theOS->getClock()->getTimeSinceStartup() - lastCycleStamp) );

	// we should never get here
	while(1);
}

extern "C"void dispatchSWI(void* sp_int, int mode)
{
#if ENABLE_NESTED_INTERRUPTS
    // we want nested interrupts so enable interrupts again
    _enableInterrupts();
#endif
	//LOG(HAL,WARN,(HAL,WARN,"SWI: stack 0x%x mode 0x%x",sp_int,mode));
	//dumpContext(sp_int);

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