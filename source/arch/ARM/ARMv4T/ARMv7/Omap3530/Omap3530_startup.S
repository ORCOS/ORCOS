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

#include <SCLConfig.hh>
#include "assembler.h"
#include "ARMv4T.h"

EXPORT_FCN(void, startORCOS,    (void))
EXPORT_FCN(void, exit,            (int))
EXPORT_FCN(void, _exit,            (int))

.extern __bss_start
.extern __bss_end

// text section
    .section    .text

// start and end addresses of the BSS and SBSS sections
// they are taken from the linker script

    .set    START_BSS,    __bss_start
    .set    END_BSS,    __bss_end

    .set    START_SBSS,    __sbss_start
    .set    END_SBSS,    __sbss_end

// TODO: enable later
// #if USE_SAFE_KERNEL_STACKS
// declare variable as global
//.global stackBucketBitmap
// #endif


FUNC_START(startORCOS)

    // Disable interrupts. They should be off by default, disable anyway to be sure.
    MSR cpsr, #svc_mode|CPSR_DISINT

    // be sure MMU is disabled
    MRC    p15, #0, r1, c1, c0, #0      // read CP15 Register 1
    BIC    r1,  r1, #0x1
    BIC    r1,  r1, #0x2                // disable alignment faults! ARMv7 processors must be able to handle unanligned accesses
    MCR    p15, #0, r1, c1, c0, #0      // disabled

#if ENABLE_BRANCH_PREDICTION
    MRC    p15, 0, r0, c1, c0, 0   // read Control Register
    MOV    r1, #0x800              // load enable branch prediction
    ORR    r1, r0, r1              // enable bit
    MCR p15, 0, r1, c1, c0, 0      // write Control Register
#else
    MRC    p15, 0, r0, c1, c0, 0   // read Control Register
    MVN    r1, #0x800              // load enable branch prediction
    AND    r1, r0, r1              // enable bit
    MCR p15, 0, r1, c1, c0, 0      // write Control Register
#endif

    MOV r0, #0
    MCR p15, 0, r0, c7 , c5, 0     // invalidate whole instruction cache
    //MCR p15, 0, r0, c15, c5, 0   // Invalidate entire data cache


// Instruction cache
#ifndef ICACHE_ENABLE
#error 'ICACHE_ENABLE' define missing in configuration!
#endif
#if ICACHE_ENABLE

    mov r1,#2
    mcr p15,#2,r1,c0,c0,#0         // Enable Level2 unified cache.

    // enable instruction cache
    MRC    p15, 0, r0, c1, c0, 0   // read Control Register
    MOV    r1, #0x1000             // load instruction cache enable bit
    ORR    r1, r0, r1              // mask
    MCR p15, 0, r1, c1, c0, 0      // write Control Register

    mrc p15, #0, r1, c1, c0, #1    // Read Auxiliary Control Register
    mov r2, #2
    orr r1, r1, r2                 // Set L2EN bit
    mcr p15, #0, r1, c1, c0, #1    // Write Auxiliary Control Register
#else
    // disable instruction cache
    MRC    p15, 0, r0, c1, c0, 0   // read Control Register
    MVN    r1, #CP15C1_IC_DIS      // load mask to disable instruction cache
    AND    r1, r0, r1              // mask
    MCR p15, 0, r1, c1, c0, 0      // write Control Register

#endif
// Data Cache
#ifndef DCACHE_ENABLE
#error 'DCACHE_ENABLE' define missing in configuration!
#endif
#if DCACHE_ENABLE
    // enable data cache
    MRC    p15, 0, r0, c1, c0, 0   // read Control Register
    MOV    r1, #0x4                // load instruction cache enable bit
    ORR    r1, r0, r1              // mask
    MCR p15, 0, r1, c1, c0, 0      // write Cont
#else
    // disable data cache
    MRC    p15, 0, r0, c1, c0, 0   // read Control Register
    MVN    r1, #CP15C1_DC_DIS      // load mask to disable data cache
    AND    r1, r0, r1              // mask
    MCR p15, 0, r1, c1, c0, 0      // write Control Register

#endif


    // be sure the vector table is at OMAP3530 rom location
    LDR  r0, =0x14000
    MCR  p15, #0, r0, c12, c0, #0

    # interrupt vector initilization
    # set RAM exception vectors to initial value:
    # from 0x4020ffc8 to 0x4020ffe0: arm instruction that loads address to pc
    # from 0x4020ffe4 to 0x4020fffc: addresses to be loaded to pc
    # compare OMAP3530 manual page 3412 (spruf98x.pdf)

    ldr r0,=0xe59ff014            // load instruction "LDR PC, [PC+20]" to r0
    ldr r1,=0x4020FFC8            // load RAM Exception Vector Table start address to r1

    ADD r2, r1, #0x20   // end address for loop

// set arm instruction: load into the PC the value in the next seven addresses
loopSetInstr:
    STR r0, [r1]        // store content of r0 in address given by r1
    ADD r1, r1, #4      // increment address
    CMP r1, r2          // compare with end address
    BNE loopSetInstr

    // default: jump to addresses from 0x14080 to 0x14098 (exception dead loops) when interrupt occurs
    // to be changed to address of custom interrupt handler when initializing interrupts

//    ldr r0,=0x14080        // load 0x14080 = Dead Address to r0
//    ADD r2, r1, #0x1C      // end address for loop

