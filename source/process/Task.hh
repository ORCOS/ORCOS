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
#include "db/LinkedListDatabase.hh"
#include "db/ArrayDatabase.hh"

struct serializedThread {
    ThreadStack threadStack;
    Bitmap status;
    int4 sleepCycles;
    void* arguments;
#ifdef HAS_PRIORITY
    unint8 phase;
    unint8 initialPriority;
    unint8 effectivePriority;
#endif
#ifdef REALTIME
    unint8 period;
    unint8 relativeDeadline;
    unint8 absoluteDeadline;
    unint8 executionTime;
    unint8 arrivalTime;
    int instance;
#endif
};

class Resource;



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
    ArrayDatabase aquiredResources;

    /*!
     * \brief reference to my databaseitem inside the os tasktable;
     */
    LinkedListDatabaseItem* myTaskDbItem;

    /*!
     *  \brief The database storing the Threads belonging to this task
     */
    LinkedListDatabase threadDb;

protected:
    /*!
     *  \brief The database storing the Threads belonging to this task
     */
    LinkedListDatabase suspendedThreadDb;

    /*!
     *   \brief The memory manager that will be used by the threads belonging to this task
     */
    Kernel_MemoryManagerCfdCl* memManager;

    /*!
     * \brief the tasktable of this task
     */
    taskTable* tasktable;


    /*!
     * brief Flag indicating if this task is stopped.
     */
    bool stopped;

    /*!
     *  \brief The ID of this task
     */
    TaskIdT myTaskId;

    /*!
     *  \brief Initialize the Task Id Counter here
     */
    static TaskIdT globalTaskIdCounter;

    /*!
     * \brief list of free task ids which can be used for task creation
     */
    static ArrayDatabase *freeTaskIDs;

public:

    unint4 platform_flags;

    /*!
     *  \brief Constructor of a task taking the memory manager and the pointer to the tasktable of this task.
     */
    Task( Kernel_MemoryManagerCfdCl* memoryManager, taskTable* tasktbl );

    /*!
     * \brief Constructor for derived classes e.g. WorkerTask
     */
    Task();

    /*!
     *  \brief Destructor of the task
     */
    ~Task();

    //! initialize
    static void initialize() {
        globalTaskIdCounter = cFirstTask;
        freeTaskIDs = new ArrayDatabase(20);

        // workertasks must be mapped to PID 0
        // to ensure they are running with kernel mappings
        // under virtual memory
        // user tasks must start at PID 1 to ensure
        // the kernel page table to be not overwritten
		#if USE_WORKERTASK
        for (unint4 i = 0; i < 20; i++) {
		#else
        for (unint4 i = 1; i < 20; i++) {
		#endif
        	freeTaskIDs->addTail((DatabaseItem*) i);
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
    void addThread( Thread* t );

    /*!
     * \brief Removes a given Thread.
     */
    void removeThread( Thread* t );

    /*!
     * \brief Ask this task to try to aquire the resource res
     */
    ErrorT aquireResource( Resource* res, Thread* t , bool blocking = true);

    /*!
     * \brief Ask this task to try to aquire the resource res
     */
    ErrorT releaseResource( Resource* res, Thread* t );

    /*!
     * \brief Returns the thread given by id. May be null if non existend in this task.
     */
    Kernel_ThreadCfdCl* getThreadbyId( ThreadIdT threadid );

    /*!
     *   \brief Get the identity of this task
     */
    inline TaskIdT getId() const {
        return (myTaskId);
    }

    /*!
     *  \brief Return the Memory Manager of the Task
     */
    inline Kernel_MemoryManagerCfdCl* getMemManager() {
        return (memManager);
    }

    /*!
     *  \brief Set the Memory Manager of the Task
     */
    void setMemManager( Kernel_MemoryManagerCfdCl* mm ) {
        memManager = mm;
    }

    /*!
     * \brief Get the resource with id 'id' owned by this resource. May return null if not owned or not existend.
     */
    Resource* getOwnedResourceById( ResourceIdT id );

    /*!
     * \brief Returns the tasktable of this task.
     */
    inline
    taskTable* getTaskTable() {
        return (tasktable);
    }
    ;

    /*!
     * \brief Returns a ThreadCB of a suspended thread which has a stack that is >= stacksize
     */
    LinkedListDatabaseItem* getSuspendedThread(unint4 stacksize);

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
    LinkedListDatabase* getThreadDB() {
        return (&threadDb);
    }
    ;

};

#endif /* _TASK_HH */

