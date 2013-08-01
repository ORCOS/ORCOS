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
#include <types.hh>
#include <process/Task.hh>
#include "asm/sparc.hh"

/**************************************
 *  Syscall related assembler defines
 **************************************/

/*
 * \brief assembler function setting the return value for system calls
 *
 * sp_int: The stack pointer at interruption
 * retval: The return value of the syscall
 */
#define SET_RETURN_VALUE(sp_int, retval) \
										asm volatile ( \
										"st   %1, [%0 + 32];" \
										: \
										: "r" (sp_int), "r" (retval) \
										: \
										)

#define GET_SYSCALL_NUM(sp_int,syscallnum) \
										   asm volatile(\
										   "ld [%1 + 32], %0;"	\
										   : "=&r" (syscallnum)\
										   : "r" (sp_int)\
										   :\
										   )

#define SYSCALLGETPARAMS1(int_sp,param1)	\
											asm volatile (\
											"ld [%1 + 36], %0;"	\
											: "=&r" (param1) \
											: "r" (int_sp) \
											:\
											)

#define SYSCALLGETPARAMS2(int_sp,param1,param2)\
												asm volatile (\
												"ld [%2 + 36], %0;"	\
												"ld [%2 + 40], %1;"	\
												: "=&r" (param1), "=&r" (param2) \
												: "r" (int_sp) \
												:\
												)

#define SYSCALLGETPARAMS3(int_sp,param1,param2,param3)\
												asm volatile (\
												"ld [%3 + 36], %0;"	\
												"ld [%3 + 40], %1;"	\
												"ld [%3 + 44], %2;"	\
												: "=&r" (param1), "=&r" (param2), "=&r" (param3) \
												: "r" (int_sp) \
												:\
												)

#define SYSCALLGETPARAMS4(int_sp,param1,param2,param3,param4)\
												asm volatile (\
												"ld [%4 + 36], %0;"	\
												"ld [%4 + 40], %1;"	\
												"ld [%4 + 44], %2;"	\
												"ld [%4 + 48], %3;"	\
												: "=&r" (param1), "=&r" (param2), "=&r" (param3), "=&r" (param4) \
												: "r" (int_sp) \
												:\
												)


#define SYSCALLGETPARAMS5(int_sp,param1,param2,param3,param4,param5)\
												asm volatile (\
												"ld [%5 + 36], %0;"	\
												"ld [%5 + 40], %1;"	\
												"ld [%5 + 44], %2;"	\
												"ld [%5 + 48], %3;"	\
												"ld [%5 + 52], %4;"	\
												: "=&r" (param1), "=&r" (param2), "=&r" (param3), "=&r" (param4), "=&r" (param5) \
												: "r" (int_sp) \
												:\
												)

#define SYSCALL(ret,syscallnumber)\
		asm volatile(   \
		    /* uncomment this block for non optimized compilation */ \
			"mov %%i0, %%o0;" \
		    "mov %%i1, %%o1;" \
		    "mov %%i2, %%o2;" \
		    "mov %%i3, %%o3;" \
		    "mov %%i4, %%o4;" \
		    "mov %%i5, %%o5;" \
		    "ta 0x80;"\
		    "mov %%o0, %0;" \
			: "=&r" (ret)\
			: "r" (syscallnumber) \
			:\
		    )

//! No operation command
#define NOP asm("nop")

//! Sets the stack pointer
#define SETSTACKPTR(stack_addr) \
			asm volatile(\
				"mov   	%0, %%sp;"\
				: \
				: "r" (stack_addr)\
				:)\

/*!
 * Resets the stack pointer to the address right after the context save based on the sp address at interruption.
 * Needed whenever the kernel wants to save space on the stack of a thread.
 * e.g: SingleCPUDispatcher before running the idle thread. The stack frames used to get to that point are not needed any more
 * so they are discarded by using this macro.
 */
#define _resetStackPtr(stack_addr) \
asm volatile(\
				"mov   	%0, %%sp;"\
				: \
				: "r" (stack_addr)\
				:)\


#define GET_INTERRUPT_ENABLE_BIT(var) \
    asm volatile(\
            "mov  %%psr,  %0;" \
            "not  %0;" \
            "and  %0, 0xF00, %0;" \
            "srl  %0, 8, %0;" \
            : "=&r" (var)\
            : \
            :)

#define CALLMETHOD(objectptr,methodcall) \
	asm volatile(\
			methodcall \
			"mov %0, %%o0;"	\
			: \
			: "r" (objectptr) \
			: )

#define SETPID(pid) \
	asm volatile( \
				"flush;"	\
				"set 0x81000f, %%g7;"	\
				"sta %%g7, [%%g0] 2;"	\
				"set 0x200, %%g7;" \
				"sta %0, [%%g7] 0x19;" \
				: \
				: "r" (pid) \
				:"%g7" \
				)


#define GETPID(pid) \
	asm volatile( \
				"set 0x200, %%g7;" \
				"lda [%%g7] 0x19, %0;" \
				:"=&r" (pid) \
				: \
				: "%g7"\
			)

/*!
 * Macro used to call the WorkerThread::work() method for a given WorkerThread object specified by objectptr.
 * Used inside WorkerThread::callMain().
 */
