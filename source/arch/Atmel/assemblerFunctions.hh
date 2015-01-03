/*
 * assemblerFunctions.hh
 *
 *  Created on: 30.12.2013
 *    Copyright & Author: dbaldin
 */

#ifndef ASSEMBLERFUNCTIONS_HH_
#define ASSEMBLERFUNCTIONS_HH_

#include "SCLConfig.hh"
#include <types.hh>
#include <process/Thread.hh>

extern unint4 sc_return_value;
extern unint4 sc_number;
extern unint4 sc_param1;
extern unint4 sc_param2;
extern unint4 sc_param3;
extern unint4 sc_param4;
extern unint4 sc_param5;

//! No operation command
#define NOP asm("nop")

#define BRANCHTO_WORKERTHREAD_WORK(objectptr, pid, stack_addr) \
     asm volatile( \
                    "" \
                    : \
                    : \
                    :  \
                )

#define GET_INTERRUPT_ENABLE_BIT(var) var = 0

//-----------------------------------------------------------------------------
// Enabling/Disabling Interrupts
//-----------------------------------------------------------------------------

// Enable interrupts (IRQ).
#define _enableInterrupts() asm volatile( \
                                "sei" \
                                : \
                                : \
                                :  \
                            )

// Disable interrupts (IRQ).
#define _disableInterrupts() asm volatile( \
                                "cli" \
                                : \
                                : \
                                :  \
                            )


#define GET_RETURN_CONTEXT(t, sp_int)  sp_int = 0

#define SET_RETURN_VALUE(sp_int, retval) sc_return_value = retval

#define GET_SYSCALL_NUM(sp_int, syscallnum) { syscallnum = (typeof(syscallnum)) sc_number; }

#define SYSCALLGETPARAMS1(int_sp, param1) param1 = (typeof(param1)) sc_param1

#define SYSCALLGETPARAMS2(int_sp, param1, param2) {param1 = (typeof(param1)) sc_param1; param2 = (typeof(param2)) sc_param2; }

#define SYSCALLGETPARAMS3(int_sp, param1, param2, param3)  {param1 = (typeof(param1)) sc_param1; param2 = (typeof(param2)) sc_param2;  param3 = (typeof(param3)) sc_param3; }

#define SYSCALLGETPARAMS4(int_sp, param1, param2, param3, param4) {param1 = (typeof(param1)) sc_param1; param2 = (typeof(param2)) sc_param2;  param3 = (typeof(param3)) sc_param3; param4 = (typeof(param4)) sc_param4; }

#define SYSCALLGETPARAMS5(int_sp, param1, param2, param3, param4, param5) {param1 = (typeof(param1)) sc_param1; param2 = (typeof(param2)) sc_param2;  param3 = (typeof(param3)) sc_param3; param4 = (typeof(param4)) sc_param4; param5 = (typeof(param5)) sc_param5; }

#define SYSCALLGETPARAM1(int_sp, param1) param1 = sc_param1

#define SYSCALLGETPARAM2(int_sp, param2) param2 = sc_param2

#define SYSCALLGETPARAM3(int_sp, param3) param3 = sc_param3

#define SYSCALLGETPARAM4(int_sp, param4) param4 = sc_param4

#define SYSCALLGETPARAM5(int_sp, param5) param5 = sc_param5

#define SETSTACKPTR(st)

#define SETPID(pid)

#define PROCESSOR_CONTEXT_SIZE 0

#define SAVE_CONTEXT_AT {while(1) {}}

// guaranetees that instructions will be executed without being interrupted without using a mutex
#define ATOMAR(statements) statements;


#if ENABLE_NESTED_INTERRUPTS

// Dsiables irqs and saves the irq enable bit to the variable
#define DISABLE_IRQS(irqstatus) \
    bool irqstatus; \
    GET_INTERRUPT_ENABLE_BIT(irqstatus); \
    _disableInterrupts();

#define RESTORE_IRQS(irqstatus) if (irqstatus) { _enableInterrupts(); }


#else

#define DISABLE_IRQS(irqstatus)
#define RESTORE_IRQS(irqstatus)
#endif


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
} // namespace assembler


#endif /* ASSEMBLERFUNCTIONS_HH_ */
