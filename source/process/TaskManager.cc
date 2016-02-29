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

#include "TaskManager.hh"
#include <kernel/Kernel.hh>
#include <db/LinkedList.hh>
#include "inc/crc32.h"
#include "assemblerFunctions.hh"

extern Kernel* theOS;
extern Task* pCurrentRunningTask;

/* table containing references to the tasktables of every task (generated by linker), which
 are available at startup, deployed inside the Image file */
extern unint4 tasktable;
extern Kernel* theOS;

#ifndef HAS_Board_HatLayerCfd
extern void* __KERNELEND;
extern void* __RAM_END;
#endif

/*#define DUMP_PAGE_TABLES 0 */

typedef struct {
    taskTable*  tasktable;  /* pointer to task start == its task table*/
    long        size;       /* size of the memory area reserved for the task */
    char*       name;       /* name of this task */
} InitTasks_t;

/*****************************************************************************
 * Method: TaskManager::registerMemPages()
 *
 * @description
 *  Registers all memory pages of the initial tasks at the RamManager as used.
 *---------------------------------------------------------------------------*/
void TaskManager::registerMemPages() {
#ifdef HAS_Kernel_RamManagerCfd
    unint4 num_tasks = tasktable;

    InitTasks_t* initTask = reinterpret_cast<InitTasks_t*>(((unint4) &(tasktable)) + 4);

    for (unint4 i = 1; i <= num_tasks; i++) {
        // get the taskTable of task number i
        taskTable* task_info = initTask->tasktable;
        unint4 size = initTask->size;
        theOS->getRamManager()->markAsUsed((unint4) task_info, ((unint4) task_info) + size - 1, i);
        initTask++;
    }
#endif
}

/*****************************************************************************
 * Method: TaskManager::getTask(int taskId)
 *
 * @description
 *  Returns the task object of the task with given id
 *
 * @params
 *  taskId:     The ID of the task
 *
 * @returns
 *  Task*       The Task with id taskId or null if none
 *---------------------------------------------------------------------------*/
Task* TaskManager::getTask(int taskId) {
    for (LinkedListItem* lldi = this->taskDatabase->getHead(); lldi != 0; lldi = lldi->getSucc()) {
        Task* task = static_cast<Task*>(lldi->getData());
        if (task->getId() == taskId) {
            return (task);
        }
    }

    return (0);
}

/*****************************************************************************
 * Method: TaskManager::getThread(int threadId)
 *
 * @description
 *  Returns the thread object of the thread with given id
 *
 * @params
 *  threadId:     The ID of the thread
 *
 * @returns
 *  Thread*       The Thread with id threadId or null if none
 *---------------------------------------------------------------------------*/
Thread* TaskManager::getThread(int threadId) {
    for (LinkedListItem* lldi = this->taskDatabase->getHead(); lldi != 0; lldi = lldi->getSucc()) {
       Task* task = static_cast<Task*>(lldi->getData());
       Thread* t = task->getThreadbyId(threadId);
       if (t != 0) {
           return (t);
       }
   }

   return (0);
}

/*****************************************************************************
 * Method: handleTaskHeader(taskTable* taskCB, void* &taskHead, unint4 &nextHeader)
 *
 * @description
 *  Tries to handle the current task header at taskHead for the task with the given
 *  taskCB. Returns the next header inside the nextHeader variable. A header can
 *  be a CRC32 header containing the checksum of the task area.
 *
 * @params
 *  taskCB       The task control block of the tasks whichs header is to be checked
 *  taskHead     Pointer to the current header to be checked.
 *
 * @returns
 *  ErrorT:      cOk on success. Error otherwise
 *  nextHeader   The next header
 *---------------------------------------------------------------------------*/
