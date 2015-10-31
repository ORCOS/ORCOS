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
// interrupt handler must be in arm mode
.arm

EXPORT_FCN(void, restoreThreadContext, (void))
FUNC_START(restoreThreadContext)
restoreThreadContext:
    // r0 = context address
    // r1 = restore mode
    // we need to restore from another mode
    MSR     CPSR_c, #(undef_mode)|CPSR_DISINT
    MOV     sp, r0

    // be sure interrupts are still disabled when we
    // temporarily switch to target mode to setup the lr,sp and fp
    ORR     r1, r1, #0xC0
    BIC     r1, r1, #0x220 // Clear Bits: Thumb and Big Endian
    MSR     CPSR_c, r1

    LDR     lr,[r0,#64] // restore lr
    CLREX               // be sure exclusive locks are removed
    LDR     sp,[r0,#60] // restore sp

    MSR     CPSR_c, #(undef_mode)|CPSR_DISINT

    #if ARM_SUPPORT_NEON
    SUB     sp, sp, #256        // place sp at NEON context
    VLDMIA  sp!, {q0-q7}        // load floating point context .. costly
    VLDMIA  sp!, {q8-q15}
    #endif

    // get the PSR
    LDMFD   sp!, {r2}
    MSR     SPSR_cxsf, r2

    // load all other registers + restore PSR + jump to pc
    LDMFD   sp, {r0-r12,pc}^
    NOP
    NOP

//---------------------------------------------------------
//      External IRQ Handler
//---------------------------------------------------------/

    .section    .irq,  "ax", "progbits"
// one central function that reads the interrupt number from register
// and calls one central C dispatching function with this number as parameter
EXPORT_FCN(void, jumpToIrqHandler, (void))
FUNC_START(jumpToIrqHandler)
jumpToIrqHandler:
    // Determine which mode was interrupted to access the corresponding sp and lr register
    // leave virtual memory translation enabled
    MRS     sp, SPSR            // load saved status register
    AND     sp, sp, #0x1F       // extract mode bits

    CMP     sp, #user_mode                      // compare with user mode bit setting
    MOVEQ   sp, #(system_mode) | CPSR_DISINT    // use system mode to save registers
    ORRNE   sp, sp, #CPSR_DISINT                // use interrupted mode to save registers
    BICNE   sp, sp, #0x220                      // Clear Bits: Thumb and Big Endian

    // sp now contains the context save/restore mode
    MSR     CPSR_c, sp          // switch to context save mode (access to interrupted mode register bank)

    // save register context on stack. Layout:
    // offset: 0   4   8   12  16  20  24  28  32  36  40  44  48  52  56  60  64 68
    //         |psr|r0 |r1 |r2 |r3 |r4 |r5 |r6 |r7 |r8 |r9 |r10|r11|r12|pc |sp |lr |
    STMFD   sp!, {sp,lr}       // save lr and sp .. leave one register space for PC  .. deprecated
    SUB     sp,  sp, #4        // make room for pc
    STMFD   sp!, {r0-r12}      // store r0-r12 context
    MOV     r0, sp

    // we can now safely use r0-r12
    // switch to uninterrupable irq mode for interrupt handling (OS mode)
    // and store away return PC and PSR
    MSR     CPSR_c, #(irq_mode)|CPSR_DISINT

    MRS     r2 , SPSR           // get the return PSR to store away
    SUB     r3 , lr, #4         // get the return PC
    STR     r3 , [r0, #52]      // store return PC
    MOV     r1 , sp             // r1 == context save mode
    // switch back to user mode to store PSR and PC
    //MSR         CPSR_c, sp
    STMFD   r0!, {r2}           // store PSR

    #if ARM_SUPPORT_NEON
    MOV     r11, r0             // transparently place NEON context below arm context
    VSTMDB  r11!, {q0-q7}       // store floating point context .. costly
    VSTMDB  r11!, {q8-q15}
    #endif

    MSR     CPSR_c, #(svc_mode)|CPSR_DISINT
    LDR     sp, = __stack - 0x100  // irq handling is done on a seperate stack

    // r0 = saved context
    // r1 = context save mode
    // CALL INTERRUPT DISPATCHING FUNCTION
    BLX       FUNC_NAME(dispatchIRQ)
    NOP



//---------------------------------------------------------
//      SWI Handler (Syscalls)
//---------------------------------------------------------/

.section    .swi,  "ax", "progbits"

// software interrupt handler (SWC/SWI)
EXPORT_FCN(void, jumpToSwiHandler, (void))
FUNC_START(jumpToSwiHandler)
jumpToSwiHandler:
    // Determine which mode was interrupted to access the corresponding sp and lr register
    // leave virtual memory translation enabled

    MRS     sp, SPSR            // load saved status register
    AND     sp, sp, #0x1F       // extract mode bits

    CMP     sp, #user_mode                      // compare with user mode bit setting
    MOVEQ   sp, #(system_mode) | CPSR_DISINT    // use system mode to save registers
    ORRNE   sp, sp, #CPSR_DISINT                // use interrupted mode to save registers
    BICNE   sp, sp, #0x220                      // Clear Bits: Thumb and Big Endian

    // sp now contains the context save/restore mode
    MSR     CPSR_c, sp          // switch to context save mode (access to interrupted mode register bank)

    // save register context on stack. Layout:
    // offset: 0   4   8   12  16  20  24  28  32  36  40  44  48  52  56  60  64 68
    //         |psr|r0 |r1 |r2 |r3 |r4 |r5 |r6 |r7 |r8 |r9 |r10|r11|r12|pc |sp |lr |
    //STMFD   sp!, {r0,sp,lr}   // save lr and sp .. leave one register space for PC  .. deprecated
    // TODO: check sp for valid range
    STR     lr,  [sp,#-4]       // save lr
    STR     sp,  [sp,#-8]       // store stack pointer
    SUB     sp,  sp,#12         // make room for pc
    STMFD   sp!, {r0-r12}       // store r0-r12 context
    MOV     r0, sp

    // we can now safely use r0-r11
    // switch to uninterrupable svc mode for interrupt handling (OS mode)
    // and store away return PC and PSR
    MSR     CPSR_c, #(svc_mode)|CPSR_DISINT

    MRS     r2, SPSR           // get the return PSR to store away
    SUB     r3, lr, #0         // get the return PC
    STR     r3 , [r0,#52]      // store return PC
    MOV     r1, sp             // r1 == context save mode

    STMFD   r0!, {r2}          // store PSR

    #if ARM_SUPPORT_NEON
    MOV     r11, r0             // transparently place NEON context below arm context
    VSTMDB  r11!, {q0-q7}       // store floating point context .. costly
    VSTMDB  r11!, {q8-q15}
    SUB     sp , r11, #12        // use the thread stack - safety margin
    #else
    SUB     sp , r0, #12         // use the thread stack - safety margin
    #endif


    // r0 = saved context address
    // r1 = context save/restore mode
    // CALL SYSALL DISPATCHING FUNCTION
    BLX     FUNC_NAME(dispatchSWI)
    NOP


//---------------------------------------------------------
//      Data Abort Handler
//---------------------------------------------------------/

.section    .data_abort,  "ax", "progbits"

FUNC_START(abortIRQHandler)
    LDR     sp, =__stack - 0x200

    sub     sp,  sp,#12         // make room for pc
    stmfd   sp!, {r0-r12}       // store context

    MOV     r0, sp

    MRS     r1, SPSR        // load saved status register
    AND     r1, r1, #0x1F   // extract mode bits

    CMP     r1, #user_mode                      // compare with user mode bit setting
    MOVEQ   r1, #(system_mode) | CPSR_DISINT    // use system mode to save registers
    ORRNE   r1, r1, #CPSR_DISINT                // use interrupted mode to save registers
    BICNE   r1, r1, #0x220                      // Clear Bits: Thumb and Big Endian

    MSR     CPSR_c, r1          // switch to context save mode (access to interrupted mode register bank)

    str     sp, [r0, #56]        // store stack pointer
    str     lr, [r0, #60]        // save lr

    // we can now safely use r0-r12
    MSR     CPSR_c, #(abort_mode)|CPSR_DISINT

    mrs     r3, SPSR            // get the return PSR to store away
    sub     r1, lr, #4          // get the return PC

    str     r1 , [r0, #52]      // store return PC
    stmfd   r0!, {r3}           // store SPSR
    MOV     r2, r0              // get context address into r2

    // switch to uninterrupable svc mode for interrupt handling (OS mode)
    MSR     CPSR_c, #(svc_mode) | CPSR_DISINT

    LDR     sp, =__stack

    // Read CP15 Data Fault Address Register and copy to r0
    MRC     p15, 0, r0, c6, c0, 0

    // handle the data abort using a safe kernel stack
    B      FUNC_NAME(handleDataAbort)
    nop


//---------------------------------------------------------
//      Undefined IRQ Handler
//---------------------------------------------------------/


EXPORT_FCN(void, undefinedIRQHandler, (void))
FUNC_START(undefinedIRQHandler)
undefinedIRQHandler:

    MSR CPSR_c, #system_mode|CPSR_DISINT    // switch to supervisor mode
    // store context for further analysis
    LDR    sp, =__stack - 0x200
    stmfd  sp!, {r0-r12}
    mov    r2, sp
    MSR CPSR_c, #undef_mode|CPSR_DISINT     // switch to supervisor mode

    SUB    r0, lr, #4
    MRS    r1, SPSR
    MOV    r3, sp

    MSR    CPSR_c, #svc_mode|CPSR_DISINT    // switch to supervisor mode
    LDR    sp, =__stack

    B      FUNC_NAME(handleUndefinedIRQ)
    NOP

//---------------------------------------------------------
//      FIQ Handler
//---------------------------------------------------------/

    // function that handles software interrupts
EXPORT_FCN(void, fiqIRQHandler, (void))
FUNC_START(fiqIRQHandler)
fiqIRQHandler:
    B      .

    MSR    CPSR_c, #svc_mode|CPSR_DISINT   // switch to supervisor mode
    AND    r1, sp, #7
    SUB    sp, sp, r1                      // be sure we are 8 bytes aligned!

    B       FUNC_NAME(handleFIQ)
    NOP

//---------------------------------------------------------
//      Prefetch Abort Handler
//---------------------------------------------------------/

    // function that handles software interrupts
EXPORT_FCN(void, prefetchIRQHandler,    (void))
FUNC_START(prefetchIRQHandler)
prefetchIRQHandler:
    MRS     sp, SPSR            // load saved status register
    AND     sp, sp, #0x1F       // extract mode bits

    CMP     sp, #user_mode                      // compare with user mode bit setting
    MOVEQ   sp, #(system_mode) | CPSR_DISINT    // use system mode to save registers
    ORRNE   sp, sp, #CPSR_DISINT                // use interrupted mode to save registers
    BICNE   sp, sp, #0x220                      // Clear Bits: Thumb and Big Endian

    // sp now contains the context save/restore mode
    MSR     CPSR_c, sp          // switch to context save mode (access to interrupted mode register bank)

    // save register context on stack. Layout:
    // offset: 0   4   8   12  16  20  24  28  32  36  40  44  48  52  56  60  64 68
    //         |psr|r0 |r1 |r2 |r3 |r4 |r5 |r6 |r7 |r8 |r9 |r10|r11|r12|pc |sp |lr |
    //STMFD   sp!, {r0,sp,lr}   // save lr and sp .. leave one register space for PC  .. deprecated
    // TODO: check sp for valid range
    STR     lr,  [sp,#-4]       // save lr
    STR     sp,  [sp,#-8]       // store stack pointer
    SUB     sp,  sp,#12         // make room for pc
    STMFD   sp!, {r0-r12}       // store r0-r12 context
    MOV     r0, sp

    // we can now safely use r0-r11
    // switch to uninterrupable svc mode for interrupt handling (OS mode)
    // and store away return PC and PSR
    MSR     CPSR_c, #(abort_mode)|CPSR_DISINT

    MRS     r2, SPSR           // get the return PSR to store away
    SUB     r1, lr, #4         // get the aborted instruction address
    STR     r1 , [r0,#52]      // store return PC

    STMFD   r0!, {r2}          // store PSR

    ADD     r2, r0, #68

    MSR     CPSR_c, #svc_mode|CPSR_DISINT       // switch to supervisor mode
    LDR     sp, =__stack

    // r0 = context
    // r1 = instr
    // r2 = sp at aborted location

    B       FUNC_NAME(handlePrefetchAbort)


