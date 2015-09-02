/*
 * TaskErrorHandler.cc
 *
 *  Created on: 30.07.2013
 *     Copyright & Author: dbaldin
 */

#include "TaskErrorHandler.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;
extern Task* pCurrentRunningTask;
extern Kernel_ThreadCfdCl* pCurrentRunningThread;

TaskErrorHandler::TaskErrorHandler() {
}

TaskErrorHandler::~TaskErrorHandler() {
}

/*****************************************************************************
 * Method: TaskErrorHandler::handleError(ErrorType eErrorType)
 *
 * @description
 *
 * @params
 *  eErrorType:     The type of error that occurred
 *---------------------------------------------------------------------------*/
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

    if (t != 0) {
        if (t->getId() != 0) {
            /* currently no other policy then removing the task */
            theOS->getTaskManager()->removeTask(t);
        } else {
#if USE_WORKERTASK
            /* reset workerthread */
            KernelThread* wt = static_cast<KernelThread*>(pCurrentRunningThread);
            wt->stop();
#endif
        }

        pCurrentRunningThread = 0;

        theOS->getLogger()->flush();

        /* Continue executing anything else */
        LOG(KERNEL, DEBUG, "TaskErrorHandler::handleError: Dispatching");
        theOS->getDispatcher()->dispatch();
    } else {
        LOG(KERNEL, ERROR, "FATAL error occurred during BOOT. Can not recover from this.");
        theOS->getLogger()->flush();
        while (1) {
        }
    }
}
