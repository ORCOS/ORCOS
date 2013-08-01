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

#include "SCLConfig.hh"
#include "windows.hh"
#include "assembler.h"
#include "asm/sparc.hh"


#define tmpPSR	l0
#define tmpPC	l1
#define tmpNPC	l2


/*#define SAVE_IN_REGISTER(sp) \
std	%i0, [%sp + 32]; \
std	%i2, [%sp + 40]; \
std	%i4, [%sp + 48]; \
std	%i6, [%sp + 56]; */

#define SAVE_IN_REGISTER(sp) \
st	%i0, [%sp + 32]; \
st	%i1, [%sp + 36]; \
st	%i2, [%sp + 40]; \
st	%i3, [%sp + 44]; \
st	%i4, [%sp + 48]; \
st	%i5, [%sp + 52]; \
st	%i6, [%sp + 56];\
st	%i7, [%sp + 60];

/*#define SAVE_GLOBAL_REGISTER(sp) \
    st	%g1, [%sp +  4]; \
	std	%g2, [%sp +  8]; \
	std	%g4, [%sp + 16]; \
	std	%g6, [%sp + 24];*/

#define SAVE_GLOBAL_REGISTER(sp) \
    st	%g1, [%sp +  4]; \
	st	%g2, [%sp +  8]; \
	st	%g3, [%sp + 12]; \
	st	%g4, [%sp + 16]; \
	st	%g5, [%sp + 20]; \
	st	%g6, [%sp + 24]; \
	st	%g7, [%sp + 28];

/*#define LOAD_GLOBAL_REGISTER(sp) \
	ld [%sp +  4], %g1; \
	ldd [%sp +  8], %g2; \
	ldd [%sp + 16], %g4; \
	ldd [%sp + 24], %g6; */

#define LOAD_GLOBAL_REGISTER(sp) \
	ld [%sp +  4], %g1; \
	ld [%sp +  8], %g2; \
	ld [%sp + 12], %g3; \
	ld [%sp + 16], %g4; \
	ld [%sp + 20], %g5; \
	ld [%sp + 24], %g6; \
	ld [%sp + 28], %g7;

/*#define LOAD_IN_REGISTER(sp) \
	ldd [%sp + 32], %i0; \
	ldd [%sp + 40], %i2; \
	ldd [%sp + 48], %i4; \
	ldd [%sp + 56], %i6;*/

#define LOAD_IN_REGISTER(sp) \
	ld [%sp + 32], %i0; \
	ld [%sp + 36], %i1; \
	ld [%sp + 40], %i2; \
	ld [%sp + 44], %i3; \
	ld [%sp + 48], %i4; \
	ld [%sp + 52], %i5; \
	ld [%sp + 56], %i6;\
	ld [%sp + 60], %i7;

// this registers are save to use
#define tmp1	l3
#define tmp2	l4
#define tmp3	l5
#define tmp4	l6
#define tmp5	l7

	.text
	.align	4

	// timer irq trap
	.globl trap_timer
trap_timer:
	set handleTrapTimer, %l0
	jmp %l0
	mov %psr, %l0


	// Syscall trap
	.globl trap_syscall
trap_syscall:
	set handleTrapSyscall, %l0
	jmp %l0
	mov %psr, %l0


	// irq trap
	.globl trap_irq
trap_irq:
	set handleTrapIRQ, %l0
	jmp %l0
	mov %psr, %l0


	// unaligned trap
	.globl trap_alignment
trap_alignment:
	set handleTrapAlignment, %l0
	jmp %l0
	mov %psr, %l0


	//Window overflow
	.globl	_wofHandler
_wofHandler:
	set window_overflow_handler, %l0
	jmp %l0
	nop

	//Window underflow
	.globl	_wufHandler
_wufHandler:
	set window_underflow_handler, %l0
	jmp %l0
	nop

#define MAX_THREADS 32

	.section	.text

#if USE_SAFE_KERNEL_STACKS
useSafeKernelStacks:
.extern stackBucketBitmap
.extern __stack
.extern _kernelStackOffset

	// set the sp on the kernel stack pointer,
    // if the user mode was enabled (PSR[S] = 1).
    // if PSR[S] = 0 we are already executing on a safe kernel stack!
    // l0 contains the value of the PSR

	and		%l0, 0x80, %tmp1
	cmp     %tmp1, 0x80
	be		FindNewKernelStackBucket
	nop
	// already executing on safe kernel stack
	ret

FindNewKernelStackBucket:
	// find a stack bucket we can use for this thread executing kernel code

	// load the value of the stackBucketBitmap into tmp2
	set 	stackBucketBitmap, %tmp1
	ld 		[%tmp1], %tmp2
	// find a free slot for the stack
	set		-1, %tmp3

	// iterate over all slots until a free slot is found
	// there must be a free slot since the designer has to specify the maximum amount of threads
