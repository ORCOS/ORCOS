/*
 * Trace.hh
 *
 *  Created on: 08.06.2009
 *      Author: dbaldin
 */

#ifndef TRACE_HH_
#define TRACE_HH_

#include "SCLConfig.hh"
#include "inc/types.hh"


typedef struct {
	unint8 timestamp;
	unint2 type;
	unint1 threadid;
	unint1 taskid;
} Trace_Entry;


class Trace {
private:


public:
    Trace();

    void addExecutionTrace(unint1 threadid,unint8 time);
};

#endif /* TRACE_HH_ */