ErrorT handleTaskHeader(taskTable* taskCB, void* &taskHead, unint4 &nextHeader) {
    if (nextHeader == TASK_CB_CRC32) {
#if TASK_CRC32
        taskCRCHeader* crcHeader = reinterpret_cast<taskCRCHeader*>(taskHead);

        /* some simple sanity checks */
        if (crcHeader->crcEnd < crcHeader->crcStart) {
            LOG(PROCESS, ERROR, "CRC End %u < CRC Start %u!", crcHeader->crcEnd, crcHeader->crcStart);
            return (cTaskCRCFailed );
        }

        unint4 crcAreaLen = crcHeader->crcEnd - crcHeader->crcStart;
        // TODO: use valid mapped size
#ifdef HAS_Board_HatLayerCfd
        if (crcAreaLen > PAGESIZE) {
            LOG(PROCESS, ERROR, "Invalid CRC Area: 0x%x - 0x%x", crcHeader->crcStart, crcHeader->crcEnd);
            return (cTaskCRCFailed );
        }
#endif

        /* calculate the crc area start address of the task in the current address space */
        unint4 start_address = ((unint4) taskCB) + (crcHeader->crcStart - (unint4) taskCB->task_start);

        /* calculate the crc */
        unint4 crc = crc32((unsigned char*) start_address, crcAreaLen);
        /* compare with supposed value */
        if (crc != crcHeader->taskCRC32) {
            LOG(PROCESS, ERROR, "CRC Mismatch: 0x%x != 0x%x", crc, crcHeader->taskCRC32);
            return (cTaskCRCFailed );
        }

        LOG(KERNEL, INFO, "CRC: 0x%x [OK]", crc);

        nextHeader = crcHeader->next_header;
        taskHead = crcHeader++;
#endif
        return (cOk );
    }

    LOG(PROCESS, ERROR, "Unknown Header Type: 0x%x", nextHeader);

    return (cInvalidCBHeader );
}

/*****************************************************************************
 * Method: TaskManager::checkValidTask(taskTable* taskCB)
 *
 * @description
 *  Checks the given task by its control block on correctness and validity.
 *  Checks all headers provided by the task.
 *
 * @params
 *  taskCB:     Pointer to the task control block to be checked
 *
 * @returns
 *  int         Error Code
 *---------------------------------------------------------------------------*/
ErrorT TaskManager::checkValidTask(taskTable* taskCB) {
    if (taskCB->task_magic_word != 0x230f7ae9) {
        LOG(PROCESS, ERROR, "Magic Word not found!");
        return (cInvalidCBHeader );
    }
    if (taskCB->task_heap_start <= taskCB->task_start) {
        LOG(PROCESS, ERROR, "Invalid heap location!");
        return (cInvalidCBHeader );
    }
    if ((taskCB->platform & 0xff) != PLATFORM) {
        LOG(PROCESS, ERROR, "Platform Type error!");
        return (cInvalidCBHeader );
    }

#ifndef HAS_Board_HatLayerCfd
    if ( (intptr_t) taskCB->task_heap_start < (intptr_t) &__KERNELEND) return (cInvalidCBHeader);
    if ( (intptr_t) taskCB->task_heap_start > (intptr_t) &__RAM_END) return (cInvalidCBHeader);

    if ( (intptr_t) taskCB->task_entry_addr < (intptr_t) &__KERNELEND) return (cInvalidCBHeader);
    if ( (intptr_t) taskCB->task_entry_addr > (intptr_t) &__RAM_END) return (cInvalidCBHeader);

    // check for overlapping tasks if we do not virtual memoryS
    for (LinkedListItem* lldi = this->taskDatabase->getHead(); lldi != 0; lldi = lldi->getSucc()) {
        Task* task = static_cast<Task*>(lldi->getData());
        // the only task holding no task table would be the workertask!
        if (task->tasktable != 0) {
            if (    task->tasktable->task_start >= taskCB->task_start
                 && task->tasktable->task_start < taskCB->task_end)
                return (cInvalidCBHeader);
        }
    }
#endif

    unint4 nextHeader = taskCB->task_next_header;
    void* taskHeader = (taskCB + 1);

    while (nextHeader != 0) {
        ErrorT ret = handleTaskHeader(taskCB, taskHeader, nextHeader);
        if (isError(ret))
            return (ret);
    }

    return (cOk );
}

/*****************************************************************************
 * Method: TaskManager::initialize()
 *
 * @description
 *  Initializes the task manager creating all initial task
 *  provided inside the kernel image.
 *
 *---------------------------------------------------------------------------*/
