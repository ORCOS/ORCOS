/*
 * syscall.cc
 *
 *  Created on: 20.07.2009
 *      Author: dbaldin
 */


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
/*!
 * All parameters of any function call are stored on the stack,
 * so the procedure for doing a system call uses this fact.
 *
 * By doing the hardware system call, which is an unconditional jump, the ISR gives
 * control to the SyscallDispatcher. Up to now no context switch has been made,
 * (possibly supported by disabling interrupts) and all information of the former context
 * could be read. By reading the stackpointer all parameters of the system call and its
 * number could be computed and used by the dispatcher.
 *
 *
 * parameters passed to this method start at: (at time of executing the command "sc")
 *  1. param = r4 at 28(sp)
 *  2. param = r5 at 32(sp)
 *
 * up to 7 parameters are stored that way
 * if more parameters are needed the parameters need to be fetched from the stack of the calling method
 * to be explained later :)
 */
extern "C" int syscall (int syscallnumber, ...) {
    volatile int ret;
    SYSCALL(ret, syscallnumber);
    // return the result
    return ret;
}


