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

 Author: dbaldin
 */

#include "syscalls.hh"
#include Kernel_Thread_hh
#include "assemblerFunctions.hh"
#include "inc/stringtools.hh"
#include "inc/signals.hh"

/*******************************************************************
 *                RUNTASK Syscall
 *******************************************************************/
#ifdef HAS_SyscallManager_task_runCfd
/*****************************************************************************
 * Method: sc_task_run(intptr_t sp_int)
 *
 * @description
 *  task_run system call handler
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_task_run(intptr_t sp_int) {
    int    retval = 0;
    char*  path;
    char*  arguments;
    char*  stdoutdev;
    char*  workingDir;
    unint2 arg_length = 0;

    SYSCALLGETPARAMS4(sp_int, path, arguments, stdoutdev, workingDir);

    VALIDATE_IN_PROCESS(path);
    if (arguments != 0) {
        VALIDATE_IN_PROCESS(arguments);
        arg_length = (unint2) strlen(arguments);
    }
    if (stdoutdev != 0) {
        VALIDATE_IN_PROCESS(stdoutdev);
    }
    if (workingDir != 0) {
       VALIDATE_IN_PROCESS(workingDir);
    }

    LOG(SYSCALLS, DEBUG, "Syscall: runTask(%s)", path);

    Resource* res = theOS->getFileManager()->getResourceByNameandType(path, cFile);
    if (res == 0) {
        LOG(SYSCALLS, ERROR, "Syscall: runTask(%s) FAILED: invalid path.", path);
        return (cInvalidResource);
    }

    /* we got the resource .. load as task */
    TaskIdT taskId;
    retval = theOS->getTaskManager()->loadTaskFromFile(static_cast<File*>(res), taskId, arguments, arg_length);
    /* on failure return the error number*/
    if (isError(retval)) {
        return (retval);
    }

    Task* t = theOS->getTaskManager()->getTask(taskId);
    t->setName(res->getName());
    if (workingDir == 0 || isError(t->setWorkingDirectory(workingDir))) {
        if (path[0] == '/') {
            char* filename  = basename(path);
            char* lpos;
            if (filename != path) {
                 lpos    = filename -1;
                 lpos[0] = 0;
            }
            t->setWorkingDirectory(path);
            if (filename != path) {
                /* restore original string */
                lpos[0] = '/';
            }
        } else {
            // relative path
            char tmppath[256];
            sprintf(tmppath, "%s/%s", pCurrentRunningTask->getWorkingDirectoryPath(), path);
            if (isError(t->setWorkingDirectory(tmppath))) {
                t->setWorkingDirectory("/");
            }
        }
    }

    /* set specified stdout */
    if (stdoutdev != 0) {
        CharacterDevice* stdoutres = static_cast<CharacterDevice*>(theOS->getFileManager()->getResourceByNameandType(stdoutdev, (ResourceType) (cStreamDevice | cFile)));
        if (stdoutres != 0) {
            /* SET STDOUT of task*/
            if (stdoutres->getType() & cFile) {
                File* file = static_cast<File*>(stdoutres);
                file->resetPosition();
                file->seek(file->getFileSize());
            }

            t->setStdOut(stdoutres);
        }
    }

    /* on success return task id*/
    if (isOk(retval)) {
        retval = taskId;
    }

    SET_RETURN_VALUE(sp_int, retval);

    /* new task may have higher priority so dispatch now! */
    theOS->getDispatcher()->dispatch();
    __builtin_unreachable();
    return (cError);
}
#endif

