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
#include "scheduler/ScheduleableItem.hh"

class Resource;

#ifndef MAX_NUM_TASKS
#define MAX_NUM_TASKS 16
#endif

#define MAX_TASK_RESOURCES 64

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

private:
    /*!
     * \brief The database storing the actually acquired resources.
     */
    Resource*          acquiredResources[MAX_TASK_RESOURCES];

    /*!
     * \brief Current number of acquired resources
     */
    int                numResources;

    /*!
     * \brief Bitmap of free resource IDs used for resource acquisition
     */
    IDMap<MAX_TASK_RESOURCES>  freeResourceIds;


public:
    /*!
     *  \brief Unconditionally adds the Resource to the Task. Only to be used by resources adding themself
     *         to the task after successful acquisition! Threads and other components should definitely use
     *         acquireResource() instead!
     */
    ErrorT              addResource(Resource* res);

    /*!
     *  \brief Removes the sources from the Task. Only to be used by resources!
     */
    ErrorT              removeResource(Resource* res);

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
    static IDMap<MAX_NUM_TASKS>    freeTaskIDs;

    /*!
     * \brief The name of this task. Typically its filename
     */
    char               name[32];

    /*!
     * \brief The current working directory path of this task.
     */
    char               workingDirectoryPath[256];

    /*!
     * \brief The current working directory of this task (corresponds to workingDirectoryPath).
     */
    Directory*         workingDirectory;

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
    explicit Task(taskTable* tasktbl);

    /*!
     * \brief Constructor only used for derived classes e.g. WorkerTask
     */
    Task();

    /*!
     *  \brief Destructor of the task
     */
    ~Task();

    /*****************************************************************************
     * Method: initialize()
     *
     * @description
     *  Static Task initialization.
     *******************************************************************************/
    static void initialize() {
        globalTaskIdCounter = cFirstTask;

        /* Workertasks must be mapped to PID 0
         to ensure they are running with kernel mappings
         under virtual memory
         user tasks must start at PID 1 to ensure
         the kernel page table to be not overwritten */
        #if USE_WORKERTASK
        //freeTaskIDs.invalidateID(0);
        #endif
    }

    /*****************************************************************************
     * Method: terminate()
     *
     * @description
     *  Terminates the task.
     *  This will cause all threads to be terminated,
     *  resources to be released and connections (sockets) to be destroyed.
     *******************************************************************************/
    void terminate();


    /*****************************************************************************
     * Method: run()
     *
     * @description
     *  Run this task.
     *******************************************************************************/
    void run();


    /*****************************************************************************
     * Method: addThread(Thread* t)
     *
     * @description
     *  Adds a new thread to this task.
     *******************************************************************************/
    inline void addThread(Thread* t) {
        this->threadDb.addTail(t);
    }


    /*****************************************************************************
     * Method: removeThread(Thread* t)
     *
     * @description
     *  Removes a given Thread.
     *******************************************************************************/
    void removeThread(Thread* t);


    /*****************************************************************************
     * Method: acquireResource(Resource* res, Thread* t, bool blocking = true)
     *
     * @description
     *  Ask this task to try to aquire the resource res
     *******************************************************************************/
    ErrorT acquireResource(Resource* res, Thread* t, bool blocking = true);


    /*****************************************************************************
     * Method: releaseResource(Resource* res, Thread* t)
     *
     * @description
     *  Ask this task to try to aquire the resource res
     *******************************************************************************/
    ErrorT releaseResource(Resource* res, Thread* t);


    /*****************************************************************************
     * Method: getThreadbyId(ThreadIdT threadid)
     *
     * @description
     *  Returns the thread given by id. May be null if non existent in this task.
     *******************************************************************************/
    Kernel_ThreadCfdCl* getThreadbyId(ThreadIdT threadid);


    /*****************************************************************************
     * Method: getId()
     *
     * @description
     *  Get the identity of this task
     *******************************************************************************/
    inline TaskIdT getId() const {
        return (myTaskId);
    }


    /*****************************************************************************
     * Method: getName()
     *
     * @description
     *  Returns the name of this task
     *******************************************************************************/
    inline const char* getName() const {
           return (this->name);
    }

    /*****************************************************************************
     * Method: setName(const char* newname)
     *
     * @description
     *   Sets he name of this task. Maximum 32 characters
     *******************************************************************************/
    void setName(const char* newname) {
        strncpy(this->name, newname, 32);
    }

    /*****************************************************************************
     * Method: setStdOut(CharacterDevice* dev)
     *
     * @description
     *
     *******************************************************************************/
    void setStdOut(CharacterDevice* dev) {
        this->stdOutput = dev;
    }


    /*****************************************************************************
     * Method: getOwnedResourceById(ResourceIdT id)
     *
     * @description
     *   Get the resource with id 'id' owned by this resource.
     *   May return null if not owned or not existent.
     *******************************************************************************/
    Resource* getOwnedResourceByFileDescriptor(ResourceIdT id);

    /*****************************************************************************
     * Method: getOwnedResource(ResourceIdT id)
     *
     * @description
     *   Returns the File Descriptor if this task owns the resource. -1 otherwise
     *******************************************************************************/
    int getOwnedResourceFileDescriptor(Resource* res);


    /*****************************************************************************
     * Method: getTaskTable()
     *
     * @description
     *   Returns the tasktable of this task.
     *******************************************************************************/
    inline taskTable* getTaskTable() const {
        return (tasktable);
    }

    /*****************************************************************************
     * Method: getWorkingDirectory()
     *
     * @description
     *   Returns the current working directory of this task
     *******************************************************************************/
    inline Directory* getWorkingDirectory() {
        return (this->workingDirectory);
    }

    /*****************************************************************************
     * Method: getWorkingDirectory()
     *
     * @description
     *   Returns the current working directory path of this task
     *******************************************************************************/
    inline char* getWorkingDirectoryPath() {
        return (this->workingDirectoryPath);
    }

    /*****************************************************************************
     * Method: setWorkingDirectory(char* newDir)
     *
     * @description
     *   Sets the current working directory of this task
     *******************************************************************************/
    ErrorT setWorkingDirectory(const char* newDir);

    /*****************************************************************************
     * Method: getSysFsDirectory()
     *
     * @description
     *  Returns the sysFs directory for this task containing
     *  it exported variables
     *******************************************************************************/
    inline Directory*      getSysFsDirectory() const {
        return (sysFsDir);
    }

    /*****************************************************************************
     * Method: getStdOutputDevice()
     *
     * @description
     *  Returns the stdOutput of this task.
     *******************************************************************************/
    inline CharacterDevice* getStdOutputDevice() const {
        return (stdOutput);
    }

    /*****************************************************************************
     * Method: stop()
     *
     * @description
     *  Stops the execution of this task until resumed or destroyed.
     *******************************************************************************/
    void stop();


    /*****************************************************************************
     * Method: resume()
     *
     * @description
     *  Resumes the execution of this task if stopped.
     *******************************************************************************/
    void resume();


    /*****************************************************************************
     * Method: getIdOfNextCreatedTask()
     *
     * @description
     *  Returns the ID of the Task, who will be created next
     *******************************************************************************/
    static TaskIdT getIdOfNextCreatedTask();


    /*****************************************************************************
     * Method: getThreadDB()
     *
     * @description
     *   Returns the Database of all Threads of this task
     * @returns
     *  LinkedList  The Database of all Threads of this task
     *******************************************************************************/
    inline LinkedList* getThreadDB() {
        return (&threadDb);
    }
};

#endif /* _TASK_HH */