iterate:
	// mask the bit number
	inc		%tmp3
	set		1, %tmp4
	sll		%tmp4, %tmp3, %tmp4		// mask
	and		%tmp2,	%tmp4, %tmp5	// get masked bit
	cmp		%tmp5, 0				// compare
	bne		iterate
	nop

	// found a free slot = 0. number is stored in tmp3
	// calculate sp address for that slot
	// mask the slot as occupied = 1
	// tmp4 still contains the mask
	mov %tmp3, %g6

	or		%tmp2, %tmp4, %tmp2
	st		%tmp2, [%tmp1]

	set 	__stack, %tmp4

	umul	%tmp3, KERNEL_STACK_SIZE, %tmp3
	sub		%tmp4, %tmp3, %sp
	add 	%sp, 120, %sp
	jmpl 	%g7 + 8, %g0
	nop

#endif

/*
 * for all traps this registers are set:
 * l0: PSR
 * l1: PC
 * l2: nPC
 */

handleTrapTimer:

	set checkForInvalidWindow, %tmp5
	jmpl	 %tmp5, %tmp5
	nop

// disable interrupts
	or 		%tmpPSR, PSR_PIL, %tmp1
	wr 		%tmp1, %psr
	nop; nop; nop

	// Enable Traps for window handler
	wr		%tmp1, PSR_ET, %psr
	nop; nop; nop

	PUSH_WINDOWS_TO_STACK

	// make space for trap stack
	add 	%fp, -120, %sp

	mov %g1, %l4
	mov %g2, %l5
	mov %g3, %l6
	mov %g4, %l7
	// save current window
	STORE_WINDOW(sp)

	rd  %y, %tmp1
	st %tmp1, [%sp + 96]

	// set parameter for handleTimer
	mov 	%sp, %o0

#if USE_SAFE_KERNEL_STACKS
	set		useSafeKernelStacks, %g7
	jmpl	%g7, %g7
	nop
#endif

	call 	handleTimerTrap
	mov 	%g6, %o1

handleTrapIRQ:

	//get trap type
	rd	%tbr, %tmp1
	and	 TBR_TT, %tmp1, %tmp1
	srl	%tmp1, 4, %g5

	set checkForInvalidWindow, %tmp5
	jmpl	 %tmp5, %tmp5
	nop

// disable interrupts
	or 		%tmpPSR, PSR_PIL, %tmp1
	wr 		%tmp1, %psr
	nop; nop; nop

	// Enable Traps for window handler
	// Note: The Sparc V8 specifications recommends to
	// not set the pil and et bit at the same time.
	wr		%tmp1, PSR_ET, %psr
	nop; nop; nop

	PUSH_WINDOWS_TO_STACK

	// make space for trap stack
	add 	%fp, -120, %sp

	mov %g1, %l4
	mov %g2, %l5
	mov %g3, %l6
	mov %g4, %l7
	// save current window
	STORE_WINDOW(sp)

	rd  %y, %tmp1
	st %tmp1, [%sp + 96]

	// set parameter
	mov 	%sp, %o0
	mov 	%g5, %o1

#if USE_SAFE_KERNEL_STACKS
	set		useSafeKernelStacks, %g7
	jmpl	%g7, %g7
	nop
#endif

	call 	handleIRQTrap
	mov 	%g6, %o2


	/*
	 * For the alignment handler a context save
	 * is not necessary because we return to the same task.
	 * But hte globals and the window of the interrupting routine
	 * are saved to the stack so that the register values are available
	 * for the trap handler
	 */
handleTrapAlignment:

	set checkForInvalidWindow, %tmp5
	jmpl	 %tmp5, %tmp5
	nop

	rd 		%wim, %tmp1
	wr	 	%g0, %wim
	nop; nop; nop

	// save previous window to stack
	restore
	STORE_WINDOW(sp)
	save

	wr 		%tmp1, %wim
	nop; nop; nop


	// disable interrupts
	set 	PSR_PIL, %tmp1
	or 		%tmpPSR, %tmp1, %tmp1
	wr 		%tmp1, %psr
	nop; nop; nop

	// Enable Traps for window handler
	wr		%tmp1, PSR_ET, %psr
	nop; nop; nop

	add 	%fp, -80, %sp

	SAVE_GLOBAL_REGISTER(sp)
	SAVE_IN_REGISTER(sp)

	mov	 	%sp, %o0
	call 	unalignedTrapHandler;
	ld 		[%l1], %o1

	// load register values
	mov %o0, %sp

	LOAD_GLOBAL_REGISTER(sp)
	LOAD_IN_REGISTER(sp)

	rd		%wim, %tmp1
	wr 		%g0, %wim
	nop; nop; nop

	restore
	LOAD_WINDOW(sp)
	save

	wr %tmp1, %wim
	nop; nop; nop

	// restore psr
	wr %tmpPSR, %psr
	nop; nop; nop

	//disable traps
	mov		PSR_ET, %tmp1
	not     %tmp1
	and	    %tmpPSR, %tmp1, %tmp1
	wr 		%tmp1, %psr
	nop; nop; nop


	jmp 	%tmpNPC
	rett 	%tmpNPC + 4