void TaskManager::initialize() {
    unint1 num_tasks = tasktable;
    //register Kernel_MemoryManagerCfdCl* OSMemManager = theOS->getMemoryManager();

    LOG(KERNEL, INFO, "Creating Initial Tasks");

    // InitTasks_t* initTask = (InitTasks_t*) ((unint4*) &(tasktable) + 1);
    InitTasks_t* initTask = reinterpret_cast<InitTasks_t*>(((unint4) &(tasktable)) + 4);

    /* create the initial set of tasks */
    for (unint1 i = 1; i <= num_tasks; i++) {
        /* get the taskTable of task number i */
        taskTable* task_info = initTask->tasktable;
        unint4 size          = initTask->size;
        char* taskname       = initTask->name;

        LOG(KERNEL, INFO, "TaskCB @0x%x", task_info);

        /* create the memory manager for the task. The memory manager will be inside the
         * kernel space */
        //void* memaddr = OSMemManager->alloc(sizeof(Kernel_MemoryManagerCfdCl), true);

#ifndef HAS_Board_HatLayerCfd

        if (checkValidTask(task_info) != cOk) {
            LOG(KERNEL, ERROR, "Task invalid.. dropping");
            continue;
        }

        /*
         * create a new Task CB in Kernel Space!! only holds information about the task.
         * The task code itself remains at task_info->task_start_addr
         */
        Task* task = new Task(task_info );

        LOG(KERNEL, INFO, "TaskID %d:", task->getId());
        LOG(KERNEL, INFO, " start at 0x%x, end  at 0x%x" , task->getTaskTable()->task_start, task->getTaskTable()->task_end);
        LOG(KERNEL, INFO, " entry at 0x%x, exit at 0x%x" , task->getTaskTable()->task_entry_addr, task->getTaskTable()->task_thread_exit_addr);
        LOG(KERNEL, INFO, " heap  at 0x%x, size %d" , task->getTaskTable()->task_heap_start, (int) task->getTaskTable()->task_end - (int) task->getTaskTable()->task_heap_start);

#else
        /* we get the ID of the task we will create next */
        TaskIdT tid = Task::getIdOfNextCreatedTask();

        /* get the size of the task area */
        unint4 task_end = (unint4) task_info + size - 1;

        LOG(KERNEL, INFO, "Task %d:", tid);
        LOG(KERNEL, INFO, " start at 0x%x, end at 0x%x", task_info, task_end);

        /* create the vm map for the task! protection = 7 = RWX, ZoneSelect = 3 */
        theOS->getHatLayer()->map(reinterpret_cast<void*>(LOG_TASK_SPACE_START), /* logical base */
                                  reinterpret_cast<void*>(task_info),            /* physical base */
                                  size,                                          /* size of region to be mapped */
                                  hatProtectionExecute | hatProtectionRead | hatProtectionWrite, /* access protection */
                                  3,                                             /* zone selection */
                                  tid,                                           /* Task ID == PID of mapping */
                                  hatCacheWriteBack);


#if DUMP_PAGE_TABLES
        theOS->getHatLayer()->dumpPageTable(tid);
#endif

        /* temporarily map the task into kernel space to allow access to its task table */
        theOS->getHatLayer()->map(reinterpret_cast<void*>(LOG_TASK_SPACE_START),
                                  reinterpret_cast<void*>(task_info),
                                  size,
                                  hatProtectionExecute | hatProtectionRead | hatProtectionWrite, /* access protection */
                                  0,
                                  0,
                                  hatCacheInhibit);

        if (checkValidTask(reinterpret_cast<taskTable*>(LOG_TASK_SPACE_START)) != cOk) {
            LOG(KERNEL, ERROR, "Task invalid.. dropping");
            theOS->getHatLayer()->unmap(reinterpret_cast<void*>(LOG_TASK_SPACE_START), 0);
            continue;
        }

        /* task is valid .. mark the used are at the ram manager */
        theOS->getRamManager()->markAsUsed((unint4) task_info, task_end, tid);

        register taskTable* tt = reinterpret_cast<taskTable*>(LOG_TASK_SPACE_START);
        LOG(KERNEL, INFO, "heap at 0x%x, size %d", tt->task_heap_start, tt->task_end - tt->task_heap_start);

        /*
         * create a new Task CB in Kernel Space!! only holds information about the task.
         * The task code itself remains at task_info->task_start_addr
         */
        Task* task = new Task(tt);
        if (taskname != 0)
            task->setName(taskname);
        task->platform_flags = tt->platform;
#endif

        this->taskDatabase->addTail(task);
        task->run();

#ifdef HAS_Board_HatLayerCfd
        /* remove the temporary mapping */
        theOS->getHatLayer()->unmap(reinterpret_cast<void*>(LOG_TASK_SPACE_START), 0);
#endif
        initTask++;
    }
}

