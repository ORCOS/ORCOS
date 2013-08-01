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

#define trap_base_addr		g1
#define node_offset			g2

#include "assembler.h"
#include "asm/sparc.hh"
#include "asm/Leon3.hh"
#include <SCLConfig.hh>


EXPORT_FCN(void, startORCOS,	(void))
EXPORT_FCN(void, exit,			(int))
EXPORT_FCN(void, _exit,			(int))

.section	".data"

#if USE_SAFE_KERNEL_STACKS
stackBucketBitmap:
	.int		0
#endif

.section	".text"

// start and end addresses of the BSS and SBSS sections
// they are taken from the linker script

	.set	START_BSS,	__bss_start
	.set	END_BSS,	__bss_end

	.set	START_SBSS,	__sbss_start
	.set	END_SBSS,	__sbss_end

#if USE_SAFE_KERNEL_STACKS
// global declarations
.global stackBucketBitmap
#endif


FUNC_START(startORCOS)

	// set interrupt level to 15
	rd		%psr, %l0
	set 	PSR_PIL, %l1
	or 		%l0, %l1, %l1
	wr 		%l1, %psr
	nop; nop; nop;

	// flush cache
	flush
	set 0x81000f, %g1
	sta %g1, [%g0] 2

#ifndef ICACHE_ENABLE
#error 'ICACHE_ENABLE' define missing in configuration!
#endif
#if ICACHE_ENABLE
// instruction cache is per default enabled
#else
	// disable instruction cache
	lda [%g0] 2, %l2
	set 0xFFFFFFF3, %l3
	and %l2, %l3, %l3
	sta %l3, [%g0]2
#endif

#ifndef DCACHE_ENABLE
#error 'DCACHE_ENABLE' define missing in configuration!
#endif
#if DCACHE_ENABLE
// data cache is per default enabled
#else
	// disable data cache
	lda [%g0] 2, %l2
	set 0xFFFFFFFC, %l3
	and %l2, %l3, %l3
	sta %l3, [%g0]2
#endif

	set TT_BASE_ADRESS, %trap_base_addr

#if MP_CONFIG
	set NODE_OFFSET, %node_offset

	//set the trap base register dependent
	// on cpu index
	mov 	%asr17, %g7
	srl %g7, 0x1C, %g7

	umul %node_offset, %g7, %g6
	add %g6, %trap_base_addr, %trap_base_addr
#endif

	// initialize Trap Base Register (TBR)
	mov %trap_base_addr, %tbr
	nop; nop; nop


	// initialize prescaler so that it generates a tick every microsecond
	set (CLOCK_RATE / 1000000) -1 , %l0
	set PSRLR, %l1
	st %l0, [%l1]	// set reload value
	set PSVR, %l1
	st %g0, [%l1]	// set current value


 	// disable all timer
	set 0x80000308, %l0
	ld [%l0], %l1
	and %l1, 0x7, %l1	// get timer count
	add %l0, 0x10, %l0
loop:
	st %g0, [%l0]		// disable each timer
	deccc %l1
	bne loop
	add %l0, 0x10, %l0  // delay slot


	// Wipe out any existing interrupt vectors.
	// This is hardcoded to clear all memory within the range 0x0000..0x4000.
	// Any change of the memory map requires a modification of this code.
	set 0x3FFC, %l1
	add %trap_base_addr, %l1, %l1

wipe:
	clr [%l1]					// clear register
	cmp %trap_base_addr, %l1	//compare upper against lower bound
	bnz wipe
	deccc 4, %l1				// decrement address

	//set kernel stack pointer
	set __stack, %l0
	mov %l0, %sp

	// initialize trap handlers for overflow underflow and misalignment. This must
	// be done here because these traps occures already before the interrupt
	// handler is initialized
	call FUNC_NAME(initTrapHandler)
	nop

// clear the BSS section

	set START_BSS, %l0	// load start of BSS into l0
	set END_BSS, %l1	// load end of BSS into l1

	cmp	%l0, %l1		// check to see whether length equals 0
	bz doneBSS			// in case of length 0 we're already done
	nop
	sub	%l1, 4, %l1		// because of offset start 4 bytes lower
loopBSS:
	clr	[%l1]			// zero one word of BSS section
	cmp	%l0, %l1
	bnz	loopBSS			// keep going until BSS is entirely clean
	deccc 4, %l1		// decrement address
doneBSS:

// clear the SBSS section

	set START_SBSS, %l0	// load start of SBSS into l1
	set END_SBSS, %l1	// load end of SBSS into l2

	cmp	%l0, %l1		// check to see whether length equals 0
	bz doneSBSS			// in case of length 0 we're already done
	nop
	sub	%l1, 4, %l1		// because of offset start 4 bytes lower
loopSBSS:
	clr	[%l1]			// zero one word of SBSS section
	cmp	%l0, %l1
	bnz	loopSBSS		// keep going until SBSS is entirely clean
	deccc 4, %l1		// decrement worth length
doneSBSS:


#if MASTER_NODE
	// start other processors
	set 	MAX_NODES, %l1
	dec		%l1;
	set 	1, %l2
	set		0, %l4
loopNodes:
	sll		%l2, %l1, %l3
	or		%l3, %l4, %l4
	deccc 	%l1
	bne		loopNodes
	nop

	set 	0x80000210, %l0
	st %l4, [%l0]
#endif





	// now branch to c code entry method
	// if we return the system will hang since thats not supposed to happen
	ba	FUNC_NAME(kernelmain)
	nop

FUNC_START(exit)
FUNC_START(_exit)
	ba	hang
	nop

hang:
	nop

#	if 0
	.int	0	/* cause a program exception to stop debugging */
#	endif

	ba	hang
	nop
