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

#include <filesystem/SysFs.hh>
#include "process/Task.hh"
#include "kernel/Kernel.hh"
#include "filesystem/Resource.hh"
#include "inc/memtools.hh"
#include "filesystem/SharedMemResource.hh"
#include "assemblerFunctions.hh"
#include "inc/signals.hh"

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
Task::Task(taskTable* tasktbl) :
        aquiredResources(20),
        exitValue(0),
        stopped(false),
        sysFsDir(0),
        platform_flags(0) {
    myTaskId = (TaskIdT) ((unint4) Task::freeTaskIDs->removeHead());

    /* store reference to my tasktable */
    this->tasktable = tasktbl;
    /* Set initial name and working directory */
    strcpy(this->name, "No Name");
    strcpy(this->workingDirectory, "/");
    stdOutput = theOS->getStdOutputDevice();

#if SYSFS_SUPPORT
    Directory* dir = KernelVariable::getEntry("tasks", true);
    if (dir) {
        char* idstr = new char[8];
        sprintf(idstr, "%u", myTaskId);
        sysFsDir = new Directory(idstr);
        dir->add(sysFsDir);
        SYSFS_ADD_RO_STRING(sysFsDir, name);
        SYSFS_ADD_RO_STRING(sysFsDir, workingDirectory);
        SYSFS_ADD_RO_UINT(sysFsDir, platform_flags);
        SYSFS_ADD_RO_UINT(sysFsDir, myTaskId);
        SYSFS_ADD_RO_UINT(sysFsDir, stopped);
        SYSFS_ADD_RO_UINT_NAMED(sysFsDir, "num_resources", aquiredResources.numEntries);
        SYSFS_ADD_RO_UINT_NAMED(sysFsDir, "num_threads"  , threadDb.size);
        // TODO: update this using the information of mapped heap pages!
       // SYSFS_ADD_RO_UINT_NAMED(sysFsDir, "usedmem"      , 0);
       // SYSFS_ADD_RO_UINT_NAMED(sysFsDir, "totalmem"     , 0);
    }
#endif

    /* create initial thread for this task */
    new Kernel_ThreadCfdCl(reinterpret_cast<void*>(tasktbl->task_entry_addr),
                           reinterpret_cast<void*>(tasktbl->task_thread_exit_addr),
                           this,
                           DEFAULT_USER_STACK_SIZE,
                           reinterpret_cast<void*>(&tasktbl->initial_thread_attr));
}

Task::Task() :
        aquiredResources(10),
        exitValue(0),
        stopped(false),
        sysFsDir(0),
        platform_flags(0) {
    tasktable = 0;
    myTaskId = (TaskIdT) ((unint4) Task::freeTaskIDs->removeHead());
    /* Set initial name and working directory */
    strcpy(this->name, "No Name");
    strcpy(this->workingDirectory, "/");
    stdOutput = theOS->getStdOutputDevice();

#if SYSFS_SUPPORT
    Directory* dir = KernelVariable::getEntry("tasks", true);
    if (dir) {
        char* idstr = new char[8];
        sprintf(idstr, "%u", myTaskId);
        sysFsDir = new Directory(idstr);
        dir->add(sysFsDir);

        SYSFS_ADD_RO_STRING(sysFsDir, name);
        SYSFS_ADD_RO_STRING(sysFsDir, workingDirectory);
        SYSFS_ADD_RO_UINT(sysFsDir, platform_flags);
        SYSFS_ADD_RO_UINT(sysFsDir, myTaskId);
        SYSFS_ADD_RO_UINT(sysFsDir, stopped);
        SYSFS_ADD_RO_UINT_NAMED(sysFsDir, "num_resources", aquiredResources.numEntries);
        SYSFS_ADD_RO_UINT_NAMED(sysFsDir, "num_threads"  , threadDb.size);
    }
#endif
}

Task::~Task() {
#if SYSFS_SUPPORT
    Directory* dir = KernelVariable::getEntry("tasks", true);
    if (dir) {
        dir->remove(sysFsDir);
        /* schedule deletion instead of directly freeing the memory
         * as some other threads may be accessing the directory contents
         * right now!*/
        theOS->getMemoryManager()->scheduleDeletion(sysFsDir);
    }
#endif
}

