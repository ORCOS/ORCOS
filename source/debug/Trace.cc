/*
 * Trace.cc
 *
 *  Created on: 08.06.2009
 *      Author: dbaldin
 */

#include "Trace.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

#ifndef DEBUG_TRACE_LOCATION
#define DEBUG_TRACE_LOCATION 0
#endif

#ifndef DEBUG_TRACE_SIZE
#define DEBUG_TRACE_SIZE 0
#endif

Trace::Trace() {


}


void Trace::addExecutionTrace(unint1 threadid,unint8 time)
{


}
