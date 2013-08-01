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

#include "handle_syscalls.hh"
#include ThreadCfd_hh
#include "assembler.h"

int mapMemory(int4 sp_int)
{
	 const char* log_start;
	 const char* phy_start;
	 size_t size;
	 int protection;

	 SYSCALLGETPARAMS4(sp_int,log_start,phy_start,size,protection);

#ifdef HAS_MemoryManager_HatLayerCfd
     // create the vm map for the task! protection = 7 = RWX, ZoneSelect = 3
     theOS->getHatLayer()->map((void*) log_start,(void*) phy_start, size ,protection,3,pCurrentRunningTask->getId(), !ICACHE_ENABLE);
     return cOk;
#endif

     return cError;
}

int shm_mapSyscall(int4 sp_int) {
	const char* file;
	unint4* mapped_address;
	unint4* mapped_size;

	SYSCALLGETPARAMS3(sp_int,file,mapped_address,mapped_size);

	// try to find the specified resource
	Resource* res = theOS->getFileManager()->getResourceByNameandType(file,cSharedMem);
	if (res == 0) return cInvalidArgument;

	// must be a shared mem resource
	SharedMemResource* shm_res = (SharedMemResource*) res;
	unint4 virtual_address;

	// is this shared mem resource valid?
	if (shm_res->getPhysicalStartAddress() == 0) return cInvalidResource;

	// mpa it into the address space of the calling task
	int retval = shm_res->mapIntoTask(pCurrentRunningTask,virtual_address);

	// set return addresses
	*mapped_address = virtual_address;
	*mapped_size = shm_res->getSize();

	return retval;

}



int runTask(int4 sp_int) {

	int retval = 0;
	char* path;

	SYSCALLGETPARAMS1(sp_int,path);

	LOG(SYSCALLS,INFO,(SYSCALLS,INFO,"Syscall: runTask(%s)",path));

	if (path == 0) return cError;

    Resource* res = theOS->getFileManager()->getResourceByNameandType( path, cFile );
	/*if ( res != 0 ) {
		retval = pCurrentRunningTask->aquireResource( res, pCurrentRunningThread, false );
	}
	else {
		retval = cInvalidResource;
		LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: runTask(%s) FAILED: cannot aquire resource.",path));
	}

	if (retval < 0) return cCanNotAquireResource;*/
    if (res == 0) return cInvalidResource;

	// we got the resource .. load as task

	TaskIdT taskId;
	retval = theOS->getTaskManager()->loadTaskFromFile((File*)res,taskId);
	if (retval < 0) return retval;



	return taskId;

}


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

	theOS->getTaskManager()->removeTask(t);

	return cOk;


}


#ifdef HAS_SyscallManager_task_stopSyscallCfd
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

#ifdef HAS_SyscallManager_task_resumeSyscallCfd
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

#ifdef HAS_SyscallManager_sleepSyscallCfd
int sleepSyscall( int4 int_sp ) {
    int t;
    SYSCALLGETPARAMS1(int_sp,(void*)t);

    LOG(SYSCALLS,DEBUG,(SYSCALLS,DEBUG,"Syscall: sleep(%d)",t));

    pCurrentRunningThread->sleep( (t * (CLOCK_RATE / 1000000) * 1000));

    // return will not be called
    // if the system works correctly
    return cOk;
}
#endif

#define MEM_NO_FREE

#ifdef HAS_SyscallManager_thread_createSyscallCfd
int thread_createSyscall( int4 int_sp ) {
    int* threadid;
    thread_attr_t* attr;
    void* start_routine;
    void* arg;

    SYSCALLGETPARAMS4(int_sp,threadid,attr,start_routine,arg);

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
			 rt->relativeDeadline = attr->deadline * (CLOCK_RATE / 1000000);
			 rt->executionTime = attr->executionTime * (CLOCK_RATE / 1000000);
			 rt->period = attr->period * (CLOCK_RATE / 1000000);
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
                    new ThreadCfdCl( start_routine, (void*) tasktable->task_thread_exit_addr, pCurrentRunningTask, pCurrentRunningTask->getMemManager(), attr->stack_size, attr );
#endif

    // set the return value for threadid
    *threadid = newthread->getId();

    LOG(SYSCALLS,INFO,(SYSCALLS,INFO,"Syscall: thread_created with id %d",*threadid));
    LOG(SYSCALLS,INFO,(SYSCALLS,INFO,"Syscall: thread_created stack: %x - %x, top:%d",newthread->threadStack.startAddr,newthread->threadStack.endAddr,newthread->threadStack.top));

    // set the arguments of the new thread
    newthread->arguments = arg;

    return cOk;
}
#endif

