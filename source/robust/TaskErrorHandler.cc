/*
 * TaskErrorHandler.cc
 *
 *  Created on: 30.07.2013
 *      Author: dbaldin
 */

#include "TaskErrorHandler.hh"
#include "kernel/Kernel.hh"

extern Kernel* 	theOS;
extern TimeT 	lastCycleStamp;
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
	//pCurrentRunningTask = 0;  /* < keep as is */
	pCurrentRunningThread = 0;

	/* currently no other policy then removing the task */
	theOS->getTaskManager()->removeTask(t);

    /* Continue executing anything else */
	LOG(KERNEL,DEBUG,(KERNEL,DEBUG,"TaskErrorHandler::handleError: dispatching"));
	theOS->getCPUDispatcher()->dispatch( (unint4) ( theOS->getClock()->getTimeSinceStartup() - lastCycleStamp) );

}
