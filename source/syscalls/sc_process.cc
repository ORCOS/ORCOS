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

#include "handle_syscalls.hh"
#include Kernel_Thread_hh
#include "assemblerFunctions.hh"
#include "inc/stringtools.hh"

/*******************************************************************
 *				RUNTASK Syscall
 *******************************************************************/
#ifdef HAS_SyscallManager_task_runCfd
int runTask(int4 sp_int) {

	int retval = 0;
	char* path;
	char* arguments;
	unint2 arg_length = 0;
	unint4 flags;

	SYSCALLGETPARAMS3(sp_int,path,arguments,flags);

	// TODO: safely validate path length to avoid running out of process bounds
	// TODO: safely validate arguments length to avoid running out of process bounds
	VALIDATE_IN_PROCESS(path);
	if (arguments != 0) {
		VALIDATE_IN_PROCESS(arguments);
		arg_length = (unint2) strlen(arguments);
	}

	LOG(SYSCALLS,DEBUG,"Syscall: runTask(%s)",path);

	if (path == 0) return (cError);

    Resource* res = theOS->getFileManager()->getResourceByNameandType( path, cFile );
    if (res == 0) {
		LOG(SYSCALLS,ERROR,"Syscall: runTask(%s) FAILED: invalid path.",path);
    	return (cInvalidResource);
    }

	// we got the resource .. load as task
	TaskIdT taskId;
	retval = theOS->getTaskManager()->loadTaskFromFile((File*)res,taskId,arguments,arg_length);
	/* on failure return the error number*/
	if (isError(retval))
	    return (retval);

	Task* t = theOS->getTaskManager()->getTask(taskId);
	t->setName(res->getName());

	if (flags & cWait) {
		/* lets wait for this task to finish */
		pCurrentRunningThread->sigwait(t);
	} else
	{
	    /* on success return task id*/
		if (isOk(retval)) retval = taskId;

		/* set return value now! as we must dispatch */
		void* sp_int;
		GET_RETURN_CONTEXT(pCurrentRunningThread,sp_int);
		SET_RETURN_VALUE(sp_int,retval);
	}

	/* new task may have higher priority so dispatch now! */
	theOS->getDispatcher()->dispatch();

	return (cError);
}
#endif

#ifdef HAS_SyscallManager_thread_waitCfd
int thread_wait(int4 sp_int) {

	int pid;
	SYSCALLGETPARAMS1(sp_int,pid);
	LOG(SYSCALLS,DEBUG,"Syscall: task_wait: pid %d",pid);

	if (pid == 0) {
		/* wait for any child to terminate on our current process
		 * this will block the current thread. we will not return here
		 * as the syscall context will be restored with return code
		 * upon signal reception. */
		unint4 signal = (pCurrentRunningTask->getId() << 16) | (SIG_CHILD_TERMINATED);
		pCurrentRunningThread->sigwait((void*) signal);
	} else {
		Task* t = theOS->getTaskManager()->getTask(pid);
		if (t == 0) return (cError);
		/* wait for task to finish
		 * this will block the current thread. we will not return here
         * as the syscall context will be restored with return code
         * upon signal reception. */
		pCurrentRunningThread->sigwait((void*) (t->getId() << 16));
	}

	return (cError);
}
#endif

/*******************************************************************
 *				TASK_KILL Syscall
 *******************************************************************/
#ifdef HAS_SyscallManager_task_killCfd
int task_killSyscall(int4 sp_int) {

	int	taskid;

	SYSCALLGETPARAMS1(sp_int,taskid);

	LOG(SYSCALLS,DEBUG,"Syscall: task_kill(%d)",taskid);

#if USE_WORKERTASK
	/* we must not kill the workertask! */
	if (taskid == 0) return (cInvalidArgument);
#endif

	Task* t = theOS->getTaskManager()->getTask(taskid);
	if (t == 0) return (cError);

	/* let the task manager cleanup everything. do not call t->terminate on your own
	 * if you do not clean up everything else */
	theOS->getTaskManager()->removeTask(t);

	return (cOk);
}
#endif

/*******************************************************************
 *				TASK_STOP Syscall
 *******************************************************************/


#ifdef HAS_SyscallManager_task_stopCfd
int task_stopSyscall(int4 int_sp)
{
    int taskid;

    SYSCALLGETPARAMS1(int_sp,taskid);

    LOG(SYSCALLS,DEBUG,"Syscall: task_stop(%d)",taskid);

    if (taskid <= 0) return (cError);

    Task* task = theOS->getTaskManager()->getTask(taskid);
    if (task != 0)
    {
        task->stop();
        return (cOk);
    }
    return (cError);

}
#endif



