/*
 * TaskErrorHandler.cc
 *
 *  Created on: 30.07.2013
 *      Author: dbaldin
 */

#include "TaskErrorHandler.hh"
#include "kernel/Kernel.hh"

extern Kernel* 	theOS;
extern Task* 	pCurrentRunningTask;
extern Thread* 	pCurrentRunningThread;
extern LinkedListItem* pRunningThreadDbItem;

TaskErrorHandler::TaskErrorHandler() {

}

TaskErrorHandler::~TaskErrorHandler() {

}

void TaskErrorHandler::handleError(ErrorType eErrorType) {

	/*
	 * TODO: further analyze the error.
	 *
	 * - If the error occurred during a syscall we might even return with an error code.
	 *   for authenticated tasks (signing feature must be provided)
	 * - For security reasons we might remove the task (default)
	 * - A task  might provide an error policy
	 * */

	Task* t = pCurrentRunningTask;
	//pCurrentRunningTask = 0;  /* < keep as is */

	if (t != 0) {
		if (t->getId() != 0) {
			pCurrentRunningThread = 0;
			/* currently no other policy then removing the task */
			theOS->getTaskManager()->removeTask(t);
		}else {
#if USE_WORKERTASK
			/* reset workerthread */
			WorkerThread* wt = (WorkerThread*) pCurrentRunningThread;
			wt->setJob(None,0);
			wt->block();
#endif
		}

		pRunningThreadDbItem = 0;
		pCurrentRunningThread = 0;

		/* Continue executing anything else */
		LOG(KERNEL,DEBUG,"TaskErrorHandler::handleError: Dispatching");
		theOS->getDispatcher()->dispatch(  );
	} else {
		LOG(KERNEL,ERROR,"FATAL error occurred during BOOT. Can not recover from this.");
		while (1) {};
	}

}
