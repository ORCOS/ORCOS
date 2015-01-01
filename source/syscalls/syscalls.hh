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

#ifndef SYSCALLMANAGER_HH_
#define SYSCALLMANAGER_HH_

#include "inc/types.hh"
#include <SCLConfig.hh>

/* Syscall library defines include
 * containing the syscall ids */
#include "inc/defines.h"
/* The kernel */
#include "kernel/Kernel.hh"
/* architecture dependent functions include */
#include <assemblerFunctions.hh>

/* the kernel object */
extern Kernel* theOS;

/* Global Variables to speed up access to these objects  */
extern Kernel_ThreadCfdCl*  pCurrentRunningThread;
extern Task*                pCurrentRunningTask;
extern TimeT                lastCycleStamp;

/* TODO: get from task manager  */
#define MAX_TASK_SIZE 0x100000 * 16  // 16 MB max

/* Security feature to ensure passed addresses on syscalls only
 * point to task addresses to avoid kernel manipulation or
 * unallowed read access */
#define VALIDATE_SYSCALL_ADDRESS_RANGES 1


#define SYSCALL_ADDITIONAL_SANITY_CHECK 1

/* Security MACRO which checks whether a address is inside the current running process
 * Validation increases syscall overhead, however increases security as buffer overflow attacks
 * are minimized */
#if VALIDATE_SYSCALL_ADDRESS_RANGES
#define VALIDATE_IN_PROCESS( addr ) \
    if (((unint4) (addr) < LOG_TASK_SPACE_START) || ((unint4) (addr) > LOG_TASK_SPACE_START + MAX_TASK_SIZE)) { \
        LOG(SYSCALLS,WARN,"SYSCALL: Address Space Violation: %x, at %s, %d",addr,__FILE__,__LINE__);\
        return (cError); \
    }
#else
#define VALIDATE_IN_PROCESS( addr )
#endif

/* the syscall handler signature */
typedef  int (*p_syscall_handler_t)(intptr_t);

/* syscall handler table ( syscall #num -> handler) */
extern p_syscall_handler_t syscall_handler[];

/*******************************************************************
 *            MANDATORY SYSCALLS
 *******************************************************************/

/* the default system call handler for invalid or not implemented syscalls */
int sc_default_handler(intptr_t sp_int);

/* The thread exit syscall. not configurable and always used as it
 * is the exit point of every thread. */
int sc_thread_exit(intptr_t sp_int);

/* Standard out printing support */
int sc_printToStdOut(intptr_t sp_int );

/* Syscall handler to return the current processor cycles */
int sc_getCycles(intptr_t sp_int);

/* Syscall handler to return the current date time of the system*/
int sc_getDateTime(intptr_t sp_int);

/*******************************************************************
 *              Syscall Dispatching
 *******************************************************************/

extern "C" void handleSyscall(intptr_t sp_int);



#endif /*SYSCALLMANAGER_HH_*/