#define BRANCHTO_WORKERTHREAD_WORK(objectptr) \
/* If virtual memory is activated, it is necessary to invalidate all register windows
 * because they can contain virtual memory addresses of a previous thread*/	\
   asm volatile( \
				/* computer new WIM on base of current window pointer */ \
 				"rd %%psr, %%g5;" \
 				"set 1, %%g6;" \
 				"sll %%g6, %%g5, %%g6;" \
 				/* rotate WIM one bit to the right */ \
 				/*"srl %%g6, 1, %%g5;" \
 				"sll %%g6, 7, %%g7;" \
 				"or	%%g5, %%g7, %%g6;" */\
 				"wr %%g6, %%wim;" \
 				"nop;nop;nop;" \
 				: \
 				: \
 				: "%g5", "%g6" \
 			); \
CALLMETHOD(objectptr, "call _ZN12WorkerThread4workEv;")


/*
 * -----------------------------------------------------------------------------
 * Enabling/Disabling Interupts
 * -----------------------------------------------------------------------------
 */

/*
 * Enable interrupts by setting the interrupt level to 00
 */
#define _enableInterrupts() asm volatile( \
								"mov 	%%psr, %%l0;"	\
								"mov 	0x00000F00, %%l1;"	\
								"not	%%l1;"\
								"and 	%%l0, %%l1, %%l0;"	\
								"mov 	%%l0, %%psr;"	\
								"nop; nop; nop;" \
								: \
								: \
								: "%l0", "%l1" \
							)


/*
 * disable interrupts by setting the interrupt level to FF
 * ( Note: IRL 15 is unmaskable )
 */
#define _disableInterrupts() asm volatile( \
								"mov 	%%psr, %%l0;"	\
								"mov 	0x00000F00, %%l1;"	\
								"or 	%%l0, %%l1, %%l0;"	\
								"mov 	%%l0, %%psr;"	\
								"nop; nop; nop;" \
								: \
								: \
								: "%l0", "%l1" \
							)

#define FREE_KERNEL_STACK_SLOT(myBucketIndex) \
 asm volatile(\
           ".extern stackBucketBitmap;"\
           "set         stackBucketBitmap, %%g5;" \
            /* load the value of the stackBucketBitmap into g6 */ \
           "ld          [%%g5], %%g6;" \
           "set         1, %%g7;" \
           "sll         %%g7, %0, %%g7;" /*mask */ \
           "not         %%g7;" \
           "and         %%g6,%%g7,%%g6;"  /*update bitmap*/ \
           "st          %%g6,[%%g5];" /* store back*/ \
           :\
           : "r" (myBucketIndex)\
           : "g5","g6","g7"\
           )

/*
 * Get the cpu index
 */
#define GET_CPU_INDEX(index) asm volatile( \
								"mov 	%%asr17, %0;"	\
								"srl %0, 0x1C, %0"	\
								: "=&r" (index)\
								:  \
								: "%l0" \
								)


#define SAVE_CONTEXT_AT(mem_loc) \
    asm volatile \
    ( \
        "save %0, %%g0, %%sp;" \
        "set returnof, %%l1;"   \
        "add %%l1, 4, %%l2;"    \
        "save; save; save; save; save; save;" \
        "restore; restore; restore; restore; restore; restore;" \
        "rd %%psr, %%l0;"       \
        "st  %%l0, [%0 +  0];"   \
        "st  %%l1, [%0 +  4];"   \
        "st  %%l2, [%0 +  8];"   \
        "st  %%l3, [%0 + 12];"   \
        "st  %%l4, [%0 + 16];"   \
        "st  %%l5, [%0 + 20];"   \
        "st  %%l6, [%0 + 24];"   \
        "st  %%l7, [%0 + 28];"   \
        "st  %%i0, [%0 + 32];"   \
        "st  %%i1, [%0 + 36];"   \
        "st  %%i2, [%0 + 40];"   \
        "st  %%i3, [%0 + 44];"   \
        "st  %%i4, [%0 + 48];"   \
        "st  %%i5, [%0 + 52];"   \
        "st  %%i6, [%0 + 56];"   \
        "st  %%i7, [%0 + 60];"   \
        "st  %%g1, [%0 + 80];"   \
        "st  %%g2, [%0 + 84];"   \
        "st  %%g3, [%0 + 88];"   \
        "st  %%g4, [%0 + 92];"   \
        "rd %%y, %%g7;"			 \
        "st %%g7, [%0 + 96];"    \
        "restore;" \
        : \
        : "r"(mem_loc) \
        : \
    );

// Guarantees that instructions will be executed without beeing interrupted without using a mutes
#define ATOMAR(statements) \
        { bool int_enabled; \
        GET_INTERRUPT_ENABLE_BIT(int_enabled); \
        _disableInterrupts(); \
        statements; \
        if ( int_enabled ) { _enableInterrupts(); } }


// This namemspace will hold all assembler functions needed by non architecture OS classes.
namespace assembler{

	extern "C" void readTimeRegister(unint4* regValuePtr);

	extern "C" long testandset( long testValue,  long setValue, void* addr );

	/*!
	 * \brief Restore the context of the thread given by its stack pointer
	 */
	extern "C" void restoreContext(Thread*  t);

}
#endif /* ASSEMBLERFUNCTIONS_HH_ */
