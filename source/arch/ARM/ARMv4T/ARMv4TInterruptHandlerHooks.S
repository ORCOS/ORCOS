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


#define _ASSEMBLER

#include "SCLConfig.hh"
#include "ARMv4T.h"
#include "assembler.h"


	.section .text
.arm

EXPORT_FCN(void, restoreThreadContext, (void))
FUNC_START(restoreThreadContext)
restoreThreadContext:
    // r0 = context address
	// r1 = restore mode
	// we need to restore from another mode
	MSR 	CPSR_c, #(undef_mode)|CPSR_DISINT
	mov		sp, r0

	// be sure interrupts are still disabled when we
	// temporarily switch to target mode to setup the lr,sp and fp
    ORR		r1, r1, #0xC0
    BIC  	r1, r1, #0x220						// Clear Bits: Thumb and Big Endian
	MSR 	CPSR_c, r1

	ldr		lr,[r0,#60] // restore lr
	ldr		sp,[r0,#64] // restore sp
	ldr		fp,[r0,#68] // restore fp

	MSR 	CPSR_c, #(undef_mode)|CPSR_DISINT

	// get the PSR
	ldmfd   sp!, {r0}
	msr		SPSR, r0

    // load all other registers + restore PSR + jump to pc
	ldmfd   sp, {r0-r12,pc}^
	nop
    nop

	.section	.irq,  "ax", "progbits"

//---------------------------------------------------------
//		External IRQ Handler
//---------------------------------------------------------/

// one central function that reads the interrupt number from register
// and calls one central C dispatching function with this number as parameter
EXPORT_FCN(void, jumpToIrqHandler,			(void))
FUNC_START(jumpToIrqHandler)
jumpToIrqHandler:

	// Determine which mode was interrupted to access the corresponding sp and lr register
	// leave virtual memory translation enabled

	MRS 	sp, SPSR 		// load saved status register
	AND 	sp, sp, #0x1F 	// extract mode bits

	CMP 	sp, #user_mode 	// compare with user mode bit setting
	MOVEQ	sp, #(system_mode) | CPSR_DISINT	// use system mode to save registers
	ORRNE   sp, sp, #CPSR_DISINT				// use interrupted mode to save registers
	BICNE	sp, sp, #0x220						// Clear Bits: Thumb and Big Endian

	MSR 	CPSR_c, sp			// switch to context save mode (access to interrupted mode register bank)

	str		fp, [sp,#-4]		// store stack pointer
	str		sp, [sp,#-8]		// store stack pointer
	str	    lr, [sp,#-12]		// save lr
	sub 	sp,  sp,#16			// make room for pc
	stmfd   sp!, {r0-r12}		// store context

	// we can now safely use r0-r12
	MSR 	CPSR_c, #(irq_mode)|CPSR_DISINT

	mrs     r2, SPSR			// get the return PSR to store away
	sub		r0, lr, #4			// get the return PC
	mov		r1, sp				// r2 == context save mode

	// switch back to user mode to store PSR and PC
	MSR 	CPSR_c, sp
	str	    r0,[sp,#52] 		// store return PC
	stmfd   sp!, {r2} 			// store PSR
	mov		r0, sp				// get save context address

	// switch to uninterrupable svc mode for interrupt handling (OS mode)
	MSR 	CPSR_c, #svc_mode | CPSR_DISINT

	LDR		sp, = __stack - 0x100	// irq handling is done on a seperate stack

	// r0 = saved context
	// r1 = context save mode

	// CALL INTERRUPT DISPATCHING FUNCTION
	bl	FUNC_NAME(dispatchIRQ)
	b	.
	nop



//---------------------------------------------------------
//		SWI Handler (Syscalls)
//---------------------------------------------------------/

	.section	.swi,  "ax", "progbits"

// function that handles software interrupts
EXPORT_FCN(void, jumpToSwiHandler,			(void))
FUNC_START(jumpToSwiHandler)
jumpToSwiHandler:

	// Determine which mode was interrupted to access the corresponding sp and lr register
	// leave virtual memory translation enabled

	MRS sp, SPSR 		// load saved status register
	AND sp, sp, #0x1F 	// extract mode bits

	CMP sp, #user_mode 	// compare with user mode bit setting
	MOVEQ	sp, #(system_mode) | CPSR_DISINT	// use system mode to save registers
	ORRNE   sp, sp, #CPSR_DISINT				// use interrupted mode to save registers
	BICNE	sp, sp, #0x220						// Clear Bits: Thumb and Big Endian

	MSR 	CPSR_c, sp			// switch to context save mode (access to interrupted mode register bank)

	str		fp, [sp,#-4]		// store stack pointer
	str		sp, [sp,#-8]		// store stack pointer
	str	    lr, [sp,#-12]		// save lr
	sub 	sp,  sp,#16			// make room for pc
	stmfd   sp!, {r0-r12}		// store context

	// we can now safely use r0-r12
	MSR 	CPSR_c, #(svc_mode)|CPSR_DISINT

	mrs     r2, SPSR			// get the return PSR to store away
	sub		r0, lr, #0			// get the return PC
	mov		r1, sp				// r2 == context save mode

	// switch back to user mode to store PSR and PC
	MSR 	CPSR_c, sp
	str	    r0,[sp,#52] 		// store return PC
	stmfd   sp!, {r2} 			// store PSR
	mov		r0, sp				// get save context address

	// switch to uninterrupable svc mode for interrupt handling (OS mode)
	MSR 	CPSR_c, #(svc_mode) | CPSR_DISINT

	sub		sp, r0, #40			// use the thread stack - safety margin

	// r0 = saved context
	// r1 = context save mode

	// CALL INTERRUPT DISPATCHING FUNCTION
	bl	FUNC_NAME(dispatchSWI)
	b	.
	nop


//---------------------------------------------------------
//		Data Abort Handler
//---------------------------------------------------------/

FUNC_START(abortIRQHandler)
.section	.data_abort,  "ax", "progbits"

	MRS 	sp, SPSR 		// load saved status register
	AND 	sp, sp, #0x1F 	// extract mode bits

	CMP 	sp, #user_mode 	// compare with user mode bit setting
	MOVEQ	sp, #(system_mode) | CPSR_DISINT	// use system mode to save registers
	ORRNE   sp, sp, #CPSR_DISINT				// use interrupted mode to save registers
	BICNE	sp, sp, #0x220						// Clear Bits: Thumb and Big Endian

	MSR 	CPSR_c, sp			// switch to context save mode (access to interrupted mode register bank)

	LDR		lr, =__stack - 0x200

	str		fp, [lr,#-4]		// store frame pointer
	str		sp, [lr,#-8]		// store stack pointer
	str	    lr, [lr,#-12]		// save lr
	sub 	lr,  lr,#16			// make room for pc
	stmfd   lr!, {r0-r12}		// store context

	// we can now safely use r0-r12
	MSR 	CPSR_c, #(abort_mode)|CPSR_DISINT

	mrs     r3, SPSR			// get the return PSR to store away
	sub		r1, lr, #4			// get the return PC

	// switch back to user mode to store PSR and PC
	MSR 	CPSR_c, sp
	str	    r1,[lr,#52] 		// store return PC
	stmfd   lr!, {r3} 			// store SP
	mov		r2, lr				// get save context address

	// switch to uninterrupable svc mode for interrupt handling (OS mode)
	MSR 	CPSR_c, #(svc_mode) | CPSR_DISINT

	LDR		sp, =__stack

	// Read CP15 Data Fault Address Register and copy to r0
	MRC p15,0,r0,c6,c0,0

	// handle the data abort using a safe kernel stack

	b 	FUNC_NAME(handleDataAbort)
	nop


//---------------------------------------------------------
//		Undefined IRQ Handler
//---------------------------------------------------------/


EXPORT_FCN(void, undefinedIRQHandler,			(void))
FUNC_START(undefinedIRQHandler)
undefinedIRQHandler:

	MSR CPSR_c, #system_mode|CPSR_DISINT	// switch to supervisor mode
	// store context
	LDR		sp, =__stack - 0x200
	stmfd   sp!, {r0-r12}
 	mov		r2, sp
	MSR CPSR_c, #undef_mode|CPSR_DISINT	// switch to supervisor mode

	SUB	r0, lr, #4
	MRS	r1, SPSR
	MOV	r3, sp

	MSR CPSR_c, #svc_mode|CPSR_DISINT	// switch to supervisor mode
	LDR		sp, =__stack

	b	FUNC_NAME(handleUndefinedIRQ)
	nop

//---------------------------------------------------------
//		FIQ Handler
//---------------------------------------------------------/

	// function that handles software interrupts
EXPORT_FCN(void, fiqIRQHandler,			(void))
FUNC_START(fiqIRQHandler)
fiqIRQHandler:
	B		.

	MSR CPSR_c, #svc_mode|CPSR_DISINT	// switch to supervisor mode
	AND	r1, sp, #7
	SUB	sp, sp, r1						// be sure we are 8 bytes aligned!

	b	FUNC_NAME(handleFIQ)
	nop

//---------------------------------------------------------
//		Prefetch Abort Handler
//---------------------------------------------------------/

	// function that handles software interrupts
EXPORT_FCN(void, prefetchIRQHandler,			(void))
FUNC_START(prefetchIRQHandler)
prefetchIRQHandler:


	// prefetch abort is not serious.. just return and let the processor
	// fetch the instruction again..
	// usually caused by a wrong branch prediction!

	MSR CPSR_c, #system_mode|CPSR_DISINT	// switch to supervisor mode

	LDR		sp, =__stack - 0x200
	stmfd   sp!, {r0-r12}
 	mov		r1, sp

	MSR CPSR_c, #abort_mode|CPSR_DISINT	// switch to supervisor mode
	SUB		r0,lr, #4
	MOV		r2, sp

	MSR 	CPSR_c, #svc_mode|CPSR_DISINT	// switch to supervisor mode
	LDR		sp, =__stack

	b	FUNC_NAME(handlePrefetchAbort)


