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

#include "SingleThreadDispatcher.hh"

#include <assemblerFunctions.hh>
#include "kernel/Kernel.hh"
#include <sprintf.hh>

extern Kernel* theOS;
extern Board_TimerCfdCl* theTimer;

/*****************************************************************************
 * Method: task_main()
 *
 * @description
 *  The entry symbol of the task to be executed. The task must thus be known
 *  to the kernel at link time.
 *******************************************************************************/
extern "C" int task_main();

/*****************************************************************************
 * Method: startThread(Thread* thread)
 *
 * @description
 *  The startThread method.
 *******************************************************************************/
extern void startThread(Thread* thread);

TimeT lastCycleStamp = 0;
Kernel_ThreadCfdCl* pCurrentRunningThread = 0;
Task* pCurrentRunningTask = 0;

taskTable singleTask;

SingleThreadDispatcher::SingleThreadDispatcher() {
    singleTask.task_entry_addr = (long) &task_main;
    pCurrentRunningTask   = 0;
    pCurrentRunningThread = 0;
    pCurrentRunningTask   = new Task(theOS->getMemoryManager(), &singleTask);
    pCurrentRunningThread = (Kernel_ThreadCfdCl*) pCurrentRunningTask->getThreadDB()->getHead()->getData();
}

SingleThreadDispatcher::~SingleThreadDispatcher() {
}

/*****************************************************************************
 * Method: SingleThreadDispatcher::dispatch()
 *
 * @description
 *  Starts the single thread.
 *******************************************************************************/
void SingleThreadDispatcher::dispatch() {
    LOG(SCHEDULER, ERROR, "starting Thread");
    /* just start the single thread we have if we get here */
    startThread(pCurrentRunningThread);
}

