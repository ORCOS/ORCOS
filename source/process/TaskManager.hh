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

#ifndef TASKMANAGER_HH_
#define TASKMANAGER_HH_

#include <process/Task.hh>
#include "filesystem/File.hh"

/*!
 * \brief The Taskmanager wraps some functions around task-handling.
 *
 */
class TaskManager {
private:
    /*!
     * \brief The database storing all tasks
     */
    LinkedList* taskDatabase;

public:
    /*****************************************************************************
     * Method: TaskManager()
     *---------------------------------------------------------------------------*/
    TaskManager() {
        /* create the task Database */
        this->taskDatabase = new LinkedList();
    }

    ~TaskManager() {
    }

    /*****************************************************************************
     * Method: registerMemPages()
     *
     * @description
     *  Registers all memory pages of the initial tasks at the RamManager as used.
     *---------------------------------------------------------------------------*/
    void registerMemPages();

    /*****************************************************************************
     * Method: initialize()
     *
     * @description
     *  Initializes the task manager creating all initial task
     *  provided inside the kernel image.
     *
     *---------------------------------------------------------------------------*/
    void initialize();

    /*****************************************************************************
     * Method: getTaskDatabase()
     *
     * @description
     *  Returns the list of tasks currently active inside the system
     *
     * @returns
     *  LinkedList*       The Task List
     *---------------------------------------------------------------------------*/
    inline LinkedList* getTaskDatabase() const {
        return (this->taskDatabase);
    }

    /*****************************************************************************
     * Method: checkValidTask(taskTable* taskCB)
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
    ErrorT  checkValidTask(taskTable* taskCB);

    /*****************************************************************************
     * Method: getTask(int taskId)
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
    Task*   getTask(int taskId);

    /*****************************************************************************
     * Method: getThread(int threadId)
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
    Thread* getThread(int threadId);

    /*****************************************************************************
     * Method: removeTask(Task* task)
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
    ErrorT  removeTask(Task* task);

    /*****************************************************************************
     * Method: terminateThread(Kernel_ThreadCfdCl* pThread)
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
    ErrorT  terminateThread(Kernel_ThreadCfdCl* thread);

    /*****************************************************************************
     * Method: loadTaskFromFile(File* file,
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
    ErrorT  loadTaskFromFile(File *file, TaskIdT &tid, char* arguments = 0, unint2 arg_length = 0);
};

#endif /*TASKMANAGER_HH_*/