#ifdef HAS_SyscallManager_thread_runSyscallCfd
int thread_run( int4 int_sp ) {
    int threadid;

    SYSCALLGETPARAMS1(int_sp,(void*) threadid);

    LOG(SYSCALLS,INFO,(SYSCALLS,INFO,"Syscall: thread_run(%d)",threadid));


#ifdef REALTIME
    unint8 currentTime = theOS->getClock()->getTimeSinceStartup();
#endif

    if ( threadid >= cFirstThread ) {
        // run the given thread
        register ThreadCfdCl* t = pCurrentRunningTask->getThreadbyId( threadid );

        if ( t != 0 && t->isNew() && !t->isReady() ) {

            LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: Thread run valid"));


#ifdef REALTIME
            // we are realtime so set the arrival time
            t->arrivalTime = currentTime + t->phase;
            theOS->getCPUScheduler()->computePriority(t);
#endif
            // announce to scheduler!
            t->run();
        } else return cThreadNotFound;
    }
    else {
        // run all new threads of this task!
        // if we are realtime set the arrivaltime of all threads!
        // the arrivaltime will be the same then which is important
        // if all threads shall start at the same time (no phase shifting)
        LinkedListDatabase* threadDb = pCurrentRunningTask->getThreadDB();
        LinkedListDatabaseItem* litem = threadDb->getHead();

        while ( litem != 0 ) {
            ThreadCfdCl* thread = (ThreadCfdCl*) litem->getData();

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

    return cOk;
}
#endif

#ifdef HAS_SyscallManager_thread_selfSyscallCfd
int thread_self(int4 int_sp)
{
    return pCurrentRunningThread->getId();
}
#endif

#ifdef HAS_SyscallManager_thread_yieldSyscallCfd
int thread_yield(int4 int_sp)
{
    // dispatch directly
    theOS->getCPUDispatcher()->dispatch(theOS->getClock()->getTimeSinceStartup() - lastCycleStamp);
    return cOk;
}
#endif

#ifdef HAS_SyscallManager_signal_waitSyscallCfd
int signal_wait( int4 int_sp ) {
    void* sig;
    bool memAddrAsSig;
    SYSCALLGETPARAMS2( int_sp, sig, memAddrAsSig );

#ifdef HAS_MemoryManager_HatLayerCfd
    // if we want to use a Memory Address as the signal, we have to get the physical Address
    if (memAddrAsSig) {
        sig = (void*) theOS->getHatLayer()->getPhysicalAddress( sig );
    }
#endif //HAS_MemoryManager_HatLayerCfd

    pCurrentRunningThread->sigwait( sig );

    return cOk;
}
#endif

#ifdef HAS_SyscallManager_signal_signalSyscallCfd
int signal_signal( int4 int_sp ) {
    void* sig;
    bool memAddrAsSig;
    SYSCALLGETPARAMS2( int_sp, sig, memAddrAsSig );

#ifdef HAS_MemoryManager_HatLayerCfd
    // if we want to use a Memory Address as the signal, we have to get the physical Address
    if (memAddrAsSig) {
        sig = (void*) theOS->getHatLayer()->getPhysicalAddress( sig );
    }
#endif //HAS_MemoryManager_HatLayerCfd

    theOS->getCPUDispatcher()->signal( sig );

    return cOk;
}
#endif

#ifdef HAS_SyscallManager_newSyscallCfd
int newSyscall( int4 int_sp ) {
    size_t size;
    SYSCALLGETPARAMS1(int_sp,(void*) size);
    int retval;

    LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: Thread new called. size: %d",size));

    retval = (int) pCurrentRunningTask->getMemManager()->alloc( size, true );

    ASSERT(retval);

    //LOG(SYSCALLS,WARN,(SYSCALLS,WARN,"Syscall: assigned memory at: 0x%x",retval));

    return retval;
}
#endif


int mallocp(int4 sp_int) {
    size_t size;
    int protection_mode;
    SYSCALLGETPARAMS2(sp_int,(void*) size,protection_mode);
    int retval;

    //printf("mallocp: protection %d size %d \n\r",protection_mode,size);

#if USE_AIS_MEM
	// call AIS MemManager
   	retval = (int) theOS->getBoard()->aisMemManager->allocp(size,protection_mode);
#else
    retval = (int) pCurrentRunningTask->getMemManager()->alloc( size, true );
#endif

    //printf("mallocp: assigned memory at: 0x%x\n\r",retval);

    ASSERT(retval);

    return retval;
}

int getTime(int4 sp_int)
{
	unint8* time;
	SYSCALLGETPARAMS1(sp_int,time);
	*time = theOS->getBoard()->getClock()->getTimeSinceStartup();
	return cOk;
}

#ifdef HAS_SyscallManager_deleteSyscallCfd
int deleteSyscall( int4 int_sp ) {
    void* addr;
    SYSCALLGETPARAMS1(int_sp,(void*) addr);
    int retval;

    //LOG(SYSCALLS,WARN,(SYSCALLS,WARN,"Syscall: free(%x)",addr));
    retval = (int) pCurrentRunningTask->getMemManager()->free( addr );

    if (retval != cOk) {
    	ASSERT(0);
    }

    return retval;
}
#endif

#ifdef HAS_SyscallManager_fputcSyscallCfd
int fputcSyscall( int4 int_sp ) {
    short c;
    int stream;
    int retval;

    SYSCALLGETPARAMS2(int_sp, c, stream);

    LOG(SYSCALLS,DEBUG,(SYSCALLS,DEBUG,"Syscall: fputc(%d,%d)",c,stream));

    register Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( stream );
    if ( res != 0 ) {
        // resource valid and owned
        // check if resource is a characterdevice
        if ( ( res->getType() == cStreamDevice ) || ( res->getType() == cCommDevice ) ) {
            // we are ok to write onto this resource
            retval = cOk;
            ( (CharacterDeviceDriver*) res )->writeByte( c );

            LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: Valid Resource %d",stream));

        }
        else
            retval = cResourceNotWriteable;

    }
    // maybe resource not owned or resource doesnt exist
    else
        retval = cResourceNotOwned;

    return retval;
}
#endif

#ifdef HAS_SyscallManager_fgetcSyscallCfd
int fgetcSyscall( int4 int_sp ) {
    char c;
    int stream;
    int retval;

    SYSCALLGETPARAMS1(int_sp,(void*) stream);

    LOG(SYSCALLS,DEBUG,(SYSCALLS,DEBUG,"Syscall: fgetc(%d,%d)",c,stream));

    register Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( stream );
    if ( res != 0 ) {
        // resource valid and owned
        // check if resource is a characterdevice
        if ( ( res->getType() == cStreamDevice ) || ( res->getType() == cCommDevice ) ) {

            LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: Valid Resource %d",stream));

            // we are ok to read this resource
            retval=  ((CharacterDeviceDriver*) res )->readByte( &c );
        }
        else
            retval = cResourceNotReadable;

    }
    // maybe resource not owned or resource doesnt exist
    else
        retval = cResourceNotOwned;

    return retval;
}
#endif



#ifdef HAS_SyscallManager_fcreateSyscallCfd
int fcreateSyscall( int4 int_sp ) {
    char* filename;
    char* path;
    int retval;
    Resource* res;

    SYSCALLGETPARAMS2(int_sp,(void*) filename, (void*) path);
    LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: fcreate(%s,%s)",filename,path));
    res = theOS->getFileManager()->getDirectory( path );

    if ( res != 0 ) {
    	Directory* dir = (Directory*) res;
    	res = new Resource(cFile,true,filename);
    	dir->add(res);
    	return cOk;
    } else {
    	LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: fcreate(%s,%s) FAILED",filename,path));
    	return cInvalidResource;
    }
}
#endif


#ifdef HAS_SyscallManager_fopenSyscallCfd
int fopenSyscall( int4 int_sp ) {
    char* filename;
    int retval;
    Resource* res;
    int blocking;

    SYSCALLGETPARAMS2(int_sp,(void*) filename,(void*) blocking);

    LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: fopen(%s)",filename));

    res = theOS->getFileManager()->getResource( filename );
    if ( res != 0 ) {
        retval = pCurrentRunningTask->aquireResource( res, pCurrentRunningThread, blocking );
    }
    else {
        retval = cInvalidResource;

        LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: fopen(%s) FAILED",filename));
    }
    return retval;
}
#endif

#ifdef HAS_SyscallManager_fcloseSyscallCfd
int fcloseSyscall( int4 int_sp ) {

    int file_id;
    int retval;
    Resource* res;

    SYSCALLGETPARAMS1(int_sp,(void*) file_id);

    LOG(SYSCALLS,DEBUG,(SYSCALLS,DEBUG,"Syscall: fclose(%d)",file_id));

    res = pCurrentRunningTask->getOwnedResourceById( file_id );
    if ( res != 0 ) {
        retval = pCurrentRunningTask->releaseResource( res, pCurrentRunningThread );

#ifdef HAS_PRIORITY

#if ENABLE_NESTED_INTERRUPTS
        // first to do is disable interrupts now
        _disableInterrupts();

        pCurrentRunningThread->executinginthandler = false;
#endif

        SET_RETURN_VALUE((void*)int_sp,(void*)retval);
        // we may have unblocked a higher priority thread so we need to reschedule now!
        theOS->getCPUDispatcher()->dispatch( theOS->getClock()->getTimeSinceStartup() - lastCycleStamp );
#endif
        return retval;
    }
    else
        return cResourceNotOwned;
}
#endif

#ifdef HAS_SyscallManager_fwriteSyscallCfd
int fwriteSyscall( int4 int_sp ) {
    const char *write_ptr;
    unint4 write_size;
    unint4 write_nitems;
    unint4 write_stream;
    int retval;

    SYSCALLGETPARAMS4(int_sp,(void*) write_ptr,(void*) write_size, (void*) write_nitems,(void*) write_stream);

    LOG(SYSCALLS,DEBUG,(SYSCALLS,DEBUG,"Syscall: fwrite(...,%d,%d,%d)",write_size,write_nitems,write_stream));

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( write_stream );
    if ( res != 0 ) {

        LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: fwrite valid"));

        // resource valid and owned
        // check if resource is a characterdevice
        if ( ( res->getType() == cStreamDevice ) || ( res->getType() == cCommDevice ) ) {
            char* pointer = (char*) write_ptr;
            for ( unint4 i = 0; i < write_nitems; i++ ) {
            	ErrorT result = ( (CharacterDeviceDriver*) res )->writeBytes( pointer, write_size );
            	// check if writing failed
            	if (isError(result)) return result;
                pointer += write_size;
            }
            retval = write_nitems;
        }
        else
            retval = cResourceNotWriteable;

    }
    // maybe resource not owned or resource doesnt exist
    else {
        retval = cResourceNotOwned;

        LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: fwrite on device with id %d failed", write_stream));

    }

    return retval;

}
#endif

int printToStdOut(int4 int_sp )
{
    const char *write_ptr;
    unint4 write_size;
    SYSCALLGETPARAMS2(int_sp,(void*) write_ptr,(void*) write_size);

  // LOG(KERNEL,INFO,(KERNEL,INFO,"Syscall: printToStdOut(%s)",write_ptr));

    if (theOS->getStdOutputDevice() != 0)
        return theOS->getStdOutputDevice()->writeBytes(write_ptr,write_size);
    else return cError;
}


#ifdef HAS_SyscallManager_freadSyscallCfd
int freadSyscall( int4 int_sp ) {
    const char *read_ptr;
    unint4 read_size;
    unint4 read_nitems;
    unint4 read_stream;
    int volatile retval;

    SYSCALLGETPARAMS4(int_sp,(void*) read_ptr, (void*) read_size,(void*) read_nitems,(void*) read_stream);

    LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: fread(...,%d,%d,%d)",read_size,read_nitems,read_stream));

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( read_stream );
    if ( res != 0 ) {

        LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: fread valid"));

        // resource valid and owned
        // check if resource is a characterdevice
        if ( ( res->getType() == cStreamDevice ) || ( res->getType() == cCommDevice ) || ( res->getType() == cDirectory ) || ( res->getType() == cFile )  ) {
            char* pointer = (char*) read_ptr;
            for ( unint4 i = 0; i < read_nitems; i++ ) {

                ErrorT result = ( (CharacterDeviceDriver*) res )->readBytes( pointer, read_size );
                // check if reading failed
                if (isError(result)) return result;

                pointer += read_size;
            }
            // set the return value (bytes read)
            retval = pointer - read_ptr;

        }
        else {
            retval = cResourceNotReadable;

            LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: fread failed. Not a readable device."));

        }

    }
    // maybe resource not owned or resource doesnt exist
    else {
        retval = cResourceNotOwned;

        LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: fread on device with id %d failed", read_stream));

    }
    return retval;
}
#endif

#ifdef HAS_SyscallManager_socketSyscallCfd
int socketSyscall( int4 int_sp ) {
    int domain;
    int type;
    int protocol;
    char* buffer;
    int buffersize;

    SYSCALLGETPARAMS5(int_sp,domain,type,protocol,buffer,buffersize);

    // create new Socket
    Socket* s = new Socket( domain, type, protocol, buffer, buffersize );
    pCurrentRunningTask->aquiredResources.addTail( (DatabaseItem*) s );

    LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: Socket created with id %d",s->getId()));

    // return the id of this new resource (socket)
    return s->getId();
}
#endif

#ifdef HAS_SyscallManager_connectSyscallCfd
int connectSyscall(int4 int_sp) {
	 int socketid;
	 sockaddr* addr;
	 int retval;


	 SYSCALLGETPARAMS2(int_sp,socketid,addr);

	 Resource* res;
	res = pCurrentRunningTask->getOwnedResourceById( socketid );
	if ( res != 0 ) {

		LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: connect valid"));

		// resource valid and owned
		// check if resource is a socket
		if ( res->getType() == cSocket ) {
			retval = ( (Socket*) res )->connect(pCurrentRunningThread, addr );

		}
		else
		retval = cError;

	}
	// maybe resource not owned or resource doesnt exist

	else
	retval = cError;

	return retval;
}
#endif

#ifdef HAS_SyscallManager_listenSyscallCfd
int listenSyscall(int4 int_sp) {
	int socketid;
	int retval;


	SYSCALLGETPARAMS1(int_sp,socketid);

	Resource* res;
	res = pCurrentRunningTask->getOwnedResourceById( socketid );
	if ( res != 0 ) {

		LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: listen "));

		// resource valid and owned
		// check if resource is a socket
		if ( res->getType() == cSocket ) {
			retval = ( (Socket*) res )->listen(pCurrentRunningThread);

		}
		else
		retval = cError;

	}
	// maybe resource not owned or resource doesn't exist

	else
	retval = cError;

	return retval;
}
#endif

#ifdef HAS_SyscallManager_bindSyscallCfd
int bindSyscall( int4 int_sp ) {
    int socketid;
    sockaddr* addr;
    int retval;

    SYSCALLGETPARAMS2(int_sp,socketid,addr);

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( socketid );
    if ( res != 0 ) {

        LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: bind valid"));

        // resource valid and owned
        // check if resource is a socket
        if ( res->getType() == cSocket ) {
            retval = ( (Socket*) res )->bind( addr );

        }
        else
        retval = cError;

    }
    // maybe resource not owned or resource doesnt exist

    else
    retval = cError;

    return retval;
}
#endif

#ifdef HAS_SyscallManager_sendtoSyscallCfd
int sendtoSyscall( int4 int_sp ) {
    int socket;
    const void* buffer;
    size_t length;
    const sockaddr *dest_addr;
    int retval;

    SYSCALLGETPARAMS4(int_sp,socket,buffer,length,dest_addr);

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( socket );
    if ( res != 0 ) {

        LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: sendto valid"));

        // resource valid and owned
        // check if resource is a socket
        if ( res->getType() == cSocket ) {
            retval = ( (Socket*) res )->sendto( buffer, length, dest_addr );

        }
        else
        retval = cError;

    }
    // maybe resource not owned or resource doesnt exist

    else {
        LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: sendto invalid! Resource %d not owned",socket));
        retval = cError;
    }

    return retval;
}
#endif

#ifdef HAS_SyscallManager_recvSyscallCfd
int recvSyscall( int4 int_sp) {
    int socketid;
    char** addressofptrtomsg;
    int flags;
    int retval;
    sockaddr* sender;

    SYSCALLGETPARAMS4(int_sp,socketid,addressofptrtomsg,flags,sender);

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( socketid );
    if ( res != 0 ) {

        LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: recv valid"));

        // resource valid and owned
        // check if resource is a socket
        if ( res->getType() == cSocket ) {
            retval = ( (Socket*) res )->recvfrom( pCurrentRunningThread, addressofptrtomsg, flags, sender );
        }
        else
        retval = cError;

    }
    // maybe resource not owned or resource doesnt exist

    else
    retval = cError;

    return retval;
}
#endif

#ifdef HAS_SyscallManager_add_devaddrSyscallCfd
int add_devaddrSyscall( int4 int_sp ) {

    char* dev;
    int domain;
    unint4 addr;

    SYSCALLGETPARAMS3(int_sp,dev,domain,addr);

    // first find addressprotocol
    AddressProtocol* aproto = theOS->getProtocolPool()->getAddressProtocolbyId( domain );
    if ( aproto != 0 ) {
        //find device
        Resource* res;
        res = theOS->getFileManager()->getResource( dev );
        if ( res != 0 ) {
            // check resource type
            if ( res->getType() == cCommDevice ) {
                // add the new addr to the domain for this device
                return aproto->addLocalDeviceAddress( (CommDeviceDriver*) res, addr );
            }
        }
    }

    return cError;
}
#endif

