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

#ifndef ASSEMBLERFUNCTIONS_HH_
#define ASSEMBLERFUNCTIONS_HH_

#ifdef __cplusplus
#include "SCLConfig.hh"
#include "ARMv4T.h"
#include "inc/types.hh"

class Thread;
#endif

extern void* __PageTableSec_start;

/**************************************
 *  Syscall related assembler defines
 *************************************/

/*
 * \brief assembler function setting the return value for system calls
 *
 * sp_int: The stack pointer at interruption
 * retval: The return value of the syscall
 */
#define SET_RETURN_VALUE(sp_int, retval) \
                                        asm volatile(\
                                        "str %1, [%0, #4];" \
                                        : \
                                        : "l" (sp_int), "r" (retval) \
                                        : \
                                    )

/*
 * \brief Gets the current top return context stack pointer of Thread t into sp_int
 *
 *TODO: we need to be sure this is the stack address the context from user space was
 *         saved at.. this will not work with nested interrupts otherwise!
 *
 * sp_int: The return stack pointer
 * t     : The Thread
 **/
#define GET_RETURN_CONTEXT(t, sp_int)  sp_int = t->threadStack.stackptrs[t->threadStack.top-2]

#define GET_METHOD_RETURN_VALUE(variable) \
                                        asm volatile( \
                                        "mov %0, r1;" \
                                        : "=&r" (variable) \
                                        : \
                                        : \
                                    )

#define GET_SYSCALL_NUM(sp_int, syscallnum) \
                                           asm volatile(\
                                           "ldr %0, [%1, #4];"\
                                           : "=&r" (syscallnum)\
                                           : "r" (sp_int)\
                                           :\
                                           )

#define SYSCALLGETPARAMS1(int_sp, param1) \
                                            asm volatile(\
                                            "ldr %0, [%1, #8];"\
                                            : "=&l" (param1)\
                                            : "r" (int_sp)\
                                            :\
                                            )

#define SYSCALLGETPARAMS2(int_sp, param1, param2) \
                                            asm volatile(\
                                            "ldr %0, [%2, #8];"\
                                            "ldr %1, [%2, #12];"\
                                            : "=&l" (param1), "=&l" (param2) \
                                            : "r" (int_sp)\
                                            :\
                                            )

#define SYSCALLGETPARAMS3(int_sp, param1, param2, param3) \
                                            asm volatile(\
                                            "ldr %0, [%3, #8];"\
                                            "ldr %1, [%3, #12];"\
                                            "ldr %2, [%3, #16];"\
                                            : "=&l" (param1), "=&l" (param2), "=&l" (param3) \
                                            : "r" (int_sp)\
                                            :\
                                            )

#define SYSCALLGETPARAMS4(int_sp, param1, param2, param3, param4) \
                                            asm volatile(\
                                            "ldr %0, [%4, #8];"\
                                            "ldr %1, [%4, #12];"\
                                            "ldr %2, [%4, #16];"\
                                            "ldr %3, [%4, #68];"\
                                            : "=&l" (param1), "=&l" (param2), "=&l" (param3), "=&l" (param4) \
                                            : "r" (int_sp)\
                                            :\
                                            )

#define SYSCALLGETPARAMS5(int_sp, param1, param2, param3, param4, param5) \
                                            asm volatile(\
                                            "ldr %0, [%5, #8];"\
                                            "ldr %1, [%5, #12];"\
                                            "ldr %2, [%5, #16];"\
                                            "ldr %3, [%5, #68];"\
                                            "ldr %4, [%5, #72];"\
                                            : "=&l" (param1), "=&l" (param2), "=&l" (param3), "=&l" (param4), "=&r" (param5) \
                                            : "r" (int_sp)\
                                            :\
                                            )

#define SYSCALLGETPARAMS6(int_sp, param1, param2, param3, param4, param5, param6) \
                                            asm volatile(\
                                            "ldr %0, [%6, #8];"\
                                            "ldr %1, [%6, #12];"\
                                            "ldr %2, [%6, #16];"\
                                            "ldr %3, [%6, #68];"\
                                            "ldr %4, [%6, #72];"\
                                            "ldr %5, [%6, #76];"\
                                            : "=&l" (param1), "=&l" (param2), "=&l" (param3), "=&l" (param4), "=&r" (param5), "=&r" (param6) \
                                            : "r" (int_sp)\
                                            :\
                                            )