/*****************************************************************************
 * Method: TaskManager::removeTask(Task* task)
 *
 * @description
 *  Removes the given task from the system thereby terminating all its threads
 *  and freeing its memory.
 *
 * @params
 *  task:     The task to be removed from the system
 *
 * @returns
 *  int       Error Code
 *---------------------------------------------------------------------------*/
ErrorT TaskManager::removeTask(Task* task) {
    if (task == 0)
        return (cError );

    LOG(KERNEL, INFO, "TaskManager::removeTask: removing task %u", task->getId());
#ifdef HAS_Kernel_RamManagerCfd
    /* mark the used pages as free again */
    theOS->getRamManager()->freeAll(task->getId());
#endif

    task->terminate();
    this->taskDatabase->remove(task);
    delete task;

    return (cOk );
}

/*****************************************************************************
 * Method: TaskManager::loadTaskFromFile(File* file,
 *                                       TaskIdT& tid,
 *                                       char* arguments,
 *                                       unint2 arg_length)
 *
 * @description
 *  Tries to load a task from a given file.
 *
 * @params
 *  arguments:    Pointer to the arguments passed to the new task
 *  arg_length:   Length of the arguments area in bytes.
 *
 * @returns
 *  int         Error Code
 *  tid         The ID of the new task
 *---------------------------------------------------------------------------*/