/*******************************************************************
 *				TASK_RESUME Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_task_resumeCfd
int task_resumeSyscall(int4 int_sp)
{
    int taskid;

    SYSCALLGETPARAMS1(int_sp,taskid);

    LOG(SYSCALLS,DEBUG,"Syscall: task_resume(%d)",taskid);

    Task* task = theOS->getTaskManager()->getTask(taskid);
    if (task != 0)
    {
        task->resume();

    #ifdef HAS_PRIORITY
        // we might have unblocked higher priority threads! so dispatch!
        DISABLE_IRQS(status);

        SET_RETURN_VALUE((void*)int_sp,(void*)cOk);
        theOS->getDispatcher()->dispatch();

    #endif

    }
    return (cError);
}
#endif


/*******************************************************************
 *				SLEEP Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_sleepCfd
int sleepSyscall( int4 int_sp ) {
    int t;
    SYSCALLGETPARAMS1(int_sp,t);

    LOG(SYSCALLS,DEBUG,"Syscall: sleep(%d)",t);

    TimeT timePoint = theOS->getClock()->getClockCycles();

    // sleep time is expected to be us
#if CLOCK_RATE >= (1 MHZ)
    pCurrentRunningThread->sleep( timePoint + (t * (CLOCK_RATE / 1000000)));
#else
    pCurrentRunningThread->sleep( timePoint + ((t * CLOCK_RATE) / 1000000));
#endif
    // return will not be called
    // if the system works correctly
    return (cOk);
}
#endif




/*******************************************************************
 *				THREAD_CREATE Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_thread_createCfd
int thread_createSyscall( int4 int_sp ) {
    int* threadid;
    thread_attr_t* attr;
    void* start_routine;
    void* arg;

    SYSCALLGETPARAMS4(int_sp,threadid,attr,start_routine,arg);

    VALIDATE_IN_PROCESS(attr);
    VALIDATE_IN_PROCESS(start_routine);
    if (threadid != 0)
    	VALIDATE_IN_PROCESS(threadid);


    LOG(SYSCALLS,DEBUG,"Syscall: thread_create(0x%x,0x%x,0x%x)",attr,start_routine,arg);
    LOG(SYSCALLS,DEBUG,"Syscall: rel_deadline %d",attr->deadline);
    // create a new thread belonging to the current task
    // create the thread CB. First get the tasktable to get the address of the threads exit routine
    register taskTable* tasktable = pCurrentRunningTask->getTaskTable();

    if (attr->stack_size <= 0)
    	attr->stack_size = DEFAULT_USER_STACK_SIZE;

    Thread* newthread;

#ifdef MEM_NO_FREE
    LinkedListDatabaseItem* litem = pCurrentRunningTask->getSuspendedThread(attr->stack_size);
    if (litem != 0)
    {

    	 newthread = (Thread*) litem->getData();
    	 newthread->status.clear();
    	 newthread->status.setBits( cNewFlag );
    	 newthread->sleepCycles = 0;
    	 newthread->signal = 0;
    	 newthread->threadStack.top = 0;
    	 newthread->startRoutinePointer = start_routine;

    	 LOG(SYSCALLS,DEBUG,"Syscall: thread_create() taking suspended thread %x",newthread);


		#ifdef REALTIME
			 RealTimeThread* rt = (RealTimeThread* ) newthread;
			 rt->absoluteDeadline = 0;
			 rt->instance = 1;

		#if CLOCK_RATE >= (1 MHZ)
			rt->relativeDeadline = ((TimeT) attr->deadline) * (CLOCK_RATE / 1000000);
			rt->executionTime = ((TimeT) attr->executionTime) * (CLOCK_RATE / 1000000);
			rt->period = ((TimeT) attr->period) * (CLOCK_RATE / 1000000);
		#else
			rt->relativeDeadline = (TimeT) (((float)attr->deadline) * ((float) CLOCK_RATE / 1000000.0f));
			rt->executionTime = (TimeT) (((float)attr->executionTime) * ((float) CLOCK_RATE / 1000000.0f));
			rt->period = (TimeT) (((float)attr->period) * ((float) CLOCK_RATE / 1000000.0f));
		#endif

		#endif

    	 litem->remove();
    	 pCurrentRunningTask->threadDb.addTail(litem);
    }
    else
    newthread =
            new Kernel_ThreadCfdCl( start_routine, (void*) tasktable->task_thread_exit_addr, pCurrentRunningTask, pCurrentRunningTask->getMemManager(), attr->stack_size, attr );

#else

    // create a new thread. It will automatically register itself at the currentRunningTask which will be the parent.
	 newthread = new Kernel_ThreadCfdCl( start_routine, (void*) tasktable->task_thread_exit_addr, pCurrentRunningTask, pCurrentRunningTask->getMemManager(), attr->stack_size, attr );
#endif

    // set the return value for threadid
	if (threadid != 0) {
		 *threadid = newthread->getId();
		 LOG(SYSCALLS,DEBUG,"Syscall: thread_created with id %d",*threadid);
	}

    LOG(SYSCALLS,DEBUG,"Syscall: thread_created stack: %x - %x, top:%d",newthread->threadStack.startAddr,newthread->threadStack.endAddr,newthread->threadStack.top);

    // set the arguments of the new thread
    newthread->arguments = arg;

    return (cOk);
}
#endif


/*******************************************************************
 *				THREAD_RUN Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_thread_runCfd
int thread_run( int4 int_sp ) {
    ThreadIdT threadid;

    SYSCALLGETPARAMS1(int_sp, threadid);

    LOG(SYSCALLS,TRACE,"Syscall: thread_run(%d)",threadid);

    if ( threadid >= cFirstThread ) {
        /* run the given thread */
        register Kernel_ThreadCfdCl* t = pCurrentRunningTask->getThreadbyId( threadid );

        if ( (t != 0) && (t->isNew()) && (!t->isReady()) ) {

#ifdef REALTIME
            theOS->getCPUScheduler()->computePriority(t);
#endif
            /* announce to scheduler! */
            t->run();
        } else return (cThreadNotFound);
    }
    else {
        /* run all new threads of this task!
           if we are realtime set the arrivaltime of all threads!
           the arrivaltime will be the same then, which is important
           if all threads shall start at the same time (no phase shifting) */
        LinkedList* threadDb  = pCurrentRunningTask->getThreadDB();
        LinkedListItem* litem = threadDb->getHead();

        while ( litem != 0 ) {
        	Kernel_ThreadCfdCl* thread = (Kernel_ThreadCfdCl*) litem->getData();

            if ( thread->isNew() && !thread->isReady() ) {
                // we got a newly created thread here that has never run before
#ifdef REALTIME
                theOS->getCPUScheduler()->computePriority(thread);
#endif

                // finally announce the thread to the scheduler
                thread->run();
            }

            litem = litem->getSucc();
        }

    }

