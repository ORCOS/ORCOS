/*
 * sc_table.cc
 *
 *  Created on: 24.10.2014
 *     Author: Daniel
 *     Copyright Daniel Baldin
 */

#include "syscalls.hh"
#include "SCLConfig.hh"

#define MAX_SYSCALL_NUM 46
/*****************************************************************************
 * Method: sc_default_handler(intptr_t sp_int)
 *
 * @description
 *  The default system call handler for unimplemented system calls
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int:   Error Code
 *---------------------------------------------------------------------------*/
int sc_default_handler(intptr_t sp_int) {
    return (cNotImplemented);
}

p_syscall_handler_t syscall_handler[MAX_SYSCALL_NUM+1] = {
        sc_fopen,           /* cFOpenSysCallId          = 0 */
        sc_fclose,          /* cFCloseSysCallId         = 1 */
        sc_fread,           /* cFReadSysCallId          = 2 */
        sc_fwrite,          /* cFWriteSysCallId         = 3 */
        sc_fputc,           /* cFPutcSysCallId          = 4 */
        sc_fgetc,           /* cFGetcSysCallId          = 5 */
        sc_fcreate,         /* cFCreateSysCallId        = 6 */
        sc_new,             /* cNewSysCallId            = 7 */
        sc_delete,          /* cDeleteSysCallId         = 8 */
        sc_task_stop,       /* cTask_StopSysCallId      = 9 */
        sc_task_resume,     /* cTask_ResumeSysCallId    = 10 */
        sc_sleep,           /* cSleepSysCallId          = 11 */
        sc_thread_create,   /* cThread_CreateSysCallId  = 12 */
        sc_thread_run,      /* cThread_RunSysCallId     = 13 */
        sc_thread_self,     /* cThread_SelfSysCallId    = 14 */
        sc_thread_yield,    /* cThread_YieldSysCallId   = 15 */
        sc_thread_exit,     /* cThread_ExitSysCallId    = 16 */
        sc_signal_wait,     /* cSignal_WaitSyscallId    = 17 */
        sc_signal_signal,   /* cSignal_SignalSyscallId  = 18 */
        sc_socket,          /* cSocketSyscallId         = 19 */
        sc_connect,         /* cConnectSyscallId        = 20 */
        sc_listen,          /* cListenSyscallId         = 21 */
        sc_bind,            /* cBindSyscallId           = 22 */
        sc_sendto,          /* cSendtoSyscallId         = 23 */
        sc_recv,            /* cRecvFromSyscallId       = 24 */
        sc_ioctl,           /* cIOControl               = 25 */
        sc_printToStdOut,   /* cPrintToStdOut           = 26 */
        0,                  /* cNewProtSysCallId        = 27 */
        sc_getCycles,       /* cGetTimeSyscallId        = 28 */
        sc_mapMemory,       /* cMapMemorySyscallId      = 29 */
        0,                  /* cModuleReturnId          = 30 */
        sc_task_run,        /* cRunTaskId               = 31 */
        sc_task_kill,       /* cTask_KillSysCallId      = 32 */
        sc_shm_map,         /* cShmMapId                = 33 */
        sc_thread_wait,     /* cThread_WaitPID          = 34 */
        sc_fstat,           /* cFStatId                 = 35 */
        sc_fremove,         /* cFRemoveID               = 36 */
        sc_getpid,          /* cGetPID                  = 37 */
        0,                  /* cShmUnmapId OSBOLETE     = 38 */
        sc_getDateTime,     /* cGetDateTimeSyscallId    = 39 */
        sc_mkdev,           /* cMkDevSyscallId          = 40 */
        sc_task_ioctl,      /* cTaskioctlscallId        = 41 */
        sc_fseek,           /* cFSeekSyscallId          = 42 */
        sc_thread_terminate,/* cThreadTerminateSyscallId= 43 */
        sc_thread_name,     /* cThreadNameSyscallId     = 44 */
        sc_mount,           /* cMountSyscallId          = 45 */
        sc_waitirq          /* cThreadWaitIRQSyscallId  = 46 */
};


extern "C" void handleSyscall(intptr_t sp_int) {
    int syscallnum;
    int retval = cError;

    // check sanity of sp_int
#if SYSCALL_ADDITIONAL_SANITY_CHECK
    if (((unint4) pCurrentRunningThread->threadStack.startAddr > (unint4) sp_int) || ( (unint4) pCurrentRunningThread->threadStack.endAddr < (unint4) sp_int)) {
        LOG(SYSCALLS, ERROR, "handleSyscall: sp_int corrupt: %x", sp_int);
        theOS->getTaskManager()->terminateThread(pCurrentRunningThread);
    }
#endif

    /* get passed syscall id */
    GET_SYSCALL_NUM(sp_int, syscallnum);
    syscallnum &= 0xff;

    /* check for valid range*/
    if (syscallnum > MAX_SYSCALL_NUM) {
        retval = cError;
    } else {
        /* execute and get result from handler */
        p_syscall_handler_t handler = syscall_handler[syscallnum];
        if (handler != 0) {
#if ENABLE_NESTED_INTERRUPTS
            _enableInterrupts();
#endif
            retval = handler(sp_int);
        }
    }

    /* return from syscall */
    _disableInterrupts();
    SET_RETURN_VALUE(sp_int, retval);
    assembler::restoreContext(pCurrentRunningThread);
    __builtin_unreachable();
}



