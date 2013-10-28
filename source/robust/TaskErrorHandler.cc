/*
 * TaskErrorHandler.cc
 *
 *  Created on: 30.07.2013
 *      Author: dbaldin
 */

#include "TaskErrorHandler.hh"
#include "kernel/Kernel.hh"

extern Kernel* 	theOS;
extern unint8 	lastCycleStamp;
extern Task* 	pCurrentRunningTask;
extern Thread* 	pCurrentRunningThread;
extern LinkedListDatabaseItem* pRunningThreadDbItem;

TaskErrorHandler::TaskErrorHandler() {

}

TaskErrorHandler::~TaskErrorHandler() {

}

void TaskErrorHandler::handleError() {
	pRunningThreadDbItem = 0;
	Task* t = pCurrentRunningTask;
	//pCurrentRunningTask = 0;
	pCurrentRunningThread = 0;

	// no other policy then removing the task
	theOS->getTaskManager()->removeTask(t);

	LOG(KERNEL,INFO,(KERNEL,INFO,"TaskErrorHandler::handleError: dispatching"));
	theOS->getCPUDispatcher()->dispatch( theOS->getClock()->getTimeSinceStartup() - lastCycleStamp );

}
