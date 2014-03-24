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

#include <types.hh>
#include <process/Thread.hh>
/**************************************
 *  Syscall related assembler defines
 *************************************/

#define PROCESSOR_CONTEXT_SIZE 140

/*
 * \brief assembler function setting the return value for system calls
 *
 * sp_int: The stack pointer at interruption
 * retval: The return value of the syscall
 */
#define SET_RETURN_VALUE(sp_int,retval) \
										asm volatile ( \
										"   stw   %1  , -116(%0);" \
										: \
										: "b" (sp_int), "r" (retval) \
										: \
									)


#define GET_METHOD_RETURN_VALUE(variable) \
                                        asm volatile( \
                                        "mr %0,%%r3;" \
                                        : "=&r" (variable) \
                                        : \
                                        : \
                                    )

// syscall number stored in r3 on context
#define GET_SYSCALL_NUM(sp_int,syscallnum) \
										   asm volatile(\
										   "lwz    %0,-116(%1);"\
										   : "=&r" (syscallnum)\
										   : "b" (sp_int)\
										   :\
										   )


#define SYSCALLGETPARAMS1(int_sp,param1) \
											asm volatile(\
											"lwz  %0,-112(%1);"\
											: "=&r" (param1)\
											: "b" (int_sp)\
											:\
											)



#define SYSCALLGETPARAMS2(int_sp,param1,param2) \
											asm volatile(\
											"lwz  %0,-112(%2);"\
											"lwz  %1,-108(%2);"\
											: "=&r" (param1), "=&r" (param2)\
											: "b" (int_sp)\
											:\
											)



#define SYSCALLGETPARAMS3(int_sp,param1,param2,param3) \
											asm volatile(\
											"lwz  %0,-112(%3);"\
											"lwz  %1,-108(%3);"\
											"lwz  %2,-104(%3);"\
											: "=&r" (param1), "=&r" (param2) , "=&r" (param3)\
											: "b" (int_sp)\
											:\
											)



#define SYSCALLGETPARAMS4(int_sp,param1,param2,param3,param4) \
											asm volatile(\
											"lwz  %0,-112(%4);"\
											"lwz  %1,-108(%4);"\
											"lwz  %2,-104(%4);"\
											"lwz  %3,-100(%4);"\
											: "=&r" (param1), "=&r" (param2) , "=&r" (param3)  , "=&r" (param4)\
											: "b" (int_sp)\
											:\
											)


#define SYSCALLGETPARAMS5(int_sp,param1,param2,param3,param4,param5) \
											asm volatile(\
											"lwz  %0,-112(%5);"\
											"lwz  %1,-108(%5);"\
											"lwz  %2,-104(%5);"\
											"lwz  %3,-100(%5);"\
											"lwz  %4,-96(%5);"\
											: "=&r" (param1), "=&r" (param2) , "=&r" (param3)  , "=&r" (param4) , "=&r" (param5)\
											: "b" (int_sp)\
											:\
											)

#define SYSCALLGETPARAM1(int_sp,param1) \
											asm volatile(\
											"lwz  %0,-112(%1);"\
											: "=&r" (param1)\
											: "b" (int_sp)\
											:\
											)

#define SYSCALLGETPARAM2(int_sp,param2) \
											asm volatile(\
											"lwz  %0,-108(%1);"\
											: "=&r" (param2)\
											: "b" (int_sp)\
											:\
											)

#define SYSCALLGETPARAM3(int_sp,param3) \
											asm volatile(\
											"lwz  %0,-104(%1);"\
											: "=&r" (param3)\
											: "b" (int_sp)\
											:\
											)

#define SYSCALLGETPARAM4(int_sp,param4) \
											asm volatile(\
											"lwz  %0,-100(%1);"\
											: "=&r" (param4)\
											: "b" (int_sp)\
											:\
											)

#define SYSCALLGETPARAM5(int_sp,param5) \
											asm volatile(\
											"lwz  %0,-96(%1);"\
											: "=&r" (param5)\
											: "b" (int_sp)\
											:\
											)

/*!
 * \brief Issues a system call. only use inside a thread
 */
#define SYSCALL(ret,syscallnumber) \
	 asm volatile(   \
	    "sc;"\
		"lwz %0,-116(%%r1);"\
		: "=r" (ret)\
		: "r" (syscallnumber)\
	    )




//! No operation command
#define NOP asm("nop")


/*!
 *  Sets the pid register
 *  e.g. used in ARP, WorkerThread to allow a thread running in superuser mode to access data of another task given by pid
 *
 */

#define SETPID(pid) \
	asm volatile(\
			"mtspr 945,%0;"\
			"isync;"\
			: \
			: "b" (pid)\
			)



#define GETPID(pid) \
                asm volatile(\
                "mfpid  %0;"\
                : "=&r" (pid)\
                :\
                :\
                )

/*!
 *  Sets the stack pointer to the given address
 *  Used in WorkerThread::callMain() to set the stack the workerthread will work with.
 */

#define SETSTACKPTR(stack_addr) \
	asm volatile(\
			"mr   	%%r1, %0;"\
			: \
			: "b" (stack_addr)\
			)\