#ifdef HAS_SyscallManager_thread_waitCfd
/*****************************************************************************
 * Method: sc_thread_wait(intptr_t sp_int)
 *
 * @description
 *  TODO
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_thread_wait(intptr_t sp_int) {
    int       pid;
    int       threadid;
    SYSCALLGETPARAMS2(sp_int, pid, threadid);
    LOG(SYSCALLS, DEBUG, "Syscall: thread_wait: pid %d", pid);

#ifdef ORCOS_SUPPORT_SIGNALS
    if (pid == 0) {
        /* wait for any child to terminate on our current process
         * this will block the current thread. we will not return here
         * as the syscall context will be restored with return code
         * upon signal reception. */
        int signal;
        if (threadid == 0) {
            /* just wait for any thread to finish */
            signal = SIGNAL_SPACE_TASK(pCurrentRunningTask->getId()) | SIG_CHILD_TERMINATED;
        } else {
            /* wait for this specific thread to finish */
            signal = SIGNAL_SPACE_TASK(pCurrentRunningTask->getId()) | SIG_CHILD_TERMINATED | (threadid << 8);
        }
        pCurrentRunningThread->sigwait(SIGNAL_GENERIC, reinterpret_cast<void*>(signal));
        __builtin_unreachable();
    } else {
        /* if we are waiting on another task, we may only wait for the task
         * not for a thread of that specific task! */
        Task* t = theOS->getTaskManager()->getTask(pid);
        if (t == 0) {
            return (cInvalidArgument);
        }
        /* wait for task to finish
         * this will block the current thread. we will not return here
         * as the syscall context will be restored with return code
         * upon signal reception. */
        int signal = SIGNAL_SPACE_TASK(t->getId()) | SIG_TASK_TERMINATED;
        pCurrentRunningThread->sigwait(SIGNAL_GENERIC, reinterpret_cast<void*>(signal));
        __builtin_unreachable();
    }
#endif

    return (cError);
}
#endif

#ifdef HAS_SyscallManager_waitirqCfd
/*****************************************************************************
 * Method: sc_waitirq(intptr_t sp_int)
 *
 * @description
 *  Waits for a given IRQ number
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_waitirq(intptr_t sp_int) {
    int irq;
    SYSCALLGETPARAMS1(sp_int, irq);
    LOG(SYSCALLS, DEBUG, "Syscall: sc_waitirq: irq %d", irq);

    if (irq > 256) {
        return (cInvalidArgument);
    }

    theOS->getInterruptManager()->waitIRQ(irq, pCurrentRunningThread);
    pCurrentRunningThread->block();
    __builtin_unreachable();
}


#endif

/*******************************************************************
 *                TASK_KILL Syscall
 *******************************************************************/
/*****************************************************************************
 * Method: sc_task_kill(intptr_t sp_int)
 *
 * @description
 *  TODO
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
#ifdef HAS_SyscallManager_task_killCfd
int sc_task_kill(intptr_t sp_int) {
    int taskid;
    SYSCALLGETPARAMS1(sp_int, taskid);
    LOG(SYSCALLS, DEBUG, "Syscall: task_kill(%d)", taskid);

#if USE_WORKERTASK
    /* we must not kill the workertask! */
    if (taskid == 0) {
        return (cInvalidArgument);
    }
#endif

    Task* t = theOS->getTaskManager()->getTask(taskid);
    if (t == 0) {
        return (cError);
    }

    /* let the task manager cleanup everything. do not call t->terminate on your own
     * if you do not clean up everything else */
    return (theOS->getTaskManager()->removeTask(t));
}
#endif

/*******************************************************************
 *                TASK_STOP Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_task_stopCfd
/*****************************************************************************
 * Method: sc_task_stop(intptr_t sp_int)
 *
 * @description
 *  TODO
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_task_stop(intptr_t int_sp) {
    int taskid;

    SYSCALLGETPARAMS1(int_sp, taskid);

    LOG(SYSCALLS, DEBUG, "Syscall: task_stop(%d)", taskid);

#if USE_WORKERTASK
    if (taskid == 0) {
        return (cError);
    }
#endif

    Task* task = theOS->getTaskManager()->getTask(taskid);
    if (task != 0) {
        task->stop();
        return (cOk);
    }

    return (cError);
}
#endif

/*******************************************************************
 *                TASK_RESUME Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_task_resumeCfd
/*****************************************************************************
 * Method: sc_task_resume(intptr_t sp_int)
 *
 * @description
 *  TODO
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_task_resume(intptr_t int_sp) {
    int taskid;

    SYSCALLGETPARAMS1(int_sp, taskid);

    LOG(SYSCALLS, DEBUG, "Syscall: task_resume(%d)", taskid);

    Task* task = theOS->getTaskManager()->getTask(taskid);
    if (task != 0) {
        task->resume();

#ifdef HAS_PRIORITY
        /* we might have unblocked higher priority threads! so dispatch! */
        _disableInterrupts();

        SET_RETURN_VALUE((void*)int_sp, (void*)cOk);
        theOS->getDispatcher()->dispatch();
        __builtin_unreachable();
