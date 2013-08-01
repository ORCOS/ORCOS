/*
	ORCOS - an Organic Reconfigurable Operating System
	Copyright (C) 2008 University of Paderborn

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
#include "ppc405.h"
#include "assembler.h"

// Stores the context onto stack. sp is the stack pointer at interruption
#define SAVE_CONTEXT \
	stmw		0,	-128(r1);\
	mfxer		r25; \
	mfctr		r26; \
	mfspr		r27, SRR0; \
	mfspr		r28, SRR1; \
	mfcr		r29;\
	mflr		r30;\
	mfspr 		r31,945; \
	stmw		25,	-156(r1);


// Restores the Context stored at sp (sp at interruption)
#define RESTORE_CONTEXT \
	lmw			25,	-156(r1); \
	mtxer		r25; \
	mtctr		r26; \
	mtspr		SRR0, r27;\
	mtspr		SRR1, r28;\
	mtcr		r29;\
	mtlr		r30;\
	mtspr		945,r31;\
	lwz			r0,-128(r1);\
	lwz         r2,-120(r1);\
	lwz         r3,-116(r1);\
	lmw			r4,-112(r1);\
	lwz			r1,-124(r1);

//lmw			0,0(sp);\ // gcc bug: lmw 0,0(sp) is valid but gcc claims it to be invalid (bug already issued at gcc)



EXPORT_FCN(void, _leaveHandler,		(void))

#define STACK_ALIGN 4
#define MAX_THREADS 32

	.section	.text

#if USE_SAFE_KERNEL_STACKS
useSafeKernelStacks:
.extern stackBucketBitmap
.extern __stack
.extern _kernelStackOffset

	// set the sp on the kernel stack pointer,
    // if the user mode was enabled (MSR[PR] = 1)
    // if MSR[PR] = 0 we are already executing on a safe kernel stack!
    // SRR1 contains value of MSR by a non critical interrupt

	mfspr       r3, SRR1
	andi.       r3, r3, 0x4000 // and MSR[PR]
	cmpi        0, r3, 0x4000
	beq			FindNewKernelStackBucket
	// already executing on safe kernel stack
	blr

FindNewKernelStackBucket:
	// find a stack bucket we can use for this thread executing kernel code
	lis 		r2,(stackBucketBitmap)@h
	ori 		r2,r2,(stackBucketBitmap)@l
	// load the value of the stackBucketBitmap into r3
	lwz			r3,0(r2)

	// find a free slot for the stack
	li			r4,-1
	// iterate over all slots until a free slot is found
	// there must be a free slot since the designer has to specify the maximum amount of threads
iterate:
	// mask the bit number
	addi		r4,r4,1
	li			r5,1
	slw			r5,r5,r4 // mask
	and			r6,r3,r5 // get masked bit
	cmpi		0,r6,0	 // compare
	bne			iterate

	// found a free slot = 0. number is stored in r4
	// calculate sp address for that slot
	// mask the slot as occupied = 1
	// r5 still contains the mask
	mtspr		SPRG5,r4
	or			r3,r3,r5
	stw			r3,0(r2)

	ldc(r5,__stack)

	mulli	r4,r4,KERNEL_STACK_SIZE
	sub		sp,r5,r4
	addi 	sp,sp,172
	sync
	isync
	blr

#endif

.globl IRQ0x0200
IRQ0x0200:
    li      r4,0x200
	ba 		IRQhandleCritical

.globl IRQ0x0700
IRQ0x0700:
    li      r4,0x700
	ba 		IRQhandleCritical


IRQhandleCritical:
// enable VM, should be available BEFORE working with the stack

    SAVE_CONTEXT
    mr          r3,sp

    addi        sp,sp,-172

    mfsrr2      r5
    mfsrr3      r6

    // CALL INTERRUPT HANDLING FUNCTION
    ba          FUNC_NAME(handleCriticalError)


.globl IRQ0x1100
IRQ0x1100:
	ba 		IRQhandleTLB

.globl IRQ0x1200
IRQ0x1200:
	ba 		IRQhandleTLB

IRQhandleTLB:
// enable VM, should be available BEFORE working with the stack

    SAVE_CONTEXT
    mr          r3,sp


    addi        sp,sp,-172


    // CALL INTERRUPT HANDLING FUNCTION
    ba          FUNC_NAME(handleTLBError)


.globl IRQ0x0c00
IRQ0x0c00:
	ba 		IRQhandleSyscall

IRQhandleSyscall:
	// enable VM, should be available BEFORE working with the stack
#ifdef HAS_MemoryManager_HatLayerCfd
    mtspr    SPRG5, r3
    mfmsr	 r3
    ori		 r3,r3,0x30
    mtmsr 	 r3
    sync
    isync
    mfspr    r3, SPRG5
#endif

	// write a 1 to the gpio pin
/*	mtspr		SPRG6,r0
	mtspr		SPRG7,r5
	li			r0,-1
	lis			r5,-28637
	stw			r0,0(r5)
	mfspr		r5,SPRG7
	mfspr		r0,SPRG6 */

	SAVE_CONTEXT
	mr			r3,sp

