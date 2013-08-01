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

EXPORT_FCN(void, restoreUserContext, (void))
EXPORT_FCN(void, restoreSVCContext, (void))




	.section .text

FUNC_START(restoreSVCContext)
restoreSVCContext:
	// r0 contains the stack pointer address of the return context
    // switch back to user mode
    // we are in svc mode .. put stack pointer
	MSR CPSR_c, #(irq_mode)|CPSR_DISINT
	mov		sp,r0

	MSR CPSR_c, #(svc_mode)|CPSR_DISINT
	ldr		lr,[r0,#60] // restore lr
	ldr		sp,[r0,#64] // restore sp
	ldr		fp,[r0,#68] // restore sp

	MSR CPSR_c, #(irq_mode)|CPSR_DISINT

	// get the PSR
	ldmfd   sp!, {r0}
	msr		SPSR, r0

    // load all other registers + restore PSR + jump to pc
	ldmfd   sp!, {r0-r12,pc}^

FUNC_START(restoreUserContext)
restoreUserContext:

	// r0 contains the stack pointer address of the return context
    // switch back to user mode
    mov		sp, r0

	MSR CPSR_c, #(system_mode)|CPSR_DISINT
	ldr		lr,[r0,#60] // restore lr
	ldr		sp,[r0,#64] // restore sp
	ldr		fp,[r0,#68] // restore sp

	MSR CPSR_c, #(svc_mode)|CPSR_DISINT

	// get the PSR
	ldmfd   sp!, {r0}
	msr		SPSR, r0

    // load all other registers + restore PSR + jump to pc
	ldmfd   sp!, {r0-r12,pc}^

	.section	.irq,  "ax", "progbits"

//---------------------------------------------------------
//		External IRQ Handler
//---------------------------------------------------------/

// one central function that reads the interrupt number from register
// and calls one central C dispatching function with this number as parameter
EXPORT_FCN(void, jumpToIrqHandler,			(void))
FUNC_START(jumpToIrqHandler)
jumpToIrqHandler:

	// set to irq stack top!
	LDR	sp, = __stack

	SUB     lr, lr, #4     // construct the return address
	push 	{r0}
	// Determine which mode was interrupted to access the corresponding sp and lr register
	// leave virtual memory translation enabled

	MRS r0, SPSR 		// load saved status register
	AND r0, r0, #0x1F 	// extract mode bits

	CMP r0, #user_mode 					// compare with user mode bit setting
	BEQ save_user_mode_from_irq_mode

	CMP r0, #svc_mode 					// compare with supervisor mode bit setting
	BEQ save_svc_mode_from_irq_mode

	// irq from irq mode? .. just skip saving as we will not return to irq mode
	B  .

save_user_mode_from_irq_mode:


	pop 	{r0}

	MSR CPSR_c, #(system_mode)|CPSR_DISINT
	// store stack pointer
	str		fp, [sp,#-4]
	// store stack pointer
	str		sp, [sp,#-8]
	// save lr
	str	    lr, [sp,#-12]
	// make room for pc
	sub 	sp,sp,#16
	// store context
	stmfd   sp!, {r0-r12}

	MSR CPSR_c, #(irq_mode)|CPSR_DISINT
	// get the return PSR
	mrs     r0, SPSR
	mov		r1, lr
	// switch back to user mode
	MSR CPSR_c, #(system_mode)|CPSR_DISINT
	str	    r1,[sp,#52] // store return PC
	stmfd   sp!, {r0} // store SPSR

	// get stack pointer address for parameter to the handleInterrupt method
    mov		r0, sp
    mov		r1, #1

	MSR CPSR_c, #(irq_mode)|CPSR_DISINT

	B saving_to_irq_done

save_svc_mode_from_irq_mode:


	pop 	{r0}

	MSR CPSR_c, #(svc_mode)|CPSR_DISINT
	// store stack pointer
	str		fp, [sp,#-4]
	// store stack pointer
	str		sp, [sp,#-8]
	// save lr
	str	    lr, [sp,#-12]
	// make room for pc
	sub 	sp,sp,#16
	// store context
	stmfd   sp!, {r0-r12}

	MSR CPSR_c, #(irq_mode)|CPSR_DISINT
	// get the return PSR
	mrs     r0, SPSR
	mov		r1, lr
	// switch back to user mode
	MSR CPSR_c, #(svc_mode)|CPSR_DISINT
	str	    r1,[sp,#52] // store return PC
	stmfd   sp!, {r0} // store SPSR

	// get stack pointer address for parameter to the handleInterrupt method
    mov		r0, sp
    mov		r1, #2

	MSR CPSR_c, #(irq_mode)|CPSR_DISINT

	B saving_to_irq_done

saving_to_irq_done:


	// switch to svc mode (OS mode)
	MSR CPSR_c, #svc_mode|CPSR_DISINT
	//sub		sp, r0, #10	// use the thread stack - safety margin
	LDR	sp, =__stack   // TODO: does not work for nested interrups .. check why we cant use
	// the thread stack instead ?? should be possible...

	// CALL INTERRUPT DISPATCHING FUNCTION
	bl	FUNC_NAME(dispatchIRQ)

	b   .
	nop



//---------------------------------------------------------
//		SWI Handler (Syscalls)
//---------------------------------------------------------/

	.section	.swi,  "ax", "progbits"

// function that handles software interrupts
EXPORT_FCN(void, jumpToSwiHandler,			(void))
FUNC_START(jumpToSwiHandler)
jumpToSwiHandler:

	//LDR	sp, = (__stack - 0x500)
	LDR	sp, =__stack
	// Determine which mode was interrupted
	push {r0} // save r0 on local stack

	MRS r0, SPSR // load saved status register
	AND r0, r0, #0x1F // extract mode bits
	CMP r0, #user_mode // compare with user mode bit setting
	BEQ save_user_mode_from_svc_mode
	CMP r0, #svc_mode // compare with supervisor mode bit setting
	BEQ save_svc_mode_from_svc_mode
	CMP r0, #irq_mode // compare with irq mode bit setting
	BEQ save_irq_mode_from_svc_mode

	// should not happen
	B .

save_user_mode_from_svc_mode:

	pop 	{r0}

	// TODO: we might get a data abort if the task purposly sets the sp register to
	// an invalid value
	// however: we can handle it inside the data abort handler and restart / remove the task

	MSR CPSR_c, #(system_mode)|CPSR_DISINT
	// store stack pointer
	str		fp, [sp,#-4]
	// store stack pointer
	str		sp, [sp,#-8]
	// save lr
	str	    lr, [sp,#-12]
	// make room for pc
	sub 	sp,sp,#16
	// store context
	stmfd   sp!, {r0-r12}

	MSR CPSR_c, #(svc_mode)|CPSR_DISINT
	// get the return PSR
	mrs     r0, SPSR
	mov		r1, lr
	// switch back to user mode
	MSR CPSR_c, #(system_mode)|CPSR_DISINT
	str	    r1,[sp,#52] // store return PC
	stmfd   sp!, {r0} // store SPSR

	// get stack pointer address for parameter to the handleInterrupt method
    mov		r0, sp
    mov		r1, #1

	MSR CPSR_c, #(svc_mode)|CPSR_DISINT

	B saving_to_svc_done

save_svc_mode_from_svc_mode:
	//SAVE_CONTEXT(svc_mode, svc_mode)
	//B saving_to_svc_done
	B .

save_irq_mode_from_svc_mode:
	//SAVE_CONTEXT(irq_mode, svc_mode)
	//B saving_to_svc_done
	B .

saving_to_svc_done:
	// switch to svc mode (OS mode)
	sub		sp, r0, #12	// use the thread stack - safety margin
	//LDR	sp, =__stack - 0x500 // TODO <-- doesnt work for nested interrupts!
	// CALL SYSCALL DISPATCHING FUNCTION
	b	FUNC_NAME(dispatchSWI)



no_int_mode_determined:
	NOP
	b no_int_mode_determined

	nop


//---------------------------------------------------------
//		Data Abort Handler
//---------------------------------------------------------/

.section	.data_abort,  "ax", "progbits"

	MRC p15,0,r0,c6,c0,0 				// Read CP15 Data Fault Address Register and copy to r0

	MOV r1, lr							// copy user stack pointer to r1
	MSR CPSR_c, #svc_mode|CPSR_DISINT	// switch to supervisor mode

	LDR	sp, =__stack - 0x200

	// handle the data abort using a safe kernel stack

	b 	FUNC_NAME(handleDataAbort)
	nop


//---------------------------------------------------------
//		Undefined IRQ Handler
//---------------------------------------------------------/


// function that handles software interrupts
EXPORT_FCN(void, undefinedIRQHandler,			(void))
FUNC_START(undefinedIRQHandler)
undefinedIRQHandler:


	MOV	r0, lr
	MRS	r1, SPSR
	MSR CPSR_c, #svc_mode|CPSR_DISINT	// switch to supervisor mode

	LDR	sp, =__stack - 0x200


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

	SUBS PC, LR, #4

	/*MOV	r1,lr
	MSR CPSR_c, #svc_mode|CPSR_DISINT	// switch to supervisor mode

	AND	r1, sp, #7
	SUB	sp, sp, r1						// be sure we are 8 bytes aligned!

	b	FUNC_NAME(handlePrefetchAbort)*/