handleTrapSyscall:

	set checkForInvalidWindow, %tmp5
	jmpl	 %tmp5, %tmp5
	nop

	// disable interrupts
	or 		%tmpPSR, PSR_PIL, %tmp1
	wr 		%tmp1, %psr
	nop; nop; nop

	// Enable Traps for window handler
	wr	%tmp1, PSR_ET, %psr
	nop; nop; nop


	PUSH_WINDOWS_TO_STACK

	// make space for trap stack
	add %fp, -120, %sp

	//return to the instruction after the trapped instruction
	mov %tmpNPC, %tmpPC
	add %tmpNPC, 4, %tmpNPC

	mov %g1, %l4
	mov %g2, %l5
	mov %g3, %l6
	mov %g4, %l7
	// save current window
	STORE_WINDOW(sp)

	rd  %y, %tmp1
	st %tmp1, [%sp + 96]

	// set parameter for handleSyscalls
	mov 	%sp, %o0

#if USE_SAFE_KERNEL_STACKS
	set		useSafeKernelStacks, %g7
	jmpl	%g7, %g7
	nop
#endif

	call 	handleSyscallTrap
	mov 	%g6, %o1



	.globl	leavehandler
leavehandler:

	//disable traps
	rd		%psr, %tmpPSR
	mov		PSR_ET, %tmp1
	not     %tmp1
	and	    %tmpPSR, %tmp1, %tmpPSR
	wr 		%tmpPSR, %psr
	nop; nop; nop;

#ifdef HAS_MemoryManager_HatLayerCfd
	// set pid
	set 0x200, %g7
	sta %g6, [%g7] 0x19
#endif

	ld [%g5], %tmpPSR

	//disable traps in tmp PSR
	and	    %tmpPSR, %tmp1, %tmpPSR

	// set CWP to 0 in tmp PSR
	set 	0x1F, %tmp1
	not 	%tmp1
	and 	%tmp1, %tmpPSR, %tmpPSR
	wr	 	%tmpPSR, %psr
	nop; nop; nop;

	//restore stack pointer
	mov %g5, %sp

	//restore trap window
	LOAD_WINDOW(sp)

	mov %l4, %g1
	mov %l5, %g2
	mov %l6, %g3
	mov %l7, %g4

	ld [%sp + 96], %tmp1
	wr  %tmp1, %y
	nop; nop; nop

	/*
	* set wim to 4
	*/
	mov 	4 , %tmp1
	wr 		%tmp1, %wim
	nop; nop; nop;

	/*restore previous frame
	* rett must not cause a window underflow because traps
	* must be disabled for rett instruction
	*/
	restore
	LOAD_WINDOW(sp)
	save

	//disable traps
	rd		%psr, %tmpPSR
	mov		PSR_ET, %tmp1
	not     %tmp1
	and	    %tmpPSR, %tmp1, %tmp1
	wr 		%tmp1, %psr
	nop; nop; nop


	//set 0x200, %g5
	//sta  %g6, [%g5] 0x19

	jmp %tmpPC
	rett %tmpNPC


	/* If the trap handler is in the invalid window then the
	 * following window must be saved and the wim rotated one bit
	 * to the right. (in this situation we have a window overflow
	 * and a trap at the same time)
	 */
checkForInvalidWindow:

	// Compare the CWP with the WIM
	rd 		%wim, %tmp2
	set 	1, %tmp1
	sll 	%tmp1, %tmpPSR, %tmp1
	cmp 	%tmp1, %tmp2
	bne 	notInInvalidWindow
	nop

	wr %g0, %wim
	nop; nop; nop
	save
	STORE_WINDOW(sp)
	restore
	//rotate WIM one bit to the right
	srl %tmp2, 1, %tmp3
	sll %tmp2, 7, %tmp4
	or	%tmp3, %tmp4, %tmp2
	and  %tmp2, 0xFF, %tmp2
	wr %tmp2, %wim
	nop; nop; nop

notInInvalidWindow:
	jmpl %tmp5 + 8, %g0
	nop