#if USE_SAFE_KERNEL_STACKS
	mtspr		SPRG4,sp
	bl			useSafeKernelStacks
#endif

	addi		sp,sp,-172

#if USE_SAFE_KERNEL_STACKS
	mfspr		r4,SPRG5
	mfspr		r3,SPRG4
#endif

	// CALL INTERRUPT HANDLING FUNCTION
	ba			FUNC_NAME(handleSyscallIRQ)

.globl IRQ0x1000
IRQ0x1000:
	ba 	IRQhandleTimer

.globl IRQ0x1010
IRQ0x1010:
	ba IRQhandleTimer

IRQ0x1020:
.globl IRQ0x1020
	ba IRQhandleWatchdog

IRQhandleTimer:
	// enable VM, should be available BEFORE working with the stack
#ifdef HAS_MemoryManager_HatLayerCfd
    mtspr    SPRG5, r3
    mfmsr	 r3
    ori		 r3,r3,0x30
    mtmsr 	 r3
    sync
    isync
    mfspr    r3, SPRG5
#endif

	SAVE_CONTEXT
	mr			r3,sp

#if USE_SAFE_KERNEL_STACKS
	mtspr		SPRG4,sp
	bl			useSafeKernelStacks
#endif

	addi		sp,sp,-172

#if USE_SAFE_KERNEL_STACKS
	mfspr		r4,SPRG5
	mfspr		r3,SPRG4
#endif

	// CALL INTERRUPT HANDLING FUNCTION
	ba			FUNC_NAME(handleTimerIRQ)

IRQhandleWatchdog:
	// enable VM, should be available BEFORE working with the stack
#ifdef HAS_MemoryManager_HatLayerCfd
    mtspr    SPRG5, r3
    mfmsr	 r3
    ori		 r3,r3,0x30
    mtmsr 	 r3
    sync
    isync
    mfspr    r3, SPRG5
#endif

#if USE_SAFE_KERNEL_STACKS
	bl			useSafeKernelStacks
#endif

	SAVE_CONTEXT

	mr			r3,sp
	addi		sp,sp,-172

#if USE_SAFE_KERNEL_STACKS
	mfspr		r4,SPRG5
#endif

	// CALL INTERRUPT HANDLING FUNCTION
	ba			FUNC_NAME(handleWatchdogIRQ)

.globl IRQ0x0500
IRQ0x0500:
	ba 	IRQhandleExternal

IRQhandleExternal:
	// enable VM, should be available BEFORE working with the stack
#ifdef HAS_MemoryManager_HatLayerCfd
    mtspr    SPRG5, r3
    mfmsr	 r3
    ori		 r3,r3,0x30
    mtmsr 	 r3
    sync
    isync
    mfspr    r3, SPRG5
#endif


	SAVE_CONTEXT
	mr			r3,sp

#if USE_SAFE_KERNEL_STACKS
	mtspr		SPRG4,sp
	bl			useSafeKernelStacks
#endif

	addi		sp,sp,-172

#if USE_SAFE_KERNEL_STACKS
	mfspr		r4,SPRG5
	mfspr		r3,SPRG4
#endif

	// call handling method.
	ba			FUNC_NAME(handleExternalIRQ)


FUNC_START(_leaveHandler)
_s__leaveHandler:

	// restore the context, sp is supposed to be the address at interruption
	// so sp is the sp address before SAVE_CONTEXT was executed
	RESTORE_CONTEXT

/*	mtspr		SPRG6,r0
	li			r0,0
	lis			r3,-28637
	stw			r0,0(r3)
	mfspr		r0,SPRG6*/

	// Return from exception.
	// This must have been an non critical interrupt
	// since otherwise we cant return
	sync
	isync
	rfi


