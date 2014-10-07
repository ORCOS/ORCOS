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
#include "filesystem/SharedMemResource.hh"
#include "filesystem/KernelVariable.hh"

/* the kernel object */
extern Kernel* theOS;

/* static non-const member variable initialization
   will be executed in ctor */
TaskIdT Task::globalTaskIdCounter;

/* array of free task ids for task creation */
ArrayList *Task::freeTaskIDs;

/*--------------------------------------------------------------------------*
 ** Task::Task
 *---------------------------------------------------------------------------*/
Task::Task( Kernel_MemoryManagerCfdCl* memoryManager, taskTable* tasktbl) :
        aquiredResources(20),
        exitValue(0),
        memManager(memoryManager),
        stopped(false),
        sysFsDir(0),
        platform_flags(0)
{
    myTaskId = (TaskIdT) ((unint4) Task::freeTaskIDs->removeHead());

    /* store reference to my tasktable */
    this->tasktable = tasktbl;
    /* Set initial name and working directory */
    strcpy(this->name,"No Name");
    strcpy(this->workingDirectory,"/");
    stdOutput = theOS->getStdOutputDevice();

#if SYSFS_SUPPORT
    Directory* dir = KernelVariable::getEntry("tasks",true);
    if (dir) {
        char* idstr = new char[8];
        sprintf(idstr,"%u",myTaskId);
        sysFsDir    = new Directory(idstr);
        dir->add(sysFsDir);

        EXPORT_VARIABLE(sysFsDir,SYSFS_STRING,           workingDirectory,RO);
        EXPORT_VARIABLE(sysFsDir,SYSFS_UNSIGNED_INTEGER, platform_flags,RO);
        EXPORT_VARIABLE(sysFsDir,SYSFS_STRING,           name,RO);
        EXPORT_VARIABLE(sysFsDir,SYSFS_UNSIGNED_INTEGER, myTaskId,RO);
        EXPORT_VARIABLE(sysFsDir,SYSFS_UNSIGNED_INTEGER, stopped,RO);
        EXPORT_VARIABLE_BY_NAME(sysFsDir,"num_resources",SYSFS_UNSIGNED_INTEGER,aquiredResources.numEntries,RO);
        EXPORT_VARIABLE_BY_NAME(sysFsDir,"num_threads",SYSFS_UNSIGNED_INTEGER,threadDb.size,RO);
        EXPORT_VARIABLE_BY_NAME(sysFsDir,"usedmem",SYSFS_UNSIGNED_INTEGER,memManager->getSegment()->usedBytes,RO);
        EXPORT_VARIABLE_BY_NAME(sysFsDir,"totalmem",SYSFS_UNSIGNED_INTEGER,memManager->getSegment()->memSegSize,RO);
    }
#endif

    /* create initial thread for this task */
    new Kernel_ThreadCfdCl((void*) tasktbl->task_entry_addr, (void*) tasktbl->task_thread_exit_addr, this, memoryManager,
                           DEFAULT_USER_STACK_SIZE, (void*) (&tasktbl->initial_thread_attr), false);


}

Task::Task() :
        aquiredResources(10),
        exitValue(0),
        memManager(0),
        stopped(false),
        sysFsDir(0),
        platform_flags(0)
{
    tasktable = 0;
    myTaskId = (TaskIdT) ((unint4) Task::freeTaskIDs->removeHead());
    /* Set initial name and working directory */
    strcpy(this->name,"No Name");
    strcpy(this->workingDirectory,"/");
    stdOutput = theOS->getStdOutputDevice();

#if SYSFS_SUPPORT
    Directory* dir = KernelVariable::getEntry("tasks",true);
    if (dir) {
          char* idstr = new char[8];
          sprintf(idstr,"%u",myTaskId);
          sysFsDir    = new Directory(idstr);
          dir->add(sysFsDir);

          EXPORT_VARIABLE(sysFsDir,SYSFS_STRING,           workingDirectory,RO);
          EXPORT_VARIABLE(sysFsDir,SYSFS_UNSIGNED_INTEGER, platform_flags,RO);
          EXPORT_VARIABLE(sysFsDir,SYSFS_STRING,           name,RO);
          EXPORT_VARIABLE(sysFsDir,SYSFS_UNSIGNED_INTEGER, myTaskId,RO);
          EXPORT_VARIABLE(sysFsDir,SYSFS_UNSIGNED_INTEGER, stopped,RO);
          EXPORT_VARIABLE_BY_NAME(sysFsDir,"num_resources",SYSFS_UNSIGNED_INTEGER,aquiredResources.numEntries,RO);
          EXPORT_VARIABLE_BY_NAME(sysFsDir,"num_threads",SYSFS_UNSIGNED_INTEGER,threadDb.size,RO);
    }
#endif
}

Task::~Task() {
    delete this->memManager;

#if SYSFS_SUPPORT
    Directory* dir = KernelVariable::getEntry("tasks",true);
    if (dir){
        dir->remove(sysFsDir);
        delete sysFsDir;
    }
#endif
}

Resource* Task::getOwnedResourceById(ResourceIdT id) {
    /* parse database for a resource with id 'id' */
    DISABLE_IRQS(status);
    for (int i = 0; i < this->aquiredResources.size(); i++)
    {
        Resource* res = (Resource*) aquiredResources.getItemAt(i);
        if (res == 0) {
            LOG(PROCESS, ERROR, "Task::getOwnedResourceById res==null");
            return (0);
        }
        if (res->getId() == id) {
            RESTORE_IRQS(status);
            return (res);

        }
    }
    RESTORE_IRQS(status);
    return (0);
}

