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

#include "process/Task.hh"
#include "kernel/Kernel.hh"
#include "filesystem/Resource.hh"
#include "inc/memtools.hh"
// the kernel object
extern Kernel* theOS;
extern ThreadCfdCl* pCurrentRunningThread;
extern Task* pCurrentRunningTask;

// static non-const member variable initialization
// will be executed in ctor
TaskIdT Task::globalTaskIdCounter;

ArrayDatabase *Task::freeTaskIDs;

/*--------------------------------------------------------------------------*
 ** Task::Task
 *---------------------------------------------------------------------------*/
Task::Task( MemoryManagerCfdCl* memoryManager, taskTable* tasktbl ) :
    aquiredResources( 10 ), memManager( memoryManager )

{
    //myTaskId = globalTaskIdCounter++;
	myTaskId = (unint4) Task::freeTaskIDs->removeHead();

    // store reference to my tasktable
    this->tasktable = tasktbl;

    // create initial thread for this task
    new ThreadCfdCl( (void*) tasktbl->task_entry_addr, (void*) tasktbl->task_thread_exit_addr, this, memoryManager,
            DEFAULT_USER_STACK_SIZE, (void*) ( tasktbl ), false );

}

Task::Task() :
    aquiredResources( 10 )
{
	tasktable = 0;
    //myTaskId = globalTaskIdCounter++;
	myTaskId = (unint4) Task::freeTaskIDs->removeHead();
}



Task::~Task() {

	LinkedListDatabaseItem* litem = this->suspendedThreadDb.getHead();
	while (litem != 0) {

		Thread* t = (Thread*) litem->getData();
		litem = litem->getSucc();
		this->suspendedThreadDb.remove(t);

		delete t;
	}

	delete this->myTaskDbItem;

	delete this->memManager;
}



Resource* Task::getOwnedResourceById( ResourceIdT id ) {
    // parse database for a resource with id 'id'
    for ( int i = 0; i < this->aquiredResources.size(); i++ ) {
        Resource* res = (Resource*) aquiredResources.getItemAt( i );
        if ( res->getId() == id )
            return res;
    }

    return 0;
}

ErrorT Task::aquireResource( Resource* res, Thread* t, bool blocking ) {
    //REMARK: check wheter t is a thread belonging to this task
    // check whether the task already owns this resource
    if ( getOwnedResourceById( res->getId() ) != 0 ) { // task already owns this resource

        LOG(PROCESS,TRACE,(PROCESS,TRACE,"Task: Resource already owned"));
        // increase the reference count of the res in this task

        // return the id of the resource so thread can continue working
        return res->getId();
    }
    else {
        LOG(PROCESS,TRACE,(PROCESS,TRACE,"Task: aquiring resource"));
        // change .. opening directories is now possible
        res->aquire( t, blocking );
        return cOk;

    }
}

ErrorT Task::releaseResource( Resource* res, Thread* t ) {
    //REMARK: check wheter t is a thread belonging to this task
    // get the resource to close by id from the tasks owned resource database
    if ( res != 0 ) {
        return res->release( t );
    }

    return cError;
}

void Task::run() {
    // run the very first thread!
    // this is supposed to be the thread at the head of the threaddb
    LinkedListDatabaseItem* litem = this->threadDb.getHead();
    if ( litem != 0 ) {
        ThreadCfdCl* thread = (ThreadCfdCl*) litem->getData();

#ifdef REALTIME
        // set the arrival time of this realtime thread
        thread->arrivalTime = theOS->getClock()->getTimeSinceStartup() + thread->phase;
        theOS->getCPUScheduler()->computePriority( thread );
#endif

        // announce the thread to the scheduler (this does not mean running it directly)
        thread->run();
    }
}

void Task::terminate()
{
	 LOG(KERNEL,INFO,(KERNEL,INFO,"Task::terminate()"));
	 LinkedListDatabaseItem* litem = this->threadDb.getHead();
	 while (litem != 0)
	 {
		 Thread* t = (Thread*) litem->getData();

		 // first remove all other threads than the currentRunningThread
		 if (t != pCurrentRunningThread)
			 {
				 t->terminate();
				 litem = this->threadDb.getHead();
			 }
		 else litem = litem->getSucc();
	 }

	 // add our id back to the database
	 Task::freeTaskIDs->addTail((DatabaseItem*) (unint4) this->getId());

	 litem = this->threadDb.getHead();
	 if (litem != 0)
	 {
		 // thread must have been running. terminate it as last thread
		 Thread* t = (Thread*) litem->getData();
		 t->terminate();
	 }

#ifdef HAS_MemoryManager_HatLayerCfd
	 if (pCurrentRunningTask != this)
      theOS->getHatLayer()->unmapAll(this->getId());
#endif

}