#ifdef HAS_PRIORITY
    DISABLE_IRQS(status);
    SET_RETURN_VALUE((void*)int_sp,(void*)cOk);
    theOS->getDispatcher()->dispatch();
#endif

    return (cOk);
}
#endif




/*******************************************************************
 *				THREAD_SELF Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_thread_selfCfd
int thread_self(int4 int_sp)
{
	LOG(SYSCALLS,TRACE,"Syscall: threadSelf()");

    return (pCurrentRunningThread->getId());
}
#endif



/*******************************************************************
 *				THREAD_YIELD Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_thread_yieldCfd
int thread_yield(int4 int_sp)
{
    // dispatch directly
    theOS->getDispatcher()->dispatch();
    return (cOk);
}
#endif


/*******************************************************************
 *              GETPID Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_getpidCfd
int getpid(int4 int_sp)
{
    return (pCurrentRunningTask->getId());
}
#endif

/*******************************************************************
 *              taskioctl Syscall
 *******************************************************************/
int taskioctl(intptr_t int_sp) {
    int      cmd;
    int      taskId;
    char*    devpath;

    SYSCALLGETPARAMS3(int_sp,cmd,taskId,devpath);
    VALIDATE_IN_PROCESS(devpath);

    Task* task = theOS->getTaskManager()->getTask(taskId);
    if (task == 0)
        return (cInvalidArgument);

    CharacterDevice* res = (CharacterDevice*) theOS->getFileManager()->getResourceByNameandType(devpath,cStreamDevice);
    if (res == 0)
        return (cInvalidResourceType);

    if (cmd == TIOCTL_SET_STDOUT) {
        /* SET STDOUT of task*/
        task->setStdOut(res);
        return (cOk);
    } else {
        return (cInvalidArgument);
    }
}