#endif
        return (cOk);
    }
    return (cError);
}
#endif

/*******************************************************************
 *                SLEEP Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_sleepCfd
/*****************************************************************************
 * Method: sc_sleep(intptr_t sp_int)
 *
 * @description
 *  TODO
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_sleep(intptr_t int_sp) {
    unsigned int t;
    SYSCALLGETPARAMS1(int_sp, t);
    LOG(SYSCALLS, DEBUG, "Syscall: sleep(%u)", t);

    TimeT timePoint = theOS->getClock()->getClockCycles();

    if (t == 0) {
        return (cOk);
    }

    SET_RETURN_VALUE((void*)int_sp, (void*)cOk);
    /* sleep time is expected to be us */
#if CLOCK_RATE >= (1 MHZ)
    pCurrentRunningThread->sleep(timePoint + (t * (CLOCK_RATE / 1000000)));
#else
    pCurrentRunningThread->sleep(timePoint + ((t * CLOCK_RATE) / 1000000));
#endif
    __builtin_unreachable();
    /* return will not be called
     * if the system works correctly */
    return (cOk);
}
#endif

/*******************************************************************
 *                THREAD_CREATE Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_thread_createCfd
/*****************************************************************************
 * Method: sc_thread_create(intptr_t sp_int)
 *
 * @description
 *  TODO
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_thread_create(intptr_t int_sp) {
    ThreadIdT*     threadid;
    thread_attr_t* attr;
    void*          start_routine;
    void*          arg;

    SYSCALLGETPARAMS4(int_sp, threadid, attr, start_routine, arg);

    VALIDATE_IN_PROCESS(attr);
    VALIDATE_IN_PROCESS(start_routine);
    if (threadid != 0) {
        VALIDATE_IN_PROCESS(threadid);
    }

    LOG(SYSCALLS, DEBUG, "Syscall: thread_create(0x%x,0x%x,0x%x)", attr, start_routine, arg);
    LOG(SYSCALLS, DEBUG, "Syscall: rel_deadline %d", attr->deadline);
    // create a new thread belonging to the current task
    // create the thread CB. First get the tasktable to get the address of the threads exit routine
    register taskTable* tasktable = pCurrentRunningTask->getTaskTable();

    if (attr->stack_size <= 0) {
        attr->stack_size = DEFAULT_USER_STACK_SIZE;
    }
    if (attr->stack_size > 0x10000) {
        attr->stack_size = 0x10000;
    }

    /* create a new thread. It will automatically register itself at the currentRunningTask which will be the parent. */
    Thread* newthread = new Kernel_ThreadCfdCl(
                                       start_routine,
                                       reinterpret_cast<void*>(tasktable->task_thread_exit_addr),
                                       pCurrentRunningTask,
                                       attr->stack_size,
                                       attr);

    /* set the return value for threadid */
    if (threadid != 0) {
        *threadid = newthread->getId();
        LOG(SYSCALLS, DEBUG, "Syscall: thread_created with id %d", *threadid);
    }

    LOG(SYSCALLS, DEBUG, "Syscall: thread_created stack: %x - %x, top:%d", newthread->threadStack.startAddr, newthread->threadStack.endAddr, newthread->threadStack.top);

    /* set the arguments of the new thread */
    newthread->arguments = arg;

    return (cOk );
}
#endif