#define SYSCALLGETPARAM1(int_sp, param1) \
                                            asm volatile(\
                                            "ldr %0, [%1, #8];"\
                                            : "=&l" (param1)\
                                            : "r" (int_sp)\
                                            :\
                                            )

#define SYSCALLGETPARAM2(int_sp, param2) \
                                            asm volatile(\
                                            "ldr %0, [%1, #12];"\
                                            : "=&l" (param1)\
                                            : "r" (int_sp)\
                                            :\
                                            )

#define SYSCALLGETPARAM3(int_sp, param3) \
                                            asm volatile(\
                                            "ldr %0, [%1, #16];"\
                                            : "=&l" (param1)\
                                            : "r" (int_sp)\
                                            :\
                                            )

#define SYSCALLGETPARAM4(int_sp, param4) \
                                            asm volatile(\
                                            "ldr %0, [%1, #68];"\
                                            : "=&l" (param1)\
                                            : "r" (int_sp)\
                                            :\
                                            )

#define SYSCALLGETPARAM5(int_sp, param5) \
                                            asm volatile(\
                                            "ldr %0, [%1, #72];"\
                                            : "=&l" (param1)\
                                            : "r" (int_sp)\
                                            :\
                                            )

//! No operation command
#define NOP asm("nop")

/*!
 * Activates virtual memory map
 * named SETPID because PPC does this by setting the pid
 * for ARM
 * - flush TLB,
 * - set PID in CONTEXTIDR register and
 * - set set page table base register
 */
#if HAS_Board_HatLayerCfd
#define SETPID(pid) \
           __asm__ __volatile__(    \
               "MOV r0, #0;"    \
               "MCR p15, 0, r0, c8, c5, 0;" \
               "MCR p15, 0, r0, c8, c6, 0;" \
               "MOV r1, %1;" \
               "ORR r1, r1, %1, lsl #8;" \
               "MCR p15,0, r1,c13,c0,1;"    \
               "MCR p15,0, r0, c7, c5, 4;" \
               "MOV r1, #0x4000;"    \
               "MUL r1, %1, r1;"    \
               "ADD r0, %0, r1;"    \
               "MCR p15, 0, r0, c2, c0, 0;"    \
               "MOV r0, #0; " \
               "MCR p15, 0, r0, c7, c5, 4;" \
               :     \
               : "r" (&__PageTableSec_start), "r" (pid)    \
               : "r0", "r1" \
           )

#define GETPID(pid) \
    asm volatile(\
        "MRC p15, 0, %0, c13, c0, 3;"\
        "AND %0,%0,#255;" \
        : "=&r" (pid)\
        :\
        : "r0"\
        )
#else
#define SETPID(pid)
#define GETPID(pid)
#endif
/*!
 *  Sets the stack pointer to the given address
 *  Used in WorkerThread::callMain() to set the stack the workerthread will work with.
 */
#define SETSTACKPTR(stack_addr) \
    asm volatile(\
        "mov sp, %0;"\
        : \
        : "r" (stack_addr)\
        :)

#define GETSTACKPTR(variable) \
        asm volatile(\
            "mov %0, sp;"\
            : "=&r" (variable) \
            : \
            :)

#define GETLR(variable) \
        asm volatile(\
            "mov %0, lr;"\
            : "=&r" (variable) \
            : \
            :)

#define GETPC(variable) \
        asm volatile(\
            "mov %0, pc;"\
            : "=&r" (variable) \
            : \
            :)

/*!
 * Macro used to call the WorkerThread::work() method for a given WorkerThread object specified by objectptr.
 * Used inside WorkerThread::callMain().
 */