ErrorT TaskManager::loadTaskFromFile(File* file, TaskIdT& tid, char* arguments, unint2 arg_length) {
#ifndef HAS_Kernel_RamManagerCfd
    return (cError);
#endif

    /* task loading only supported if virtual memory is activated */
#ifndef HAS_Board_HatLayerCfd
    return (cError);
#else

    LOG(KERNEL, DEBUG, "TaskManager::loadTaskFromFile: loading file.");

    /* we get the ID of the task we will create next */
    tid = Task::getIdOfNextCreatedTask();
    if (tid == 0) {
        LOG(KERNEL, WARN, "TaskManager: no more free Task IDs.");
        return (cError );
    }

    /* be sure TLB entries are unmapped for the new TID */
    theOS->getHatLayer()->unmapAll(tid);

#ifdef HAS_Board_CacheCfd
    /* invalidate cache entries of the new task id (asid) */
    theOS->getBoard()->getCache()->invalidate(tid);
#endif

    unint4 task_size  = (unint4) alignCeil((char*) file->getFileSize(), 0x100000);
    unint4 task_start = (unint4) theOS->getRamManager()->alloc_physical(task_size, tid);
    LOG(KERNEL, INFO, "TaskManager: new Task will be placed at 0x%x", task_start);

    /* no free page available */
    if (task_start == 0) {
        return (cError);
    }

    /* load the file into memory
     * create a temporary vm map ..
     * disable cache for this mapping as the data must be written directly to main memory
     * for the instruction cache to fetch the correct bytes*/
    theOS->getHatLayer()->map(reinterpret_cast<void*>(task_start), /* 1:1 mapping */
                              reinterpret_cast<void*>(task_start),
                              task_size,
                              hatProtectionRead | hatProtectionWrite,
                              3,
                              pCurrentRunningTask->getId(),
                              hatCacheInhibit);

#if DUMP_PAGE_TABLES
    theOS->getHatLayer()->dumpPageTable(pCurrentRunningTask->getId());
#endif

    /* read the file into memory */
    unint4 size = file->getFileSize();

    /* TODO: this is a fix to be sure the task is loaded correctly...
     * however, the current running task may be reading / modifying it? (should this be possible)
     * thats why we need to reset the position to the beginning. */
    file->resetPosition();

    /* read the complete task into main memory */
    ErrorT error = file->readBytes(reinterpret_cast<char*>(task_start), size);

    if (error < 0) {
        LOG(KERNEL, ERROR, "TaskManager::loadTaskFromFile: Error reading file: %d", error);
        theOS->getHatLayer()->unmap(reinterpret_cast<void*>(task_start));
        theOS->getRamManager()->freeAll(tid);
        return (error);
    }

    register taskTable* tt = reinterpret_cast<taskTable*>(task_start);
    error = this->checkValidTask(tt);
    if (error < 0) {
        LOG(KERNEL, ERROR, "TaskManager::loadTaskFromFile: Invalid Task.. checkValidTask() failed: %d", error);
        theOS->getHatLayer()->unmap(reinterpret_cast<void*>(task_start));
        theOS->getRamManager()->freeAll(tid);
        return (error);
    }

    /* align arg_length */
    if (arg_length > 0) {
        arg_length = (unint2) ((arg_length + 4) & (~0x3));
        if (arg_length > 256) {
            arg_length = 256;
        }
    }

    LOG(KERNEL, DEBUG, "TaskManager::loadTaskFromFile: valid task.");

    /* Create the final mapping for the new task! */
    theOS->getHatLayer()->map(reinterpret_cast<void*>(LOG_TASK_SPACE_START),
                              reinterpret_cast<void*>(task_start),
                              task_size,
                              hatProtectionRead | hatProtectionExecute | hatProtectionWrite,
                              3,
                              tid,
                              hatCacheWriteBack);

#if DUMP_PAGE_TABLES
    theOS->getHatLayer()->dumpPageTable(tid);
#endif

    /* create the memory manager for the task. The memory manager will be inside the kernel space */
    LOG(KERNEL, INFO,  "TaskManager: new Task memory: [0x%x - 0x%x]", tt->task_heap_start, tt->task_end);
    LOG(KERNEL, DEBUG, "TaskManager::loadTaskFromFile: creating Task object.");

    /*
     * create a new Task CB in Kernel Space!! only holds information about the task.
     * The task code itself remains at task_info->task_start_addr
     */
    Task* task = new Task(tt);

    /* copy arguments */
    if (arguments != 0) {
        char* args =  reinterpret_cast<char*> (task_start + (tt->task_heap_start - LOG_TASK_SPACE_START));
        memcpy(args, arguments, arg_length);
        args[arg_length] = 0;
        // set argument of the initial thread
        Kernel_ThreadCfdCl* thread = static_cast<Kernel_ThreadCfdCl*>(task->getThreadDB()->getHead()->getData());
        thread->arguments = reinterpret_cast<void*> (tt->task_heap_start);
    }

    /* for future accesses set it to the correct VM address */
    task->tasktable      = reinterpret_cast<taskTable*>(LOG_TASK_SPACE_START);
    task->platform_flags = tt->platform;
    LOG(KERNEL, INFO, "TaskManager::loadTaskFromFile: Platform Flags: 0x%x", task->platform_flags);

    /* unmap the new task from the current task address space */
    theOS->getHatLayer()->unmap(reinterpret_cast<void*>(task_start));

#if DUMP_PAGE_TABLES
    theOS->getHatLayer()->dumpPageTable(pCurrentRunningTask->getId());
#endif

    this->taskDatabase->addTail(task);
    LOG(KERNEL, INFO, "TaskManager::loadTaskFromFile: running Task.");
    task->run();

    return (cOk);
#endif
}

/*****************************************************************************
 * Method: TaskManager::terminateThread(Kernel_ThreadCfdCl* pThread)
 *
 * @description
 *  Terminates the given thread
 *
 * @params
 *  pThread:     The thread to be terminated
 *
 * @returns
 *  int          Error Code
 *---------------------------------------------------------------------------*/
ErrorT TaskManager::terminateThread(Kernel_ThreadCfdCl* pThread) {
    // TODO: SMP
    int irqstatus;
    DISABLE_IRQS(irqstatus);
    Task* ownerTask = pThread->owner;

    /* tell thread it terminated .. cleans up stuff*/
    pThread->terminate();

    /* remove thread from its owner */
    ownerTask->removeThread(pThread);

    /* check if we need to remove the task */
    if (ownerTask->getThreadDB()->isEmpty()) {
        this->removeTask(ownerTask);
    }

    /* finally tell CPU-dispatcher that this thread is gone.. */
    theOS->getDispatcher()->terminate_thread(pThread);
    RESTORE_IRQS(irqstatus);
    return (cOk );
}
