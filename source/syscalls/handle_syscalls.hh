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

#include "lib/defines.h"
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
extern ThreadCfdCl* pCurrentRunningThread;
extern Task*        pCurrentRunningTask;
extern unint8       lastCycleStamp;



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

#ifdef HAS_SyscallManager_task_stopSyscallCfd
    int task_stopSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_task_resumeSyscallCfd
    int task_resumeSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_sleepSyscallCfd
    int sleepSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_thread_createSyscallCfd
    int thread_createSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_thread_runSyscallCfd
    int thread_run( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_thread_selfSyscallCfd
    int thread_self(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_thread_yieldSyscallCfd
    int thread_yield(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_signal_waitSyscallCfd
    int signal_wait( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_signal_signalSyscallCfd
    int signal_signal( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_fputcSyscallCfd
    int fputcSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_fgetcSyscallCfd
    int fgetcSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_fcreateSyscallCfd
    int fcreateSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_fopenSyscallCfd
    int fopenSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_fcloseSyscallCfd
    int fcloseSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_fwriteSyscallCfd
    int fwriteSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_freadSyscallCfd
    int freadSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_newSyscallCfd
    int newSyscall( int4 int_sp );
#endif

#ifdef HAS_SyscallManager_deleteSyscallCfd
    int deleteSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_socketSyscallCfd
    int socketSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_connectSyscallCfd
    int connectSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_listenSyscallCfd
    int listenSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_bindSyscallCfd
    int bindSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_sendtoSyscallCfd
    int sendtoSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_recvSyscallCfd
    int recvSyscall(int4 int_sp);
#endif

#ifdef HAS_SyscallManager_add_devaddrSyscallCfd
    int add_devaddrSyscall(int4 sp);
#endif

inline void handleSyscall(int4 sp_int) {
        int syscallnum;
        int retval = -1;

        GET_SYSCALL_NUM(sp_int,(void*) syscallnum);

        // if all syscall numbers are together (eg. 0 - 20)
        // than gcc will convert this switch statements into a jump table!
        // this greatly speeds up the code!
        switch ( syscallnum ) {
    #ifdef HAS_SyscallManager_newSyscallCfd
            case cNewSysCallId:
                retval = newSyscall( sp_int );
                break;
    #endif

    #ifdef  HAS_SyscallManager_deleteSyscallCfd
                case cDeleteSysCallId:
                retval = deleteSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_thread_createSyscallCfd
            case cThread_CreateSysCallId:
                retval = thread_createSyscall( sp_int );
                break;
    #endif

            case cThread_ExitSysCallId:
                pCurrentRunningThread->terminate();
                break;

    #ifdef HAS_SyscallManager_thread_runSyscallCfd
            case cThread_RunSysCallId:
                retval = thread_run( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_thread_selfSyscallCfd
                case cThread_SelfSysCallId:
                retval = thread_self(sp_int);
                break;
    #endif

    #ifdef HAS_SyscallManager_thread_yieldSyscallCfd
                case cThread_YieldSysCallId:
                retval = thread_yield(sp_int);
                break;
    #endif

    #ifdef HAS_SyscallManager_signal_waitSyscallCfd
                case cSignal_WaitSyscallId:
                retval = signal_wait( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_signal_signalSyscallCfd
                case cSignal_SignalSyscallId:
                retval = signal_signal( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_task_stopSyscallCfd
                case cTask_StopSysCallId:
                retval = task_stopSyscall(sp_int);
                break;
    #endif

    #ifdef HAS_SyscallManager_task_resumeSyscallCfd
                case cTask_ResumeSysCallId:
                retval = task_resumeSyscall(sp_int);
                break;
    #endif

    #ifdef HAS_SyscallManager_fgetcSyscallCfd
            case cFGetcSysCallId:
                retval = fgetcSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_sleepSyscallCfd
            case cSleepSysCallId:
                retval = sleepSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_fputcSyscallCfd
            case cFPutcSysCallId:
                retval = fputcSyscall( sp_int );
                break;
    #endif
	#ifdef HAS_SyscallManager_fcreateSyscallCfd
            case cFCreateSysCallId:
            	retval = fcreateSyscall( sp_int );
            	break;
    #endif

    #ifdef HAS_SyscallManager_fopenSyscallCfd
            case cFOpenSysCallId:
                retval = fopenSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_fcloseSyscallCfd
            case cFCloseSysCallId:
                retval = fcloseSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_freadSyscallCfd
            case cFReadSysCallId:
                retval = freadSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_fwriteSyscallCfd
            case cFWriteSysCallId:
                retval = fwriteSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_socketSyscallCfd
                case cSocketSyscallId:
                retval = socketSyscall( sp_int );
                break;
    #endif

	#ifdef HAS_SyscallManager_connectSyscallCfd
               case cConnectSyscallId:
               retval = connectSyscall( sp_int );
               break;
    #endif

	#ifdef HAS_SyscallManager_listenSyscallCfd
           case cListenSyscallId:
        	   retval = listenSyscall( sp_int );
        	   break;
	#endif

    #ifdef HAS_SyscallManager_bindSyscallCfd
                case cBindSyscallId:
                retval = bindSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_sendtoSyscallCfd
                case cSendtoSyscallId:
                retval = sendtoSyscall( sp_int );
                break;
    #endif

    #ifdef HAS_SyscallManager_recvSyscallCfd
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