#define BRANCHTO_WORKERTHREAD_WORK(objectptr, pid, stack_addr) \
                asm volatile(\
                        "mov sp, %2;"\
                        "MOV r0, #0;"    \
                        "MCR p15, 0, r0, c8, c5, 0;" \
                        "MCR p15, 0, r0, c8, c6, 0;" \
                        "MOV r1, %1;" \
                        "ORR r1, r1, %1, lsl #8;" \
                        "MCR p15,0, r1,c13,c0,1;"    \
                        "MCR p15,0, r0, c7, c5, 4;" \
                        "MOV r1, #0x4000;"    \
                        "MUL r1, %1, r1;"    \
                        "ADD r0, %0, r1;"    \
                        "MCR p15, 0, r0, c2, c0, 0;"    \
                        "MOV r0, #0; " \
                        "MCR p15, 0, r0, c7, c5, 4;" \
                        "mov r0, %3;" \
                        "b _ZN12KernelThread4workEv;" \
                        : \
                        : "r" (&__PageTableSec_start), "r" (pid) , "r" (stack_addr), "r" (objectptr)\
                        : "r0", "r1" , "memory")


// get interrupt enable bit (IRQ only)
#define GET_INTERRUPT_ENABLE_BIT(var) \
                            asm volatile( \
                                    "MRS    %0, cpsr;" \
                                    "AND    %0, %0, #0x80;" \
                                    : "=&r" (var)\
                                    : \
                                    : \
                                    )
//-----------------------------------------------------------------------------
// Enabling/Disabling Interrupts
//-----------------------------------------------------------------------------

// Enable interrupts (IRQ).
#define _enableInterrupts() asm volatile("CPSIE i" : : : "memory");


// Disable interrupts (IRQ).
#define _disableInterrupts() asm volatile("CPSID i" : : : "memory");


#define GET_CPSR(var) asm volatile("mrs  %0, CPSR;" : "=&r" (var):  :)


// Disables irqs and saves the irq enable bit to the variable
#define DISABLE_IRQS(irqstatus) \
    GET_INTERRUPT_ENABLE_BIT(irqstatus); \
    _disableInterrupts();

#define RESTORE_IRQS(irqstatus) if (!(irqstatus & 0x80)) {_enableInterrupts();}


// supervisor mode: 19, system mode: 31, disable interrupts: 0xC0
#define SAVE_CONTEXT_AT(mem_loc, mode) asm volatile \
   ( \
       ".align  4;" \
       "mov     r0, pc;" \
       "bx      r0;" \
       ".code   32;" \
       "mov     r0, %0;"\
       "add     r0, r0,#68;" \
       "str     lr, [r0,#-4];" \
       "str     sp, [r0,#-8];" \
       "sub     r0, r0,#12;" \
       "stmfd   r0!, {r0-r12};" \
       "ldr     r1, =returnof;" \
       "str     r1, [r0,#52];" \
       "mov     r1, %1;" \
       "orr     r1, r1,#0x20;" \
       "stmfd   r0!,{r1};" \
       "add     r0, pc,#1;" \
        "bx     r0;" \
        ".code 16;" \
       : \
       : "r"(mem_loc), "r" (mode) \
       : "r0", "r1", "r3" , "r4");

// guaranetees that instructions will be executed without being interrupted without using a mutex
#define ATOMAR(statements) \
        { DISABLE_IRQS(irqstatus); \
          statements; \
          RESTORE_IRQS(irqstatus); }


/*****************************************************************************
 * Method: testandset(void* address, int testvalue, int setvalue)
 *
 * @description
 * Tries to set 'setvalue' at address 'address'. Before setting the value
 * it tests the address on testvalue. If the address contains testvalue
 * or setting setvalue fails (due to concurrent access) the function returns 0.
 * On exclusively and successfully setting setvalue the function returns 1.
 * This function is SMP safe.
 *******************************************************************************/
static int inline testandset(void* address, int testvalue, int setvalue) {
    int result;
    /* smp test and set for ARM >= v6 */
    asm volatile(
            "LDREX     r0, [%1];"       // load the address value
            "CMP       r0, %2;"         // compare with test value
            "ITT       EQ;"             // if then then (2 conditional instr following)
            "STREXEQ   r0, %3, [%1];"   // try to store the set value and get success to r0
            "CMPEQ     r0, #0;"         // did it succeed?
            "ITE       EQ;"             // if then else
            "MOVEQ     %0, #1;"         // return value == 1 on success
            "MOVNE     %0, #0;"         // return value == 0 on failure
            : "=&r" (result)
            : "r" (address) , "r" (testvalue) , "r" (setvalue)
            : "r0"
    );
    return (result);
}


