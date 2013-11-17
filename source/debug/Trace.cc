/*
 * Trace.cc
 *
 *  Created on: 08.06.2009
 *      Author: dbaldin
 */

#include "Trace.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;
extern Kernel_ThreadCfdCl*    pCurrentRunningThread;
extern Task*           		  pCurrentRunningTask;

#ifndef DEBUG_TRACE_LOCATION
#define DEBUG_TRACE_LOCATION 0
#endif

#ifndef DEBUG_TRACE_SIZE
#define DEBUG_TRACE_SIZE 0
#endif

/**
 * 16 KB of memory for trace
 */
#define NUM_TRACE_ENTRIES 1024

static Trace_Entry trace[NUM_TRACE_ENTRIES];

static unint2 current_entry;

Trace::Trace() {
	current_entry = 0;

}

void Trace::initEntry(unint2 number) {
	if (pCurrentRunningTask != 0)
		trace[number].taskid = pCurrentRunningTask->getId();
	else
		trace[number].taskid = 0;

	if (pCurrentRunningThread != 0)
		trace[number].threadid = pCurrentRunningThread->getId();
	else
		trace[number].threadid = 0;

	trace[number].timestamp = (unint4) theOS->getClock()->getTimeSinceStartup();

}

void Trace::trace_memAlloc(unint4 address, unint4 size)
{
	initEntry(current_entry);
	trace[current_entry].type = TRACE_MEM_ALLOC;
	trace[current_entry].arg.mem_info.address = address;
	trace[current_entry].arg.mem_info.size 	  = size;
	current_entry = (current_entry + 1) & (NUM_TRACE_ENTRIES-1);
}


void Trace::trace_memFree(unint4 address)
{
	initEntry(current_entry);
	trace[current_entry].type = TRACE_MEM_FREE;
	trace[current_entry].arg.mem_info.address = address;
	trace[current_entry].arg.mem_info.size 	  = 0;
	current_entry = (current_entry + 1) & (NUM_TRACE_ENTRIES-1);
}

void Trace::trace_threadInfo(unint2 type, unint4 sp, unint4 address)
{
	initEntry(current_entry);
	trace[current_entry].type = type;
	trace[current_entry].arg.thread_info.sp = sp;
	trace[current_entry].arg.thread_info.address = address;
	current_entry = (current_entry + 1) & (NUM_TRACE_ENTRIES-1);
}