/*****************************************************************************
 * Method: Task::getOwnedResourceById(ResourceIdT id)
 *
 * @description
 *   Get the resource with id 'id' owned by this resource.
 *   May return null if not owned or not existent.
 *******************************************************************************/
Resource* Task::getOwnedResourceById(ResourceIdT id) {
    /* parse database for a resource with id 'id' */
    Resource* ret = 0;
    /* resource list of task may be modified by some other thread
     * AND processor.. protect against this using the resource spinlock*/
    aquiredResources.lock();

    /* ID 0 is invalid */
    if (id == 0) {
        goto out;
    }

    for (int i = 0; i < this->aquiredResources.size(); i++) {
        Resource* res = static_cast<Resource*>(aquiredResources.getItemAt(i));
        if (res == 0) {
            LOG(PROCESS, ERROR, "Task::getOwnedResourceById res==null");
            goto out;
        }
        if (res->getId() == id) {
            ret = res;
            goto out;
        }
    }

out:
    aquiredResources.unlock();
    return (ret);
}

/*****************************************************************************
 * Method: Task::acquireResource(Resource* res, Thread* t, bool blocking = true)
 *
 * @description
 *  Ask this task to try to acquire the resource res
 *******************************************************************************/
ErrorT Task::acquireResource(Resource* res, Thread* t, bool blocking) {
    // REMARK: check whether t is a thread belonging to this task
    if (res == 0) {
        LOG(PROCESS, ERROR, "Task::acquireResource res==null");
        return (cError );
    }
    // check whether the task already owns this resource
    if (getOwnedResourceById(res->getId()) != 0) {  // task already owns this resource
        LOG(PROCESS, TRACE, "Task: Resource already owned");
        // increase the reference count of the res in this task
        // return the id of the resource so thread can continue working
        return (res->getId());
    } else {
        LOG(PROCESS, TRACE, "Task: acquiring resource %d", res->getId());
        int error = res->acquire(t, blocking);
        if (isError(error)) {
            LOG(PROCESS, ERROR, "Task::aquireResource() acquire failed: %d", error);
        }
        return (error);
    }
}

/*****************************************************************************
 * Method: Task::releaseResource(Resource* res, Thread* t)
 *
 * @description
 *  Ask this task to try to acquire the resource res
 *******************************************************************************/
ErrorT Task::releaseResource(Resource* res, Thread* t) {
    /* REMARK: we might alos check whether t is a thread belonging to this task
     get the resource to close by id from the tasks owned resource database */
    if (res != 0) {
        if (res->getType() == cSocket) {
            res->release(t);
            Socket* s = static_cast<Socket*>(res);
            LOG(KERNEL, DEBUG, "Task::removeThread(): destroying socket!");
            delete s;
            return (cOk );
        } else if (res->getType() == cSharedMem) {
            SharedMemResource* shmres = static_cast<SharedMemResource*>(res);
            shmres->unmapFromTask(t->getOwner());
            int retval = shmres->release(t);
            /* Cleanup unused areas */
            if (shmres->getMappedCount() == 0 && shmres->getOwner() != 0)
                delete shmres;

            return (retval);
        } else if (res->getType() & cFile) {
            File* pFile = static_cast<File*>(res);
            pFile->onClose();
        }

        int error = res->release(t);
        if (isError(error)) {
            LOG(PROCESS, ERROR, "Task::releaseResource() release failed: %d", error);
        }
        return (error);
    }

    return (cError );
}


/*****************************************************************************
 * Method: Task::run()
 *
 * @description
 *  Run this task.
 *******************************************************************************/
void Task::run() {
    /* run the very first thread!
     this is supposed to be the thread at the head of the threaddb */
    LinkedListItem* litem = this->threadDb.getHead();
    if (litem != 0) {
        Kernel_ThreadCfdCl* thread = static_cast<Kernel_ThreadCfdCl*>(litem->getData());
        /* announce the thread to the scheduler (this does not mean running it directly) */
        thread->run();
    }
}