/*******************************************************************
 *                THREAD_RUN Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_thread_runCfd
/*****************************************************************************
 * Method: sc_thread_run(intptr_t sp_int)
 *
 * @description
 *  TODO
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_thread_run(intptr_t int_sp) {
    ThreadIdT threadid;
    SYSCALLGETPARAMS1(int_sp, threadid);
    LOG(SYSCALLS, TRACE, "Syscall: thread_run(%u)", threadid);

    if (threadid != 0) {
        /* run the given thread */
        register Kernel_ThreadCfdCl* t = pCurrentRunningTask->getThreadbyId(threadid);

        if ((t != 0) && (t->isNew()) && (!t->isReady())) {
#ifdef REALTIME
            theOS->getCPUScheduler()->computePriority(t);
#endif
            /* announce to scheduler! */
            t->run();
        } else {
            return (cThreadNotFound);
        }
    } else {
        /* run all new threads of this task! */
        LinkedList* threadDb  = pCurrentRunningTask->getThreadDB();
        LinkedListItem* litem = threadDb->getHead();

        while (litem != 0) {
            Kernel_ThreadCfdCl* thread = static_cast<Kernel_ThreadCfdCl*>(litem->getData());

            if (thread->isNew() && !thread->isReady()) {
                /* we got a newly created thread here that has never run before */
#ifdef REALTIME
                theOS->getCPUScheduler()->computePriority(thread);
#endif

                /* finally announce the thread to the scheduler.
                 * If an arrivaltime has been specified the dispatcher
                 * will put the thread to sleep until that timepoint */
                thread->run();
            }

            litem = litem->getSucc();
        }
    }

#ifdef HAS_PRIORITY
    _disableInterrupts();
    SET_RETURN_VALUE((void*)int_sp, (void*)cOk);
    theOS->getDispatcher()->dispatch();
    __builtin_unreachable();
#endif

    return (cOk);
}
#endif

/*******************************************************************
 *                THREAD_SELF Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_thread_selfCfd
/*****************************************************************************
 * Method: sc_thread_self(intptr_t sp_int)
 *
 * @description
 *  TODO
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_thread_self(intptr_t int_sp) {
    LOG(SYSCALLS, TRACE, "Syscall: threadSelf()");
    return (pCurrentRunningThread->getId());
}
#endif

/*******************************************************************
 *                THREAD_YIELD Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_thread_yieldCfd
/*****************************************************************************
 * Method: sc_thread_yield(intptr_t sp_int)
 *
 * @description
 *  TODO
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_thread_yield(intptr_t int_sp) {
    /* dispatch directly */
    theOS->getDispatcher()->dispatch();
    __builtin_unreachable();
    return (cOk );
}
#endif

/*******************************************************************
 *              GETPID Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_getpidCfd
/*****************************************************************************
 * Method: sc_getpid(intptr_t sp_int)
 *
 * @description
 *  TODO
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_getpid(intptr_t int_sp) {
    return (pCurrentRunningTask->getId());
}
#endif

#ifdef HAS_SyscallManager_thread_nameCfd
/*****************************************************************************
 * Method: sc_thread_name(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_thread_name(intptr_t int_sp) {
    char*     name;
    ThreadIdT tid;

    SYSCALLGETPARAMS2(int_sp, tid, name);
    VALIDATE_IN_PROCESS(name);

    Thread* p_thread = pCurrentRunningThread;
    if (tid != 0) {
        p_thread = pCurrentRunningTask->getThreadbyId(tid);
        if (!p_thread) {
            return (cInvalidArgument);
        }
    }

    p_thread->setName(name);
    return (cOk);
}
#endif

/*******************************************************************
 *              task_ioctl Syscall
 *******************************************************************/
#ifdef HAS_SyscallManager_task_ioctlCfd
/*****************************************************************************
 * Method: sc_task_ioctl(intptr_t sp_int)
 *
 * @description
 *  I/O Control on tasks. Allows the modification of task parameters.
 *  Currently only modification of stdout is possible.
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
int sc_task_ioctl(intptr_t int_sp) {
    int   cmd;
    int   taskId;
    char* devpath;

    SYSCALLGETPARAMS3(int_sp, cmd, taskId, devpath);
    VALIDATE_IN_PROCESS(devpath);

    Task* task = theOS->getTaskManager()->getTask(taskId);
    if (task == 0)
        return (cInvalidArgument);

    CharacterDevice* res = static_cast<CharacterDevice*>(theOS->getFileManager()->getResourceByNameandType(devpath, cStreamDevice));
    if (res == 0) {
        return (cInvalidResourceType);
    }

    if (cmd == TIOCTL_SET_STDOUT) {
        /* SET STDOUT of task*/
        task->setStdOut(res);
        return (cOk);
    } else {
        return (cInvalidArgument);
    }
}
#endif

