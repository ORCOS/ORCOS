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

#include "SCLConfig.hh"
#include <ARMv4T.h>
#include <types.hh>
#include <process/Thread.hh>

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
#define SET_RETURN_VALUE(sp_int,retval) \
										asm volatile ( \
										"str %1, [%0, #4];" \
										: \
										: "r" (sp_int), "r" (retval) \
										: \
									)

#define GET_METHOD_RETURN_VALUE(variable) \
                                        asm volatile( \
                                        "mov %0, r1;" \
                                        : "=&r" (variable) \
                                        : \
                                        : \
                                    )

#define GET_SYSCALL_NUM(sp_int,syscallnum) \
										   asm volatile(\
										   "ldr %0, [%1, #4];"\
										   : "=&r" (syscallnum)\
										   : "r" (sp_int)\
										   :\
										   )

#define SYSCALLGETPARAMS1(int_sp,param1) \
											asm volatile(\
											"ldr %0, [%1, #8];"\
											: "=&r" (param1)\
											: "r" (int_sp)\
											:\
											)

#define SYSCALLGETPARAMS2(int_sp,param1,param2) \
											asm volatile(\
											"ldr %0, [%2, #8];"\
											"ldr %1, [%2, #12];"\
											: "=&r" (param1), "=&r" (param2) \
											: "r" (int_sp)\
											:\
											)

#define SYSCALLGETPARAMS3(int_sp,param1,param2,param3) \
											asm volatile(\
											"ldr %0, [%3, #8];"\
											"ldr %1, [%3, #12];"\
											"ldr %2, [%3, #16];"\
											: "=&r" (param1), "=&r" (param2), "=&r" (param3) \
											: "r" (int_sp)\
											:\
											)

#define SYSCALLGETPARAMS4(int_sp,param1,param2,param3,param4) \
											asm volatile(\
											"ldr %0, [%4, #8];"\
											"ldr %1, [%4, #12];"\
											"ldr %2, [%4, #16];"\
											"ldr %3, [%4, #76];"\
											: "=&r" (param1), "=&r" (param2), "=&r" (param3), "=&r" (param4) \
											: "r" (int_sp)\
											:\
											)

#define SYSCALLGETPARAMS5(int_sp,param1,param2,param3,param4,param5) \
											asm volatile(\
											"ldr %0, [%5, #8];"\
											"ldr %1, [%5, #12];"\
											"ldr %2, [%5, #16];"\
											"ldr %3, [%5, #76];"\
											"ldr %4, [%5, #80];"\
											: "=&r" (param1), "=&r" (param2), "=&r" (param3), "=&r" (param4), "=&r" (param5) \
											: "r" (int_sp)\
											:\
											)

#define SYSCALLGETPARAM1(int_sp,param1) \
											asm volatile(\
											"ldr %0, [%1, #8];"\
											: "=&r" (param1)\
											: "r" (int_sp)\
											:\
											)

#define SYSCALLGETPARAM2(int_sp,param2) \
											asm volatile(\
											"ldr %0, [%1, #12];"\
											: "=&r" (param1)\
											: "r" (int_sp)\
											:\
											)

#define SYSCALLGETPARAM3(int_sp,param3) \
											asm volatile(\
											"ldr %0, [%1, #16];"\
											: "=&r" (param1)\
											: "r" (int_sp)\
											:\
											)

#define SYSCALLGETPARAM4(int_sp,param4) \
											asm volatile(\
											"ldr %0, [%1, #76];"\
											: "=&r" (param1)\
											: "r" (int_sp)\
											:\
											)