// set default addresses(0x14080 to 0x14098): redirect to exception dead loops
//loopSetAddr:
//    STR r0, [r1]          // store content of r0 in address given by r1
//    ADD r1, r1, #4        // increment address content is stored in
//    ADD r0, r0, #4        // increment address to be stored
//    CMP r1, r2            // compare with end address
//    BNE loopSetAddr

    // clear pending interrupts
    //LDR r0, =MPU_INTCPS_CONTROL
    //MOV r1, #0x3
    //STR r1, [r0]

    LDR r0, =undefinedIRQHandler
    LDR r1, =0x4020ffe4
    STR r0, [r1]

    ldr r0,=jumpToSwiHandler
    ldr r1,=0x4020FFE8            // load Software Interrupt (SWI) vector table address
    STR r0, [r1]

    LDR r0, =prefetchIRQHandler
    LDR r1, =0x4020ffec
    STR r0, [r1]

    LDR r0, =_data_abort_start
    LDR r1, =0x4020fff0
    STR r0, [r1]

    LDR r0, =undefinedIRQHandler
    LDR r1, =0x4020fff4
    STR r0, [r1]

    ldr r0,=jumpToIrqHandler
    ldr r1,=0x4020FFF8            // load Interrupt Request (IRQ) vector table address
    STR r0, [r1]

    LDR r0, =fiqIRQHandler
    LDR r1, =0x4020fffc
    STR r0, [r1]


// zero bss section
    LDR r0, =START_BSS      // load start of BSS into r0
    LDR r1, =END_BSS        // load end of BSS into r1
    CMP r0, r1              // compare start and end address
    BEQ doneBSS             // if they are equal we are done
    MOV r2, #0              // value to write to memory address
    SUB r1, r1, #4          // start 4 bytes lower (we delete 4 bytes in each step)


loopBSS:
    STR r2, [r1]            // zero content of address in r1
    SUB r1, r1, #4          // decrement address
    CMP r0, r1
    BNE loopBSS             // keep going until BSS is entirely clean

doneBSS:

// zero sbss section
    LDR r0, =START_SBSS     // load start of SBSS into r0
    LDR r1, =END_SBSS       // load end of SBSS into r1
    CMP r0, r1              // compare start and end address
    BEQ doneSBSS            // if they are equal we are done
    MOV r2, #0              // value to write to memory address
    SUB r1, r1, #4          // start 4 bytes lower (we delete 4 bytes in each step)

loopSBSS:
    STR r2, [r1]            // zero content of address in r1
    SUB r1, r1, #4          // decrement address
    CMP r0, r1
    BNE loopSBSS

doneSBSS:


    // enable user access to performance counter
    MOV r0,#1
    MCR p15, 0, r0, C9, C14, 0

    // disable overflow interrupts
    MOV r0,#0x8000000f
    MCR p15, 0, r0, C9, C14, 2

    // program the performance-counter control-register:
    MOV r0,#1
    MCR p15, 0, r0, c9, c12, 0

    // enable all counters:
    MOV r0,#0x8000000f
    MCR p15, 0, r0, c9, c12, 1
    // clear overflows
    MCR p15, 0, r0, c9, c12, 3


// Set system (supervisor mode) stack pointer.
// Take  __stack from linker script.
    LDR      sp, =__stack
    // stack limit sl = r10
    // frame pointer fp = r11

// initialize user stack
// should be done when jumping to user code
    MSR      CPSR_c, #31|CPSR_DISINT    // system mode shares registers with user mode
    LDR      sp, =__stack
    //MSR   CPSR_c, #19|CPSR_DISINT    // supervisor mode

// set irq stack
    MSR      CPSR_c, #18|CPSR_DISINT    // irq mode
    LDR      sp, =__stack    //
    //MSR   CPSR_c, #19|CPSR_DISINT    // supervisor mode

// set fiq stack
    MSR      CPSR_c, #17|CPSR_DISINT    // fiq mode
    LDR      sp, =__stack    //    //
    //MSR   CPSR_c, #19|CPSR_DISINT    // supervisor mode

// set abort stack
    MSR      CPSR_c, #23|CPSR_DISINT    // abort mode
    LDR      sp, =__stack    //    //
    //MSR   CPSR_c, #19|CPSR_DISINT    // supervisor mode

// set undefined stack
    MSR      CPSR_c, #27|CPSR_DISINT    // undefined mode
    LDR      sp, =__stack    //
    //MSR   CPSR_c, #19|CPSR_DISINT    // supervisor mode

    MSR         CPSR_c, #19|CPSR_DISINT    // supervisor mode

#if ARM_SUPPORT_NEON
    MRC p15,0,r0,c1,c0,2    // Read CP Access register
    ORR r0,r0,#0x00f00000   // Enable full access to NEON/VFP (Coprocessors 10 and 11)
    MCR p15,0,r0,c1,c0,2    // Write CP Access register
    ISB
    MOV r0,#0x40000000      // Switch on the VFP and NEON hardware
    VMSR FPEXC,r0           // Set EN bit in FPEXC
#endif

// now branch to c code entry method kernelmain
// if we return the system will hang since that is not supposed to happen
    bl      kernelmain

FUNC_START(exit)
FUNC_START(_exit)

// if kernelmain returns: hang
    b .

