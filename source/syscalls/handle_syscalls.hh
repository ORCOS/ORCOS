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

#include "inc/defines.h"
#include "kernel/Kernel.hh"
#include "filesystem/Resource.hh"
#include "process/Task.hh"
#include "comm/Socket.hh"
#include "process/Module.hh"

// architecture dependent functions include
#include <assemblerFunctions.hh>
#include <process/TaskManager.hh>

extern Kernel* theOS;

// Global Variables to speed up access to these objects
extern Kernel_ThreadCfdCl* pCurrentRunningThread;
extern Task*        pCurrentRunningTask;
extern unint8       lastCycleStamp;

#define VALIDATE_SYSCALL_ADDRESS_RANGES 1

#define MAX_TASK_SIZE 0x100000 * 16  // 16 MB max

// Security MACRO which checks whether a address is inside the current running process
// Validation increases syscall overhead, however increases security as buffer overflow attacks
// are minimized
#if VALIDATE_SYSCALL_ADDRESS_RANGES
#define VALIDATE_IN_PROCESS( addr ) \
	if (((unint4) (addr) < LOG_TASK_SPACE_START) || ((unint4) (addr) > LOG_TASK_SPACE_START + MAX_TASK_SIZE)) { \
		LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"SYSCALL: Address Space Violation: %x",addr));\
		return (cError); \
	}
#else
#define VALIDATE_IN_PROCESS( addr )
#endif

/*!
 * \brief Manager class used to handle system calls.
 * \ingroup syscall
 *
 * This class can be configured using the \ref Configuration to contain the
 * implementation for different system calls. This makes it possible to configure
 * the SyscallManager in a way that it only contains the system call functionality that is needed
 * in order to reduce kernel size.  Also have a look at ORCOS.hh which implements the syscall API available for user taks programmer.
 */

int printToStdOut(int4 sp_int );

int ioctl(int4 sp_int);

int mallocp(int4 sp_int);

int getTime(int4 sp_int);

int mapMemory(int4 sp_int);

int runTask(int4 sp_int);

int task_killSyscall(int4 sp_int);

int shm_mapSyscall(int4 sp_int);