ErrorT Task::acquireResource(Resource* res, Thread* t, bool blocking) {
    //REMARK: check whether t is a thread belonging to this task

    if (res == 0) {
        LOG(PROCESS, ERROR, "Task::acquireResource res==null");
        return (cError);
    }
    // check whether the task already owns this resource
    if (getOwnedResourceById(res->getId()) != 0)
    {  // task already owns this resource

        LOG(PROCESS, TRACE, "Task: Resource already owned");
        // increase the reference count of the res in this task
        // return the id of the resource so thread can continue working
        return (res->getId());
    }
    else
    {
        LOG(PROCESS, TRACE, "Task: acquiring resource %d", res->getId());
        int error = res->acquire(t, blocking);
        if (isError(error)) {
            LOG(PROCESS,ERROR,"Task::aquireResource() acquire failed: %d",error);
        }
        return (error);
    }
}

ErrorT Task::releaseResource(Resource* res, Thread* t) {
    /* REMARK: we might alos check whether t is a thread belonging to this task
       get the resource to close by id from the tasks owned resource database */
    if (res != 0)
    {
        if (res->getType() == cSocket)
        {
            res->release(t);
            Socket* s = (Socket*) res;
            LOG(KERNEL, DEBUG, "Task::removeThread(): destroying socket!");
            delete s;
            return (cOk );
        }
        else if (res->getType() == cSharedMem)
        {
            SharedMemResource* shmres = (SharedMemResource*) res;
            shmres->unmapFromTask(t->getOwner());
            int retval = shmres->release(t);
            /* Cleanup unused areas */
            if (shmres->getMappedCount() == 0)
                delete shmres;

            return (retval);
        }


       int error = res->release(t);
       if (isError(error)) {
           LOG(PROCESS,ERROR,"Task::releaseResource() release failed: %d",error);
       }
       return (error);
    }

    return (cError );
}

void Task::run() {
    /* run the very first thread!
       this is supposed to be the thread at the head of the threaddb */
    LinkedListItem* litem = this->threadDb.getHead();
    if (litem != 0)
    {
        Kernel_ThreadCfdCl* thread = (Kernel_ThreadCfdCl*) litem->getData();
        /* announce the thread to the scheduler (this does not mean running it directly) */
        thread->run();
    }
}

void Task::terminate() {
    LOG(KERNEL, DEBUG, "Task::terminate()");

    LinkedListItem* litem = this->threadDb.getHead();
    while (litem != 0)
    {
        Thread* t = (Thread*) litem->getData();
        t->terminate();
        litem = litem->getSucc();
    }

    /* add our id back to the database */
    Task::freeTaskIDs->addTail((ListItem*) (unint4) this->getId());

}

void Task::removeThread(Thread* t) {
    LOG(KERNEL, DEBUG, "Task::removeThread() removing %x",t);

    LinkedListItem* litem = this->threadDb.getItem(t);
    if (litem != 0)
    {
        /* ok valid thread. remove it from its database (this->threadDb) */
        litem->remove();
        delete litem;

        /* check if all threads are terminated */
        if (this->threadDb.isEmpty())
        {
            /* no more threads. this task can be destroyed.
             * free all acquired resources if applicable */

            LOG(KERNEL, WARN, "Task::removeThread(): being destroyed! aq_size = %d", this->aquiredResources.size());
            Resource* res = (Resource*) this->aquiredResources.getHead();
            while (res != 0)
            {
                LOG(KERNEL, DEBUG,"Task::removeThread(): releasing resource %s, Id: %d",res->getName(),res->getId());
                /* the last thread forces all resources to be released */
                res->release(t);

                if (res->getType() == cSocket)
                {
                    Socket* s = (Socket*) res;
                    LOG(KERNEL, DEBUG, "Task::removeThread(): destroying socket!");
                    delete s;
                }

                res = (Resource*) this->aquiredResources.getHead();
            }
        }

        t->owner = 0;
    }
}

Kernel_ThreadCfdCl* Task::getThreadbyId(ThreadIdT threadid) {
    LinkedListItem* litem = threadDb.getHead();

    while (litem != 0 && ((Kernel_ThreadCfdCl*) litem->getData())->getId() != threadid)
        litem = litem->getSucc();

    if (litem != 0)
        return ((Kernel_ThreadCfdCl*) litem->getData());
    else
        return (0);
}

TaskIdT Task::getIdOfNextCreatedTask() {
    return ((TaskIdT) ((unint4) Task::freeTaskIDs->getHead()));
}

void Task::stop() {
    /* stop the execution of all child threads */
    LinkedListItem* litem = this->threadDb.getHead();
    while (litem != 0)
    {
        ((Thread*) litem->getData())->stop();
        litem = litem->getSucc();
    }
    /* done this task is stopped */
}

void Task::resume() {
    /* resume the execution of all child threads */
    LinkedListItem* litem = this->threadDb.getHead();
    while (litem != 0)
    {
        ((Thread*) litem->getData())->resume();
        litem = litem->getSucc();
    }
    /* done this task is resumed */
}