void Task::addThread( Thread* t ) {
    this->threadDb.addTail( t );
}

void Task::removeThread( Thread* t ) {
	 LOG(KERNEL,DEBUG,(KERNEL,DEBUG,"Task::removeThread() removing %x",t));

    LinkedListDatabaseItem* litem = this->threadDb.getItem( t );
    if ( litem != 0 ) {
        // ok valid thread. remove it from its database (this->threadDb)
        litem->remove();

        // store in the suspended database
        suspendedThreadDb.addTail(litem);
        LOG(KERNEL,TRACE,(KERNEL,TRACE,"Task::removeThread() added to suspended list"));

        // check if all threads are terminated
        if ( this->threadDb.isEmpty() ) {
            // no more threads. this task can be destroyed.
            // free all acquired resources if applicable

        	LOG(KERNEL,DEBUG,(KERNEL,DEBUG,"Task::removeThread(): being destroyed! aq_size = %d", this->aquiredResources.size()));

            Resource* res = (Resource*) this->aquiredResources.removeTail();
            while ( res != 0 ) {
            	LOG(KERNEL,DEBUG,(KERNEL,DEBUG,"Task::removeThread(): destroying resource of type %d",res->getType()));
                // the last thread forces all resources to be released
                res->release( t );

                if ( res->getType() == cSocket ) {
                	Socket* s = (Socket*) res;
                	LOG(KERNEL,DEBUG,(KERNEL,DEBUG,"Task::removeThread(): destroying socket!"));
                	delete s;
                }

                res = (Resource*) this->aquiredResources.removeTail();
            }

            this->myTaskDbItem->remove();
            // do additional cleanup

        }
    }
}

LinkedListDatabaseItem* Task::getSuspendedThread(unint4 stacksize)
{
	LinkedListDatabaseItem* litem = this->suspendedThreadDb.getHead();
	while (litem != 0)
	{
		ThreadCfdCl* t = (ThreadCfdCl*) litem->getData();
		if ( ((unint4) t->threadStack.endAddr - (unint4) t->threadStack.startAddr) >= stacksize)
		{
			litem->remove();
			return litem;
		}
	}

	return 0;
}

ThreadCfdCl* Task::getThreadbyId( ThreadIdT threadid ) {
    LinkedListDatabaseItem* litem = threadDb.getHead();

    while ( litem != 0 && ( (ThreadCfdCl*) litem->getData() )->getId() != threadid )
        litem = litem->getSucc();

    if ( litem != 0 )
        return (ThreadCfdCl*) litem->getData();
    else
        return 0;
}

TaskIdT Task::getIdOfNextCreatedTask() {
  //  return ( Task::globalTaskIdCounter );
	  return ( (unint4) Task::freeTaskIDs->getHead() );
}

void Task::stop() {
    // stop the execution of all child threads
    LinkedListDatabaseItem* litem = this->threadDb.getHead();
    while ( litem != 0 ) {
        ( (Thread*) litem->getData() )->stop();
        litem = litem->getSucc();
    }
    // done this task is stopped
}

void Task::resume() {
    // resume the execution of all child threads
    LinkedListDatabaseItem* litem = this->threadDb.getHead();
    while ( litem != 0 ) {
        ( (Thread*) litem->getData() )->resume();
        litem = litem->getSucc();
    }
    // done this task is resumed
}

bool Task::serialize( void* buffer, unint2 &length ) {

    // first of all the task control block

    unint2* id = (unint2*) buffer;
    *id = 1; // reserved for the used memory manager id
    id++;

#ifdef REALTIME
    *id = 3;
#else
#ifdef HAS_PRIORITY
    *id = 2;
#else
    *id = 1;
#endif
#endif

    long* tmp = (long*) buffer + 1;
    // first copy the task table
    // copy all long values includes those inside the attribute structure behind the tasktable
    memcpy( tmp, this->tasktable, 88 );
    // + 11 long
    tmp += 11;

    // now store the memory manager
   /* void* s_obj = 0;
    unint2 len;
    this->memManager->serialize( s_obj, len );
    // store length of serialized memory manager object
    *tmp = len;
    tmp++;

    // copy serialized object into our task object stream
    memcpy( tmp, s_obj, len );

    unint1* tmp2 = (unint1*) tmp;
    tmp2 += len;

    // now store the thread control blocks
    *tmp2 = this->threadDb.getSize();
    tmp2++;

    // serialize threads
    for ( unint2 i = 0; i < this->threadDb.getSize(); i++ ) {
        LinkedListDatabaseItem* li = this->threadDb.getHead();
        ThreadCfdCl* t = (ThreadCfdCl*) li->getData();

        // store always needed variables
        serializedThread* st = (serializedThread*) tmp2;
        st->arguments = t->arguments;
        st->sleepCycles = t->sleepCycles;
        st->status = t->status;
        st->threadStack = t->threadStack;

#ifdef HAS_PRIORITY
        st->phase = t->phase;
        st->initialPriority = t->initialPriority;
        st->effectivePriority = t->effectivePriority;
#endif

#ifdef REALTIME
        st->period = t->period;
        st->relativeDeadline = t->relativeDeadline;
        st->absoluteDeadline = t->absoluteDeadline;
        st->executionTime = t->executionTime;
        st->arrivalTime = t->arrivalTime;
        st->instance = t->instance;
#endif

        // all needed thread information stored
        tmp2 += sizeof(serializedThread);
    }

    // now care about the resources!
    // difficult part!!*/

    length = (unint4) tmp - (unint4) buffer;

    return true;
}

