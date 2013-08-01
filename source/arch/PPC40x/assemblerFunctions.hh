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

/*
 * \brief assembler function setting the return value for system calls
 *
 * sp_int: The stack pointer at interruption
 * retval: The return value of the syscall
 */
#if defined(__GNUC__)
#define SET_RETURN_VALUE(sp_int,retval) \
										asm volatile ( \
										"   stw   %1  , -116(%0);" \
										: \
										: "b" (sp_int), "r" (retval) \
										: \
									)
#elif defined(__WINDRIVER__)
asm volatile void SET_RETURN_VALUE(void* sp_int,void* retval)
{
%reg    sp_int,retval

    stw     retval, -116(sp_int)
}

#endif


#define GET_METHOD_RETURN_VALUE(variable) \
                                        asm volatile( \
                                        "mr %0,%%r3;" \
                                        : "=&r" (variable) \
                                        : \
                                        : \
                                    )

// syscall number stored in r3 on context
#if defined(__GNUC__)
#define GET_SYSCALL_NUM(sp_int,syscallnum) \
										   asm volatile(\
										   "lwz    %0,-116(%1);"\
										   : "=&r" (syscallnum)\
										   : "b" (sp_int)\
										   :\
										   )
#elif defined(__WINDRIVER__)
asm volatile void GET_SYSCALL_NUM(int4 sp_int,void* syscallnum)
{
%reg    sp_int,syscallnum

    lwz     syscallnum, -116(sp_int)
}

#endif

#if defined(__GNUC__)
#define SYSCALLGETPARAMS1(int_sp,param1) \
											asm volatile(\
											"lwz  %0,-112(%1);"\
											: "=&r" (param1)\
											: "b" (int_sp)\
											:\
											)
#elif defined(__WINDRIVER__)
asm volatile void SYSCALLGETPARAMS1(int4 int_sp,void* param1)
{
%reg    int_sp,param1

    lwz     param1, -112(int_sp)
}

#endif

#if defined(__GNUC__)
#define SYSCALLGETPARAMS2(int_sp,param1,param2) \
											asm volatile(\
											"lwz  %0,-112(%2);"\
											"lwz  %1,-108(%2);"\
											: "=&r" (param1), "=&r" (param2)\
											: "b" (int_sp)\
											:\
											)
#elif defined(__WINDRIVER__)
asm volatile void SYSCALLGETPARAMS2(int4 int_sp,void* param1,void* param2)
{
%reg    int_sp,param1,param2

    lwz     param1, -112(int_sp)
    lwz     param2, -108(int_sp)
}

#endif

#if defined(__GNUC__)
#define SYSCALLGETPARAMS3(int_sp,param1,param2,param3) \
											asm volatile(\
											"lwz  %0,-112(%3);"\
											"lwz  %1,-108(%3);"\
											"lwz  %2,-104(%3);"\
											: "=&r" (param1), "=&r" (param2) , "=&r" (param3)\
											: "b" (int_sp)\
											:\
											)
#elif defined(__WINDRIVER__)
asm volatile void SYSCALLGETPARAMS3(int4 int_sp,void* param1,void* param2, void*param3)
{
%reg    int_sp,param1,param2,param3

    lwz     param1, -112(int_sp)
    lwz     param2, -108(int_sp)
    lwz     param3, -104(int_sp)
}

#endif

#if defined(__GNUC__)
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
#elif defined(__WINDRIVER__)
asm volatile void SYSCALLGETPARAMS4(int4 int_sp,void* param1,void* param2, void* param3, void* param4)
{
%reg    int_sp,param1,param2,param3,param4

    lwz     param1, -112(int_sp)
    lwz     param2, -108(int_sp)
    lwz     param3, -104(int_sp)
    lwz     param4, -100(int_sp)
}

#endif

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
#if defined(__GNUC__)
#define SYSCALL(ret,syscallnumber) \
	 asm volatile(   \
	    "sc;"\
		"lwz %0,-116(%%r1);"\
		: "=r" (ret)\
		: "r" (syscallnumber)\
	    )
#elif defined(__WINDRIVER__)
asm volatile void SYSCALL(int ret,int syscallnumber)
{
%reg    ret,syscallnumber;

    sc
    lwz     ret,-116(r1)
}
#endif



