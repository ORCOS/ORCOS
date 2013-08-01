/*
 * TaskErrorHandler.hh
 *
 *  Created on: 30.07.2013
 *      Author: dbaldin
 */

#ifndef TASKERRORHANDLER_HH_
#define TASKERRORHANDLER_HH_

class TaskErrorHandler {
public:
	TaskErrorHandler();

	virtual ~TaskErrorHandler();

	/*!
	 * Handles fatal error that occurred during execution of the current
	 * running task.
	 */
	void handleError();
};
#endif /* TASKERRORHANDLER_HH_ */

