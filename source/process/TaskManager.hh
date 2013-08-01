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
    LinkedListDatabase* taskDatabase;

public:
    TaskManager() {
    	// create the task Database
    	this->taskDatabase 	= new LinkedListDatabase( );
    }
    ;
    ~TaskManager() {
    }
    ;

    // Per-Initialization reoutine which registers the
    // memory pages of tasks loaded at boot time
    void registerMemPages();

    // Initializes the task manager and creates the initially loaded tasks
    void initialize();

   //! Returns the database of all registered tasks
   LinkedListDatabase* getTaskDatabase() {
	   return this->taskDatabase;
   }
   ;

   /*!
    * Tests a given task by its task control block on sanity
    * Checks include CB check for valid values.
    * CRC check if CRC header support is available.
    * Signature based Authentication (tbd).
    */
   ErrorT checkValidTask(taskTable* taskCB);

   /*!
    * Returns the task by its id or null
    */
   Task* getTask( int taskId );

   /*!
    * Tries to stop and remove a task by its ID
    */
   ErrorT removeTask(Task* task);

   /*!
    * Tries to load a given file as an executable task.
    * Returns cOk on success, an error otherwise.
    * The Out paramter tid is used to return the created task id on success.
    */
   ErrorT loadTaskFromFile(File *file, TaskIdT &tid);

};

#endif /*TASKMANAGER_HH_*/