//! No operation command
#define NOP asm("nop")


/*!
 *  Sets the pid register
 *  e.g. used in ARP, WorkerThread to allow a thread running in superuser mode to access data of another task given by pid
 *
 */
#if defined(__GNUC__)
#define SETPID(pid) \
	asm volatile(\
			"mtspr 945,%0;"\
			"isync;"\
			: \
			: "b" (pid)\
			)
#elif  defined(__WINDRIVER__)
asm volatile void SETPID(int pid)
{
%reg    pid

    mtspr   945,pid
    isync
}

#endif


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
#if defined(__GNUC__)

#define SETSTACKPTR(stack_addr) \
	asm volatile(\
			"mr   	%%r1, %0;"\
			: \
			: "b" (stack_addr)\
			)\

#elif  defined(__WINDRIVER__)

asm volatile void SETSTACKPTR(void* stack_addr)
{
%reg    stack_addr

    mr      r1,stack_addr
    isync
}
#endif
/*!
 * Resets the stack pointer to the address right after the context save based on the sp address at interruption.
 * Needed whenever the kernel wants to save space on the stack of a thread.
 * e.g: SingleCPUDispatcher before running the idle thread. The stack frames used to get to that point are not needed any more
 * so they are discarded by using this macro.
 */
#if defined(__GNUC__)

#define _resetStackPtr(sp_int)\
	asm volatile(\
			"mr		%%r1,	%0;"\
			"addi   %%r1,%%r1,-172;"\
			: \
			: "b" (sp_int)\
			:)\

#elif defined(__WINDRIVER__)
asm volatile void _resetStackPtr(void* sp_int)
{
%reg sp_int

    mr      r1,sp_int
    addi    r1,r1,-200
}
#endif

//! Calls the method specifed by the 'methodcall' string and sets the this pointer to 'object'
#if defined(__GNUC__)
#define CALLMETHOD(objectptr,methodcall) \
	asm volatile(\
			"mr 	%%r3,%0;"  \
			methodcall \
			: \
			: "b" (objectptr) \
			:)
#elif defined(__WINDRIVER__)
asm volatile void CALLMETHOD(void* objectptr)
{
%reg    objectptr

    mr  r3,objectptr
    //b   _ZN12WorkerThread4workEv
}
#endif

/*!
 * Macro used to update the kernel stack bucket bitmap whenever a thread
 * does not use the kernel stack slot any more. The corresponding bit of that slot
 * will then be unset so the slot is free for another thread.
 */
#if defined(__GNUC__)
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
#elif defined(__WINDRIVER__)
asm volatile void FREE_KERNEL_STACK_SLOT(int mybucketIndex)
{
%reg mybucketIndex
!"r2","r3","r5"

    .extern stackBucketBitmap
    lis         r2,(stackBucketBitmap)@h;
    ori         r2,r2,(stackBucketBitmap)@l
    /* load the value of the stackBucketBitmap into r3*/ \
    lwz         r3,0(r2)
    li          r5,1
    slw         r5,r5,mybucketIndex
    not         r5,r5
    and         r3,r3,r5
    stw         r3,0(r2)
}

#endif

/*!
 * Macro used to call the WorkerThread::work() method for a given WorkerThread object specified by objectptr.
 * Used inside WorkerThread::callMain().
 */
#if defined(__GNUC__)
#define BRANCHTO_WORKERTHREAD_WORK(objectptr) CALLMETHOD(objectptr,"b _ZN12WorkerThread4workEv;")
#elif defined(__WINDRIVER__)
#define BRANCHTO_WORKERTHREAD_WORK(objectptr) CALLMETHOD(objectptr)
#endif

#if defined(__GNUC__)
#define GET_INTERRUPT_ENABLE_BIT(var) \
    asm volatile(\
            "mfmsr  %0;" \
            "andi.  %0,%0,0x8000;" \
            "srwi   %0,%0,15;" \
            : "=&r" (var)\
            : \
            )
#elif defined(__WINDRIVER__)

asm volatile void GET_INTERRUPT_ENABLE_BIT(int var)
{
%reg var

    mfmsr   var
    andi.   var,var,0x8000
    srwi    var,var,15
}
#endif

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
