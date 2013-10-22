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
#include "assembler.h"

/*******************************************************************
 *				RUNTASK Syscall
 *******************************************************************/

int runTask(int4 sp_int) {

	int retval = 0;
	char* path;
	char* arguments;
	unint2 arg_length = 0;

	SYSCALLGETPARAMS2(sp_int,path,arguments);

	// TODO: safely validate path length to avoid running out of process bounds
	// TODO: safely validate arguments length to avoid running out of process bounds
	VALIDATE_IN_PROCESS(path);
	if (arguments != 0) {
		VALIDATE_IN_PROCESS(arguments);
		arg_length = strlen(arguments);
	}

	LOG(SYSCALLS,INFO,(SYSCALLS,INFO,"Syscall: runTask(%s)",path));


	if (path == 0) return (cError);

    Resource* res = theOS->getFileManager()->getResourceByNameandType( path, cFile );
    if (res == 0) {
		LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: runTask(%s) FAILED: invalid path.",path));
    	return (cInvalidResource);
    }

	// we got the resource .. load as task
	TaskIdT taskId;
	retval = theOS->getTaskManager()->loadTaskFromFile((File*)res,taskId,arguments,arg_length);

	// on failure return the error number
	if (retval < 0) return (retval);

	// on success return the taskId
	return (taskId);

}

/*******************************************************************
 *				TASK_KILL Syscall
 *******************************************************************/

int task_killSyscall(int4 sp_int) {

	int	taskid;

	SYSCALLGETPARAMS1(sp_int,taskid);

	LOG(SYSCALLS,INFO,(SYSCALLS,INFO,"Syscall: task_kill(%d)",taskid));

#if USE_WORKERTASK
	// we must not kill the workertask!
	if (taskid == 0) return cInvalidArgument;
#endif

	Task* t = theOS->getTaskManager()->getTask(taskid);
	if (t == 0) return cError;

	// let the taskmanager cleanup everything. do not call t->terminate on your own
	// if you do not clean up everything else
	theOS->getTaskManager()->removeTask(t);

	return cOk;
}


/*******************************************************************
 *				TASK_STOP Syscall
 *******************************************************************/


#ifdef HAS_SyscallManager_task_stopCfd
int task_stopSyscall(int4 int_sp)
{
    int taskid;

    SYSCALLGETPARAMS1(int_sp,taskid);

    LOG(SYSCALLS,INFO,(SYSCALLS,INFO,"Syscall: task_stop(%d)",taskid));

    Task* task = theOS->getTaskManager()->getTask(taskid);
    if (task != 0)
    {
        task->stop();
        return cOk;
    }
    return cError;

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

    LOG(SYSCALLS,INFO,(SYSCALLS,INFO,"Syscall: task_resume(%d)",taskid));

    Task* task = theOS->getTaskManager()->getTask(taskid);
    if (task != 0)
    {
        task->resume();

    #ifdef HAS_PRIORITY
        // we might have unblocked higher priority threads! so dispatch!
        #if ENABLE_NESTED_INTERRUPTS
            // first to do is disable interrupts now since we are going to dispatch now
            _disableInterrupts();
            pCurrentRunningThread->executinginthandler = false;
        #endif
            SET_RETURN_VALUE((void*)int_sp,(void*)cOk);
            theOS->getCPUDispatcher()->dispatch(theOS->getClock()->getTimeSinceStartup() - lastCycleStamp);
    #endif

    }
    return cError;
}
#endif