Task* Task::deserialize( void* serialized_object, unint2 length, void* new_physical_start_address ) {
    unint2* id = (unint2*) serialized_object;
    id++;
    unint2 myid;

#ifdef REALTIME
    myid = 3;
#else
#ifdef HAS_PRIORITY
    myid = 2;
#else
    myid = 1;
#endif
#endif

    // check whether thread classes match or not
    if ( myid != *id ) {
        LOG(KERNEL,ERROR,(KERNEL,ERROR,"Task::deserialize() : Source Thread Class does not match this nodes Thread Class!\n"));
        return 0;
    }

    long* tmp = (long*) serialized_object + 1;

    Task* task = new Task();
    // store and adapt Tasktable
    // PID must be set and virtual memory must be acitvated with correct mapping
    memcpy( (void*) LOG_TASK_SPACE_START, tmp, 88 );

    task->tasktable = (taskTable*) LOG_TASK_SPACE_START;
    // modify the tasktable to point to the correct new phyiscal address. logical addresses wont change
    task->tasktable->task_heap_end = (long) new_physical_start_address + ( (long) task->tasktable->task_heap_end
            - (long) task->tasktable->task_start_addr );
    task->tasktable->task_heap_start = (long) new_physical_start_address + ( (long) task->tasktable->task_heap_start
            - (long) task->tasktable->task_start_addr );
    task->tasktable->task_start_addr = (long) new_physical_start_address;

    // create the memory manager for the task
    void* memaddr = theOS->getMemManager()->alloc(sizeof(MemoryManagerCfdCl),false);

    MemoryManagerCfdCl* task_memManager =
                      new(memaddr)  MemoryManagerCfdCl((void*) (LOG_TASK_SPACE_START + (  (long) task->tasktable->task_heap_start -  (long) task->tasktable->task_start_addr)),
                              (void*) (LOG_TASK_SPACE_START +  (long) task->tasktable->task_heap_end - (long) task->tasktable->task_start_addr ) );

    task->memManager = task_memManager;

    /*tmp += 11;
    unint2 len = *tmp;
    tmp++;
    MemoryManagerCfdCl* memManager = MemoryManagerCfdCl::deserialize( (void*) tmp, len );

    if ( memManager == 0 ) {
        LOG(KERNEL,ERROR,(KERNEL,ERROR,"Task::deserialize() : Deserialization of Memory Manager failed!\n"));
        return 0;
       }


    task->memManager = memManager;

    unint1* tmp2 = (unint1*) tmp;
    tmp2 += len;

    unint1 numthreads = *tmp2;
    tmp2++;

    // now restore the threads
    for ( unint1 i = 0; i < numthreads; i++ ) {
        serializedThread* st = (serializedThread*) tmp2;
        ThreadCfdCl* t = (ThreadCfdCl*) theOS->getMemManager()->alloc( sizeof(ThreadCfdCl) );
        t->owner = task;
        t->arguments = st->arguments;
        t->sleepCycles = st->sleepCycles;
        t->status = st->status;
        t->threadStack = st->threadStack;

#ifdef HAS_PRIORITY
        t->phase = st->phase;
        t->initialPriority = st->initialPriority;
        t->effectivePriority = st->effectivePriority;
#endif

#ifdef REALTIME
        t->period = st->period;
        t->relativeDeadline = st->relativeDeadline;
        t->absoluteDeadline = st->absoluteDeadline;
        t->executionTime = st->executionTime;
        t->arrivalTime = st->arrivalTime;
        t->instance = st->instance;
#endif

        t->startRoutinePointer = (void*) task->tasktable->task_entry_addr;
        t->exitRoutinePointer  = (void*) task->tasktable->task_thread_exit_addr;
        task->addThread(t);
    }

    // handle resources
*/
    return task;
}