/*
 * SMP Capable atomic addition operation.
 */
static void inline ATOMIC_ADD(void* addr, int value) {
        asm volatile(
          "1: "
          "LDREX r1, [%0];"
          "ADD   r1, %1;"
          "STREX r2, r1, [%0];"
          "CMP   r2, #0;"
          "BNE   1b;"
          "DMB;"
           :
           : "r" (addr), "r" (value)
           : "r1", "r2");
}

#if NUM_CPUS > 1

/*
 * SMP capable spinlock. Enforces disable interrupts to ensure
 * the local thread does not get preempted. Uses
 * an smp spinlock to check if another processor is currently using this
 * spinlock.
 *
 * This SMP_SPINLOCK is best used for SHORT exclusive access areas!
 * If interrupts would stay enabled other processors might spin for a very long time
 * or another process might preempt us leading to a deadlock if it is using the same
 * spinlock.
 */
#define SMP_SPINLOCK_GET(spinlock) \
    asm volatile ( \
         "MRS     r1,   cpsr_c;"    \
         "CPSID   i;" \
         "1:     " \
         "LDREX   r2,  [%0];"     /* get spinlock value. try to get exclusive access */ \
         "CMP     r2,  #0;"       /* test if locked.. */  \
         "BNE     1b;"            /* locked .. spin until unlocked */ \
         "STREX   r2,  r1, [%0];" /* not locked .. try to lock it..*/ \
         "CMP     r2,  #1;"       /* check if STREX failed.. */ \
         "BEQ     1b;"            /* if failed restart from 1 */ \
         "DMB;" \
          : \
          : "r" (&spinlock) \
          : "r1", "r2" , "memory");


/*
 * Frees the spinlock giving other processors or threads the access to its. Also restores
 * the interrupt enable/disable state at its corresponding SMP_SPINLOCK_GET call.
 */
#define SMP_SPINLOCK_FREE(spinlock) \
        asm volatile ( \
          "LDREX  r1, [%0];"     /* get spinlock value. contains irq status */ \
          "MRS    r2, cpsr_c;"      \
          "AND    r1, r1, #0x80;"   \
          "ORR    r2, r1;"          \
          "MSR    cpsr_c, r2;"      \
          "MOV    r1, #0;"          \
          "STR    r1, [%0];" /* set spinlock value to 0 */\
           : \
           : "r" (&spinlock) \
           : "r1", "r2");

// be sure different cpus se correct order
#define SMP_MEM_BARRIER()       asm volatile ("dmb ish" : : : "memory")
#define SMP_MEM_BARRIER_FULL()  asm volatile ("dmb"     : : : "memory")

#else  // CPUS == 1

// memory barrier not needed only avoid compiler reordering
#define SMP_MEM_BARRIER()       asm volatile ("" : : : "memory")
#define SMP_MEM_BARRIER_FULL()  asm volatile ("" : : : "memory")


#define SMP_SPINLOCK_GET(spinlock)  DISABLE_IRQS(spinlock)
#define SMP_SPINLOCK_FREE(spinlock) RESTORE_IRQS(spinlock)
#endif

#define MEM_BARRIER()           asm volatile ("dmb ish" : : : "memory")
#define MEM_BARRIER_FULL()      asm volatile ("dmb"     : : : "memory")
#define INSTR_BARRIER()         asm volatile ("dsb ish" : : : "memory")
#define INSTR_BARRIER_FULL()    asm volatile ("dsb"     : : : "memory")

#ifdef __cplusplus

// This namespace will hold all assembler functions needed by non architecture OS classes.
namespace assembler {

/*!
 * Restores the context of a given thread t.
 *
 * Called whenever a thread is restored and thus will execute.
 * Called from SyscallManager to finish a syscall, SingleCPUDisptacher to resume the execution of a preempted thread,
 * and Resource to finish a syscall earlier.
 *
 */
extern "C" void restoreContext(Thread* t, Thread* previousThread);
} // namespace assembler

#endif

#endif