#define SYSCALLGETPARAM5(int_sp,param5) \
											asm volatile(\
											"ldr %0, [%1, #80];"\
											: "=&r" (param1)\
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
#ifdef HAS_MemoryManager_HatLayerCfd
#define SETPID(pid) \
       	__asm__ __volatile__(	\
       		"MOV r0, #0;"	\
       		"MCR p15, 0, r0, c8, c5, 0;" \
       		"MCR p15, 0, r0, c8, c6, 0;" \
       		"MOV r1, %1;" \
       		"ORR r1, r1, %1, lsl #8;" \
       		"MCR p15,0, r1,c13,c0,1;"	\
       		"MCR p15,0, r0, c7, c5, 4;" \
       		"MOV r1, #0x4000;"	\
       		"MUL r1, %1, r1;"	\
       		"ADD r0, %0, r1;"	\
       		"MCR p15, 0, r0, c2, c0, 0;"	\
       		"MOV r0, #0; " \
       		"MCR p15, 0, r0, c7, c5, 4;" \
       		: 	\
       		: "r" (&__PageTableSec_start), "r" (pid)	\
       		: "r0", "r1" \
       	)

#define GETPID(pid) \
	asm volatile(\
		"MRC p15, 0, %0, c13, c0, 3;"\
		"AND %0,%0,#255;" \
		: "=&r" (pid)\
		:\
		:\
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
		: \
		)\

/*!
 * Resets the stack pointer to the address right after the context save based on the sp address at interruption.
 * Needed whenever the kernel wants to save space on the stack of a thread.
 * e.g: SingleCPUDispatcher before running the idle thread. The stack frames used to get to that point are not needed any more
 * so they are discarded by using this macro.
 */
#define _resetStackPtr(sp_int) /*\
	asm volatile(\
			"mov sp, %0;"\
			: \
			: "r" (sp_int)\
			:)\*/

//! Calls the method specifed by the 'methodcall' string and sets the this pointer to 'object'
#define CALLMETHOD(objectptr,methodcall) \
	asm volatile(\
			"mov r0, %0;"  \
			methodcall \
			: \
			: "r" (objectptr) \
			: "r0")

/*!
 * Macro used to update the kernel stack bucket bitmap whenever a thread
 * does not use the kernel stack slot any more. The corresponding bit of that slot
 * will then be unset so the slot is free for another thread.
 */
#define FREE_KERNEL_STACK_SLOT(myBucketIndex) /*\
 asm volatile(\
           ";" \
           :\
           : "r" (myBucketIndex)\
           : "2","3","5"\
           )*/
/*!
 * Macro used to call the WorkerThread::work() method for a given WorkerThread object specified by objectptr.
 * Used inside WorkerThread::callMain().
 */
#define BRANCHTO_WORKERTHREAD_WORK(objectptr) CALLMETHOD(objectptr,"b _ZN12WorkerThread4workEv;")

// get interrupt enable bit (IRQ only)
#define GET_INTERRUPT_ENABLE_BIT(var) \
    asm volatile( \
            "MRS	%0, cpsr;" \
            "AND	%0, %0, #0x80;" \
            "LSR	%0, #7;" \
            "EOR	%0, %0, #1;" \
            : "=&r" (var)\
            : \
            : \
            )

//-----------------------------------------------------------------------------
// Enabling/Disabling Interrupts
//-----------------------------------------------------------------------------

// Enable interrupts (IRQ).
#define _enableInterrupts() asm volatile( \
                                "MRS	r0, cpsr;" \
                                "BIC	r0, r0, #0x80;" \
                                "MSR	cpsr, r0;" \
                                : \
                                : \
                                : "r0" \
                            )

// Disable interrupts (IRQ).
#define _disableInterrupts() asm volatile( \
                                "MRS	r0, cpsr;" \
                                "ORR	r0, r0, #0x80;" \
                                "MSR	cpsr, r0;" \
                                : \
                                : \
                                : "r0" \
                            )


// supervisor mode: 19, system mode: 31, disable interrupts: 0xC0
#define SAVE_CONTEXT_AT(mem_loc) asm volatile \
   ( \
	   "mov  r0,%0;"\
	   "add  r0,r0,#72;" \
	   "str  fp,[r0,#-4];" \
	   "str  sp,[r0,#-8];" \
	   "str  lr,[r0,#-12];" \
	   "sub  r0,r0,#16;" \
	   "stmfd r0!,{r0-r12};" \
	   "ldr  r1, =returnof;" \
	   "str	 r1,[r0,#52];" \
	   "mrs  r1,CPSR;" \
	   "sub  r0,r0,#4;" \
	   "str  r1,[r0];" \
       : \
       : "r"(mem_loc) \
       : "r0", "r1" \
   );

// guaranetees that instructions will be executed without being interrupted without using a mutex
#define ATOMAR(statements) \
        { bool int_enabled; \
        GET_INTERRUPT_ENABLE_BIT(int_enabled); \
        _disableInterrupts(); \
        statements; \
        if ( int_enabled ) { _enableInterrupts(); } }



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
extern "C" void restoreContext(Thread* t);
}
#endif