/*!
 * Resets the stack pointer to the address right after the context save based on the sp address at interruption.
 * Needed whenever the kernel wants to save space on the stack of a thread.
 * e.g: SingleCPUDispatcher before running the idle thread. The stack frames used to get to that point are not needed any more
 * so they are discarded by using this macro.
 */
#define _resetStackPtr(sp_int)\
	asm volatile(\
			"mr		%%r1,	%0;"\
			"addi   %%r1,%%r1,-172;"\
			: \
			: "b" (sp_int)\
			:)\


//! Calls the method specifed by the 'methodcall' string and sets the this pointer to 'object'
#define CALLMETHOD(objectptr,methodcall) \
	asm volatile(\
			"mr 	%%r3,%0;"  \
			methodcall \
			: \
			: "b" (objectptr) \
			:)


/*!
 * Macro used to update the kernel stack bucket bitmap whenever a thread
 * does not use the kernel stack slot any more. The corresponding bit of that slot
 * will then be unset so the slot is free for another thread.
 */

#define FREE_KERNEL_STACK_SLOT(myBucketIndex) \
 asm volatile(\
           ".extern stackBucketBitmap;"\
           "lis         %%r2,(stackBucketBitmap)@h;"\
           "ori         %%r2,%%r2,(stackBucketBitmap)@l;"\
            /* load the value of the stackBucketBitmap into r3*/ \
           "lwz         %%r3,0(%%r2);"\
           "li          %%r5,1;"\
           "slw         %%r5,%%r5,%0;" /*mask */\
           "not         %%r5,%%r5;"\
           "and         %%r3,%%r3,%%r5;" /*update bitmap*/\
           "stw         %%r3,0(%%r2);" /* store back*/\
           :\
           : "r" (myBucketIndex)\
           : "2","3","5"\
           )

/*!
 * Macro used to call the WorkerThread::work() method for a given WorkerThread object specified by objectptr.
 * Used inside WorkerThread::callMain().
 */

#define BRANCHTO_WORKERTHREAD_WORK(objectptr,pid, stack_addr) \
		asm volatile(\
					"mr    %%r1, %0;"\
					"mr    %%r3, %2;"  \
					"mtspr 945, %1;"\
					"isync;"\
					"b _ZN12WorkerThread4workEv;" \
					: \
					: "b" (stack_addr), "b" (pid), "b" (objectptr) \
					)\



#define GET_INTERRUPT_ENABLE_BIT(var) \
    asm volatile(\
            "mfmsr  %0;" \
            "andi.  %0,%0,0x8000;" \
            "srwi   %0,%0,15;" \
            : "=&r" (var)\
            : \
            )


//-----------------------------------------------------------------------------
// Enabling/Disabling Interupts
//-----------------------------------------------------------------------------


/// Enable Interrupts
#if QEMU_HACK
#define _enableInterrupts() asm volatile( \
                                "mfmsr 11;" \
                                "ori 11,11,0x8000;" \
                                "oris 11,11,0x8000@h;" \
                                "mtmsr 11;" \
                                "sync;" \
                                : \
                                : \
                                : "11" \
                                 \
                            )
#else //QEMU_HACK
#define _enableInterrupts() asm volatile( \
                                "wrteei 0x1;" \
                                "sync;" \
                                : \
                                : \
                                 \
                            )
#endif //QEMU_HACK

/// Disable interrupts.
#define _disableInterrupts() asm volatile( \
                                "wrteei 0x0;" \
                                "sync;" \
                                : \
                                : \
                                 \
                            )



#if ENABLE_NESTED_INTERRUPTS

// Dsiables irqs and saves the irq enable bit to the variable
#define DISABLE_IRQS(irqstatus) \
    bool irqstatus; \
    GET_INTERRUPT_ENABLE_BIT(irqstatus); \
    _disableInterrupts();

#define RESTORE_IRQS(irqstatus) if ( irqstatus ) { _enableInterrupts(); }


#else

#define DISABLE_IRQS(irqstatus)
#define RESTORE_IRQS(irqstatus)
#endif


#define GET_RETURN_CONTEXT(t,sp_int)  sp_int = t->threadStack.stackptrs[t->threadStack.top-1]

#define SAVE_CONTEXT_AT(mem_loc) \
    asm volatile \
    ( \
        "stmw        0,28(%0);" \
        "mfxer       %%r25;" \
        "mfctr       %%r26;" \
        "lis         %%r27,returnof@h;" \
        "ori         %%r27,%%r27,returnof@l;" \
        "mfmsr       %%r28;" \
        "mfcr        %%r29;" \
        "mflr        %%r30;" \
        "mfspr       %%r31,945;" \
        "stmw        25,0(%0);" \
        "lmw         %%r25,128(%0);" \
        : \
        : "a"(mem_loc) \
        : \
    );


// guaranetees that instructions will be executed without beeing interrupted without using a mutes
#define ATOMAR(statements) \
        { bool int_enabled; \
        GET_INTERRUPT_ENABLE_BIT(int_enabled); \
        _disableInterrupts(); \
        statements; \
        if ( int_enabled ) { _enableInterrupts(); } }


// This namemspace will hold all assembler functions needed by non architecture OS classes.
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
