/*
 * TaskErrorHandler.hh
 *
 *  Created on: 30.07.2013
 *     Copyright & Author: dbaldin
 */

#ifndef TASKERRORHANDLER_HH_
#define TASKERRORHANDLER_HH_

typedef enum {
    /* Error for Data aborts (read, write) to a memory address not accessible by the current task */
    cDataAbortError = 0,
    /* Error for access to an address not mapped into the current process */
    cMappingError = 1,
    /* Error that is raised by the cpu if an instruction could not be decoded */
    cUndefinedInstruction = 2,
    /* Watchdog interrupt was raised */
    cWatchdog = 3,
} ErrorType;

class TaskErrorHandler {
public:
    TaskErrorHandler();

    ~TaskErrorHandler();

    /*****************************************************************************
     * Method: handleError(ErrorType eErrorType)
     *
     * @description
     *  Handles errors that occurred. E.g. hardware data aborts, watchdog irqs etc.
     *
     * @params
     *  eErrorType:     The type of error that occurred
     *
     * @returns
     *  int         Error Code
     *---------------------------------------------------------------------------*/
    void handleError(ErrorType eErrorType);
};
#endif /* TASKERRORHANDLER_HH_ */