/*******************************************************************
 *				SLEEP Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_sleepCfd
int sleepSyscall( int4 int_sp ) {
    int t;
    SYSCALLGETPARAMS1(int_sp,(void*)t);

    LOG(SYSCALLS,DEBUG,(SYSCALLS,DEBUG,"Syscall: sleep(%d)",t));

    // sleep time is expected to be us
    pCurrentRunningThread->sleep( (t * (CLOCK_RATE / 1000000)));

    // return will not be called
    // if the system works correctly
    return cOk;
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
    VALIDATE_IN_PROCESS(threadid);
    VALIDATE_IN_PROCESS(arg);

    LOG(SYSCALLS,INFO,(SYSCALLS,INFO,"Syscall: thread_create(0x%x,0x%x,0x%x)",attr,start_routine,arg));
    LOG(SYSCALLS,INFO,(SYSCALLS,INFO,"Syscall: rel_deadline %d",attr->deadline));
    // create a new thread belonging to the current task
    // create the thread CB. First get the tasktable to get the address of the threads exit routine
    register taskTable* tasktable = pCurrentRunningTask->getTaskTable();

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

    	 LOG(SYSCALLS,INFO,(SYSCALLS,INFO,"Syscall: thread_create() taking suspended thread %x",newthread));


		#ifdef REALTIME
			 RealTimeThread* rt = (RealTimeThread* ) newthread;
			 rt->absoluteDeadline = 0;
			 rt->instance = 1;

		#if CLOCK_RATE >= (1 MHZ)
			rt->relativeDeadline = ((unint8) attr->deadline) * (CLOCK_RATE / 1000000);
			rt->executionTime = ((unint8) attr->executionTime) * (CLOCK_RATE / 1000000);
			rt->period = ((unint8) attr->period) * (CLOCK_RATE / 1000000);
		#else
			rt->relativeDeadline = (unint8) (((float)attr->deadline) * ((float) CLOCK_RATE / 1000000.0f));
			rt->executionTime = (unint8) (((float)attr->executionTime) * ((float) CLOCK_RATE / 1000000.0f));
			rt->period = (unint8) (((float)attr->period) * ((float) CLOCK_RATE / 1000000.0f));
		#endif

		#endif

    	 litem->remove();
    	 pCurrentRunningTask->threadDb.addTail(litem);
    }
    else
    newthread =
            new ThreadCfdCl( start_routine, (void*) tasktable->task_thread_exit_addr, pCurrentRunningTask, pCurrentRunningTask->getMemManager(), attr->stack_size, attr );

#else

    // create a new thread. It will automatically register itself at the currentRunningTask which will be the parent.
	 newthread =
			new Kernel_ThreadCfdCl( start_routine, (void*) tasktable->task_thread_exit_addr, pCurrentRunningTask, pCurrentRunningTask->getMemManager(), attr->stack_size, attr );
#endif

    // set the return value for threadid
    *threadid = newthread->getId();

    LOG(SYSCALLS,INFO,(SYSCALLS,INFO,"Syscall: thread_created with id %d",*threadid));
    LOG(SYSCALLS,INFO,(SYSCALLS,INFO,"Syscall: thread_created stack: %x - %x, top:%d",newthread->threadStack.startAddr,newthread->threadStack.endAddr,newthread->threadStack.top));

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
    int threadid;

    SYSCALLGETPARAMS1(int_sp,(void*) threadid);

    LOG(SYSCALLS,INFO,(SYSCALLS,INFO,"Syscall: thread_run(%d)",threadid));


#ifdef REALTIME
    unint8 currentTime = theOS->getClock()->getTimeSinceStartup();
#endif

    if ( threadid >= cFirstThread ) {
        // run the given thread
        register Kernel_ThreadCfdCl* t = pCurrentRunningTask->getThreadbyId( threadid );

        if ( t != 0 && t->isNew() && !t->isReady() ) {

            LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: Thread run valid"));


#ifdef REALTIME
            // we are realtime so set the arrival time
            t->arrivalTime = currentTime + t->phase;
            theOS->getCPUScheduler()->computePriority(t);
#endif
            // announce to scheduler!
            t->run();
        } else return (cThreadNotFound);
    }
    else {
        // run all new threads of this task!
        // if we are realtime set the arrivaltime of all threads!
        // the arrivaltime will be the same then, which is important
        // if all threads shall start at the same time (no phase shifting)
        LinkedListDatabase* threadDb = pCurrentRunningTask->getThreadDB();
        LinkedListDatabaseItem* litem = threadDb->getHead();

        while ( litem != 0 ) {
        	Kernel_ThreadCfdCl* thread = (Kernel_ThreadCfdCl*) litem->getData();

            if ( thread->isNew() && !thread->isReady() ) {
                // we got a newly created thread here that has never run before

#ifdef REALTIME
                // we are realtime so set the arrival time
                thread->arrivalTime = currentTime + thread->phase;
                theOS->getCPUScheduler()->computePriority(thread);
                LOG(SYSCALLS,INFO,(SYSCALLS,INFO,"Syscall: Thread Priority=%d",*((unint4*)&thread->effectivePriority)));
#endif

                // finally announce the thread to the scheduler
                thread->run();
            }

            litem = litem->getSucc();
        }

    }

#ifdef HAS_PRIORITY
    #if ENABLE_NESTED_INTERRUPTS
        // first to do is disable interrupts now since we are going to dispatch now
        _disableInterrupts();
        pCurrentRunningThread->executinginthandler = false;
    #endif
        SET_RETURN_VALUE((void*)int_sp,(void*)cOk);
        theOS->getCPUDispatcher()->dispatch(theOS->getClock()->getTimeSinceStartup() - lastCycleStamp);
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
	LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: threadSelf()"));

    return pCurrentRunningThread->getId();
}
#endif



/*******************************************************************
 *				THREAD_YIELD Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_thread_yieldCfd
int thread_yield(int4 int_sp)
{
    // dispatch directly
    theOS->getCPUDispatcher()->dispatch(theOS->getClock()->getTimeSinceStartup() - lastCycleStamp);
    return cOk;
}
#endif


