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

#include <SCLConfig.hh>
#include "assembler.h"
#include "ppc405.h"
#include <board.hh>

IMPORT_VAR(RegisterT, gBootLev)

EXPORT_FCN(void, startORCOS,	(void))
EXPORT_FCN(void, exit,			(int))
EXPORT_FCN(void, _exit,			(int))

EXPORT_VAR(int, gFlashState)

	.section	.data
gFlashState:
	.int		0


#if USE_SAFE_KERNEL_STACKS
stackBucketBitmap:
	.int		0
#endif

	.section	.text

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

	// Initialize vector tables and other registers
	// set them all to 0. The InterruptHandler implementation
	// has to set these registers later on

	li		r0,0
	mtevpr 	r0
	mticcr 	r0
	mtdccr 	r0
	mtdcwr 	r0
	mtsgr   r0

	// set the Machine State Register

	ldc(r3,SYS_MSR_VAL)		// load initial MSR value
	mtmsr r3				// set MSR to initial value

	// clear the timer registers

	li		r0,0			// zero r0
	mtpit	r0				// clear PIT so no indication after TSR cleared
	not		r3,r0			// complement in r3
	mtspr	0x3d8,r3		// clear TSR using mask
	mtspr   0x3da,r0		// disable timers (and leave them disabled!)
	li 		r5,0xffff8740   // setting the ZPR Register
	mtzpr 	r5


	// clear timebase.
	mttbl	r0		// first clear time base low to avoid rollover
	mttbu	r0		// set time base high first
	mttbl	r0		// set time base low next

#ifndef ICACHE_ENABLE
#error 'ICACHE_ENABLE' define missing in configuration!
#endif
#if ICACHE_ENABLE
	// initialize instruction cache
	iccci	0,r0			// invalidate instruction cache

	// enable instruction caching for the ram if we dont use virtual memory
	// if we use vm the cachability is set using the tlb

	li 		r5,0xffffffff   // setting the mask to enable caching for all memory blocks
	mticcr  r5

#endif


#if 1
	// Wipe out any existing interrupt vectors.
	// This is hardcoded to clear all memory within the range 0x0000..0x4000.
	// Any change of the memory map requires a modification of this code.

	li		r5,0x4000
	srwi	r4,r5,2		// convert byte-length to word-length
	mtctr	r4			// number of words to wipe out from 0..0x4000
	li		r0,0		// value to write onto the memory locations
wipe:
	stwu	r0,-4(r5)	// wipe out and decrement address
	bdnz	wipe
#endif

	// clear the BSS section

	ldc(r3,START_BSS)	// load start of BSS into r3
	ldc(r4,END_BSS)		// load end of BSS into r4
	sub		r4,r4,r3	// calculate length of BSS
	srwi	r4,r4,2		// convert byte-length to word-length
	li		r5,0		// zero r5
	cmplw	0,r4,r5		// check to see whether length equals 0
	beql	0,doneBSS	// in case of length 0 we're already done
	subi	r3,r3,4		// because of offset start 4 bytes lower
	mtctr	r4			// use word-length of BSS section as counter
loopBSS:
	stwu	r5,4(r3)	// zero one word of BSS section
	bdnz	loopBSS		// keep going until BSS is entirely clean
doneBSS:

	// zero SBSS section

	ldc(r3,START_SBSS)	// load start of SBSS into r3
	ldc(r4,END_SBSS)	// load length of SBSS into r4
	sub	r4,r4,r3		// calculate length of SBSS
	srwi	r4,r4,2		// convert byte-length to word-length
	cmplw	0,r4,r5		// check to see whether length equals 0
	beql	0,doneSBSS	// in case of length 0 we're already done
	subi	r3,r3,4		// because of offset start 4 bytes lower
	mtctr	r4			// use word-length of SBSS section as counter
loopSBSS:
	stwu	r5,4(r3)	// zero one word of SBSS section
	bdnz	loopSBSS	// keep going until SBSS is entirely clean
doneSBSS:

#ifndef DCACHE_ENABLE
#error 'DCACHE_ENABLE' define missing in configuration!
#endif
#if DCACHE_ENABLE
	// initialize the data cache after BSS and SBSS are cleared
	// since we dont want these memory areas to be in the cache right away
	li 		r0, 512		// invalidate all 512 data cache lines
	mtctr	r0			// set counter to 512
	li 		r5, 0		// start at address 0
invalidate:
	dccci	0,r5		// invalidate congurence class at address r5
	addi	r5,r5,32	// add 32 to address to get the next congurence class
	bdnz 	invalidate 	// run until ctr = 0 (512 times executed)

	li		r0,0
	mtccr0	r0
	li 		r0,0
	mtdcwr	r0

	//li 		r0,0x80000000
	// enable data caching for the ram if we dont use virtual memory
	// if we use vm the cachability is set using the tlb

	li		r0,1
	li 		r5,31
	slw		r0,r0,r5
	mtdccr	r0

#endif

	ldc(sp, cTopOfBootMemAddr)	 		// init stack pointer to top of memory

	// now branch to c code entry method
	// if we return the system will hang since thats not supposed to happen
	bl	FUNC_NAME(kernelmain)

FUNC_START(exit)
FUNC_START(_exit)
	b	hang

hang:
	nop

#	if 0
	.int	0	/* cause a program exception to stop debugging */
#	endif

	b	hang