#ifdef HAS_SyscallManager_task_stopCfd
    int task_stopSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_task_resumeCfd
    int task_resumeSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_sleepCfd
    int sleepSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_thread_createCfd
    int thread_createSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_thread_runCfd
    int thread_run( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_thread_selfCfd
    int thread_self(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_thread_yieldCfd
    int thread_yield(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_signal_waitCfd
    int signal_wait( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_signal_signalCfd
    int signal_signal( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_fputcCfd
    int fputcSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_fgetcCfd
    int fgetcSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_fcreateCfd
    int fcreateSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_fopenCfd
    int fopenSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_fcloseCfd
    int fcloseSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_fwriteCfd
    int fwriteSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_freadCfd
    int freadSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_newCfd
    int newSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_deleteCfd
    int deleteSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_socketCfd
    int socketSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_connectCfd
    int connectSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_listenCfd
    int listenSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_bindCfd
    int bindSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_sendtoCfd
    int sendtoSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_recvCfd
    int recvSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_add_devaddrCfd
    int add_devaddrSyscall(int4 sp);
#endif

inline void handleSyscall(int4 sp_int) {
        int syscallnum;
        int retval = -1;

        // check sanity of sp_int
        if (((unint4) pCurrentRunningThread->threadStack.startAddr > (unint4) sp_int) || ( (unint4) pCurrentRunningThread->threadStack.endAddr < (unint4) sp_int)) {
        	LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"handleSyscall: sp_int corrupt: %x",sp_int));
            pCurrentRunningThread->terminate();
        }


        GET_SYSCALL_NUM(sp_int,(void*) syscallnum);

        // if all syscall numbers are together (eg. 0 - 20)
        // than gcc will convert this switch statements into a jump table!
        // this greatly speeds up the code!
        switch ( syscallnum ) {
    #ifdef HAS_SyscallManager_newCfd
            case cNewSysCallId:
                retval = newSyscall( sp_int );
                break;
    #endif

    #ifdef  HAS_SyscallManager_deleteCfd
                case cDeleteSysCallId:
                retval = deleteSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_thread_createCfd
            case cThread_CreateSysCallId:
                retval = thread_createSyscall( sp_int );
                break;
    #endif

            case cThread_ExitSysCallId:
                pCurrentRunningThread->terminate();
                break;

    #ifdef HAS_SyscallManager_thread_runCfd
            case cThread_RunSysCallId:
                retval = thread_run( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_thread_selfCfd
                case cThread_SelfSysCallId:
                retval = thread_self(sp_int);
                break;
    #endif

    #ifdef HAS_SyscallManager_thread_yieldCfd
                case cThread_YieldSysCallId:
                retval = thread_yield(sp_int);
                break;
    #endif

    #ifdef HAS_SyscallManager_signal_waitCfd
                case cSignal_WaitSyscallId:
                retval = signal_wait( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_signal_signalCfd
                case cSignal_SignalSyscallId:
                retval = signal_signal( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_task_stopCfd
                case cTask_StopSysCallId:
                retval = task_stopSyscall(sp_int);
                break;
    #endif

    #ifdef HAS_SyscallManager_task_resumeCfd
                case cTask_ResumeSysCallId:
                retval = task_resumeSyscall(sp_int);
                break;
    #endif

    #ifdef HAS_SyscallManager_fgetcCfd
            case cFGetcSysCallId:
                retval = fgetcSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_sleepCfd
            case cSleepSysCallId:
                retval = sleepSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_fputcCfd
            case cFPutcSysCallId:
                retval = fputcSyscall( sp_int );
                break;
    #endif
	#ifdef HAS_SyscallManager_fcreateCfd
            case cFCreateSysCallId:
            	retval = fcreateSyscall( sp_int );
            	break;
    #endif

    #ifdef HAS_SyscallManager_fopenCfd
            case cFOpenSysCallId:
                retval = fopenSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_fcloseCfd
            case cFCloseSysCallId:
                retval = fcloseSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_freadCfd
            case cFReadSysCallId:
                retval = freadSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_fwriteCfd
            case cFWriteSysCallId:
                retval = fwriteSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_socketCfd
                case cSocketSyscallId:
                retval = socketSyscall( sp_int );
                break;
    #endif

	#ifdef HAS_SyscallManager_connectCfd
               case cConnectSyscallId:
               retval = connectSyscall( sp_int );
               break;
    #endif

	#ifdef HAS_SyscallManager_listenCfd
           case cListenSyscallId:
        	   retval = listenSyscall( sp_int );
        	   break;
	#endif

    #ifdef HAS_SyscallManager_bindCfd
                case cBindSyscallId:
                retval = bindSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_sendtoCfd
                case cSendtoSyscallId:
                retval = sendtoSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_recvCfd
                case cRecvFromSyscallId:
                retval = recvSyscall( sp_int );
                break;
    #endif

                case cPrintToStdOut:
                retval = printToStdOut( sp_int );
                break;

                case cNewProtSysCallId:
                retval = mallocp(sp_int);
                break;

                case cGetTimeSyscallId:
                retval = getTime(sp_int);
                break;

                case cMapMemorySyscallId:
                retval = mapMemory(sp_int);
                break;

                case cModuleReturnId:
                ((Module*) pCurrentRunningTask)->moduleReturn();
                break;

                case cRunTaskId:
                retval = runTask(sp_int);
                break;

                case cTask_KillSysCallId:
                retval = task_killSyscall(sp_int);
                break;

                case cShmMapId:
                retval = shm_mapSyscall(sp_int);
                break;

                case cIOControl:
                retval = ioctl(sp_int);
                break;

            default:
                LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"invalid syscall %d",syscallnum));
                retval = -1;
                break;
        }

    #if ENABLE_NESTED_INTERRUPTS
        // first to do is disable interrupts now since we are going to restore the context now
        _disableInterrupts();

        pCurrentRunningThread->executinginthandler = false;
    #endif

        SET_RETURN_VALUE((void*)sp_int,(void*)retval);
        assembler::restoreContext( pCurrentRunningThread );
}




#endif /*SYSCALLMANAGER_HH_*/
