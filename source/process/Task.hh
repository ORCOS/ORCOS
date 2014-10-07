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

#ifndef _TASK_HH
#define _TASK_HH

#include "SCLConfig.hh"
#include "inc/const.hh"
#include "inc/types.hh"
#include Kernel_Thread_hh
#include Kernel_MemoryManager_hh
#include "db/LinkedList.hh"
#include "db/ArrayList.hh"
#include "inc/stringtools.hh"
#include "hal/CharacterDevice.hh"
#include "filesystem/Directory.hh"

struct serializedThread {
    ThreadStack threadStack;
    Bitmap status;
    TimeT sleepCycles;
    void* arguments;
#ifdef HAS_PRIORITY
    TimeT phase;
    TimeT initialPriority;
    TimeT effectivePriority;
#endif
#ifdef REALTIME
    TimeT period;
    TimeT relativeDeadline;
    TimeT absoluteDeadline;
    TimeT executionTime;
    TimeT arrivalTime;
    int instance;
#endif
};

class Resource;

#ifndef MAX_NUM_TASKS
#define MAX_NUM_TASKS 16
#endif

/*!
 * \brief Task CB Class which holds information about a task and its threads.
 * \ingroup process
 *
 * Task inherit from ScheduleableItem because they need to be scheduled in the
 * Mutex class.
 *
 */
class Task: public ScheduleableItem {

    friend class MigrationManager;
    friend class TaskManager;

public:
    /*!
     * \brief The database storing the actually aquired resources.
     */
    ArrayList           aquiredResources;

    /*!
     *  \brief The database storing the Threads belonging to this task
     */
    LinkedList          threadDb;

    /*!
     *  \brief the exit Value of the task == exit value of last terminated task
     */
    int                 exitValue;

protected:
    /*!
     *  \brief The database storing references to suspended (removed) threads
     *         These might come handy for zombie mode threads.
     */
   // LinkedList          suspendedThreadDb;

    /*!
     *   \brief The memory manager that will be used by the threads belonging to this task
     */
    Kernel_MemoryManagerCfdCl* memManager;

    /*!
     * \brief the tasktable of this task
     */
    taskTable*          tasktable;

    /*!
     * brief Flag indicating if this task is stopped.
     */
    bool                stopped;

    /*!
     *  \brief The ID of this task
     */
    TaskIdT             myTaskId;

    /*!
     *  \brief Initialize the Task Id Counter here
     */
    static TaskIdT      globalTaskIdCounter;

    /*!
     * \brief List of free task ids which can be used for task creation
     */
    static ArrayList    *freeTaskIDs;

    /*!
     * \brief The name of this task. Typically its filename
     */
    char               name[32];

    /*!
     * \brief The current working directory of this task.
     */
    char               workingDirectory[256];

    /*!
     * \brief The Device this task is writing to by default as e.g. by printf().
     */
    CharacterDevice*   stdOutput;


    /*!
     * \brief Our SysFs Directory
     */
    Directory*          sysFsDir;

public:


    /*!
     * \brief platform specific flags which are used upon task start by the
     *        platform code. Copy of the task table entry (platform) for access
     *        out of the VM of the process.
     */
    unint4              platform_flags;

    /*!
     *  \brief Constructor of a task taking the memory manager and the pointer to the tasktable of this task.
     */
    Task( Kernel_MemoryManagerCfdCl* memoryManager, taskTable* tasktbl);

    /*!
     * \brief Constructor only used for derived classes e.g. WorkerTask
     */
    Task();

    /*!
     *  \brief Destructor of the task
     */
    ~Task();

    /*!
     * static Task initialization.
     */
    static void initialize() {
        globalTaskIdCounter = cFirstTask;
        freeTaskIDs = new ArrayList(MAX_NUM_TASKS);

        /* Workertasks must be mapped to PID 0
         to ensure they are running with kernel mappings
         under virtual memory
         user tasks must start at PID 1 to ensure
         the kernel page table to be not overwritten */
#if USE_WORKERTASK
        for (unint4 i = 0; i < MAX_NUM_TASKS; i++)
        {
#else
            for (unint4 i = 1; i < MAX_NUM_TASKS; i++)
            {
#endif
            freeTaskIDs->addTail((ListItem*) i);
        }
    }

    /*!
     * \brief Terminates the task.
     *
     * This will cause all threads to be terminated, resources to be released and connections (sockets) to be destroyed.
     */
    void terminate();

    /*!
     * \brief Run this task.
     */
    void run();

    /*!
     * \brief Adds a new thread to this task.
     */
    inline void addThread(Thread* t) {
        this->threadDb.addTail(t);
    }

    /*!
     * \brief Removes a given Thread.
     */
    void removeThread(Thread* t);

    /*!
     * \brief Ask this task to try to aquire the resource res
     */
    ErrorT acquireResource(Resource* res, Thread* t, bool blocking = true);

    /*!
     * \brief Ask this task to try to aquire the resource res
     */
    ErrorT releaseResource(Resource* res, Thread* t);

    /*!
     * \brief Returns the thread given by id. May be null if non existend in this task.
     */
    Kernel_ThreadCfdCl* getThreadbyId(ThreadIdT threadid);

    /*!
     *   \brief Get the identity of this task
     */
    inline TaskIdT getId() const {
        return (myTaskId);
    }

    /*!
     * \brief Reurns the name of this task
     */
    inline const char* getName() const {
           return (this->name);
    }

    /*!
     *  \brief Return the Memory Manager of the Task
     */
    inline Kernel_MemoryManagerCfdCl* getMemManager() const {
        return (memManager);
    }

    /*!
     *  \brief Set the Memory Manager of the Task
     */
    void setMemManager( Kernel_MemoryManagerCfdCl* mm) {
        memManager = mm;
    }

    /*!
     * \brief Sets he name of this task
     */
    void setName(const char* newname) {
        strncpy(this->name,newname,32);
    }

    void setStdOut(CharacterDevice* dev) {
        this->stdOutput = dev;
    }

    /*!
     * \brief Get the resource with id 'id' owned by this resource. May return null if not owned or not existend.
     */
    Resource* getOwnedResourceById(ResourceIdT id);

    /*!
     * \brief Returns the tasktable of this task.
     */
    inline taskTable* getTaskTable() const {
        return (tasktable);
    }


    /*!
     * Returns the sysFs directory for this task containing
     * it exported variables
     */
    Directory*      getSysFsDirectory() {
        return (sysFsDir);
    }

    /*!
     * \brief Returns a ThreadCB of a suspended thread which has a stack that is >= stacksize
     */
    //LinkedListItem* getSuspendedThread(unint4 stacksize);

    /*!
     * \brief Returns the stdOutput of this task.
     */
    inline CharacterDevice* getStdOutputDevice() const {
        return (stdOutput);
    }

    /*!
     * \brief Stops the execution of this task until resumed or destroyed.
     */
    void stop();

    /*!
     * \brief Resumes the execution of this task if stopped.
     */
    void resume();

    /*!
     *  \brief Returns the ID of the Task, who will be created next
     */
    static TaskIdT getIdOfNextCreatedTask();

    /*!
     *  \brief Returns the Database of all Threads of this task
     *
     */
    inline LinkedList* getThreadDB() {
        return (&threadDb);
    }
    ;

};

#endif /* _TASK_HH */