/*****************************************************************************
 * Method: Task::terminate()
 *
 * @description
 *  Terminates the task.
 *  This will cause all threads to be terminated,
 *  resources to be released and connections (sockets) to be destroyed.
 *******************************************************************************/
void Task::terminate() {
    LOG(KERNEL, DEBUG, "Task::terminate()");


#ifdef ORCOS_SUPPORT_SIGNALS
    int signal =  SIGNAL_SPACE_TASK(this->getId()) | SIG_TASK_TERMINATED;
    theOS->getDispatcher()->signal(reinterpret_cast<void*>(signal), this->exitValue);
#endif

    LinkedListItem* litem = this->threadDb.getHead();
    while (litem != 0) {
        Thread* t = static_cast<Thread*>(litem->getData());
        t->terminate();
        litem = litem->getSucc();
        this->removeThread(t);
    }

    /* add our id back to the database */
    Task::freeTaskIDs->addTail(reinterpret_cast<ListItem*>(this->getId()));
}

/*****************************************************************************
 * Method: Task::removeThread(Thread* t)
 *
 * @description
 *  Removes a given Thread.
 *******************************************************************************/
void Task::removeThread(Thread* t) {
    LOG(KERNEL, DEBUG, "Task::removeThread() removing %x", t);

    LinkedListItem* litem = this->threadDb.getItem(t);
    if (litem != 0) {
        /* ok valid thread. remove it from its database (this->threadDb) */
        litem->remove();
        delete litem;

        /* check if all threads are terminated */
        if (this->threadDb.isEmpty()) {
            /* no more threads. this task can be destroyed.
             * free all acquired resources if applicable */

            LOG(KERNEL, WARN, "Task::removeThread(): being destroyed! aq_size = %d", this->aquiredResources.size());
            Resource* res = static_cast<Resource*>(this->aquiredResources.getHead());
            while (res != 0) {
                this->releaseResource(res, t);
                res = static_cast<Resource*>(this->aquiredResources.getHead());
            }
        }

        t->owner = 0;
    }
}

/*****************************************************************************
 * Method: Task::getThreadbyId(ThreadIdT threadid)
 *
 * @description
 *  Returns the thread given by id. May be null if non existent in this task.
 *******************************************************************************/
Kernel_ThreadCfdCl* Task::getThreadbyId(ThreadIdT threadid) {
    LinkedListItem* litem = threadDb.getHead();

    while (litem != 0 && (static_cast<Kernel_ThreadCfdCl*>(litem->getData())->getId() != threadid)) {
        litem = litem->getSucc();
    }

    if (litem != 0)
        return (static_cast<Kernel_ThreadCfdCl*>(litem->getData()));
    else
        return (0);
}

/*****************************************************************************
 * Method: Task::getIdOfNextCreatedTask()
 *
 * @description
 *  Returns the ID of the Task, who will be created next
 *******************************************************************************/
TaskIdT Task::getIdOfNextCreatedTask() {
    return ((TaskIdT) ((unint4) Task::freeTaskIDs->getHead()));
}

/*****************************************************************************
 * Method: Task::stop()
 *
 * @description
 *  Stops the execution of this task until resumed or destroyed.
 *******************************************************************************/
void Task::stop() {
    /* stop the execution of all child threads */
    LinkedListItem* litem = this->threadDb.getHead();
    while (litem != 0) {
        static_cast<Thread*>(litem->getData())->stop();
        litem = litem->getSucc();
    }
    /* done this task is stopped */
}

/*****************************************************************************
 * Method: Task::resume()
 *
 * @description
 *  Resumes the execution of this task if stopped.
 *******************************************************************************/
void Task::resume() {
    /* resume the execution of all child threads */
    LinkedListItem* litem = this->threadDb.getHead();
    while (litem != 0) {
        static_cast<Thread*>(litem->getData())->resume();
        litem = litem->getSucc();
    }
    /* done this task is resumed */
}

