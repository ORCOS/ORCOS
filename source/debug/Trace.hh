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


#define TRACE_THREAD_START  0x1
#define TRACE_THREAD_STOP   0x2
#define TRACE_THREAD_EXIT   0x3
#define TRACE_SYSCALL	    0x4
#define TRACE_THREAD_RETURN 0x5
#define TRACE_THREAD_STACK  0x6		/* traces the stack address of a thread at a given point */

#define TRACE_IRQ		    0x8

#define TRACE_MEM_ALLOC    0x10
#define TRACE_MEM_FREE	   0x11

typedef struct {
	unint4 sp;
	unint4 address;
} __attribute__((packed)) Trace_ThreadInfo;

typedef struct {
	unint4 address;
	unint4 size;
} __attribute__((packed)) Trace_MemInfo;

typedef struct {
	unint1 syscall_number;
} __attribute__((packed)) Trace_SyscallInfo;

typedef struct {
	unint1 irq_number;
} __attribute__((packed)) Trace_IRQInfo;

typedef union {
 Trace_ThreadInfo 	thread_info;
 Trace_MemInfo 		mem_info;
 Trace_SyscallInfo 	syscall_info;
 Trace_IRQInfo		irq_info;
} __attribute__((packed)) TraceArgs;

typedef struct {
	unint4 timestamp;
	unint2 type;
	unint1 threadid;
	unint1 taskid;
	TraceArgs arg;
} __attribute__((packed)) Trace_Entry;


#define TRACE_THREADSTART(sp) theOS->getTrace()->trace_threadInfo(TRACE_THREAD_START,sp,0)
#define TRACE_THREADSTOP() theOS->getTrace()->trace_threadInfo(TRACE_THREAD_STOP,0,0)
#define TRACE_THREADEXIT() theOS->getTrace()->trace_threadInfo(TRACE_THREAD_EXIT,0,0)
#define TRACE_THREADSTACK() { unint4 sp = GETSTACKPTR(); \
						      theOS->getTrace()->trace_threadInfo(TRACE_THREAD_STACK,sp,0); }

#define TRACE_MEMALLOC(address,size) theOS->getTrace()->trace_threadInfo(TRACE_MEM_ALLOC,address,size)
#define TRACE_MEMFREE(address,size) theOS->getTrace()->trace_threadInfo(TRACE_MEM_FREE,address,0)

class Trace {
private:

	void initEntry(unint2 number);

public:
    Trace();

    void trace_memAlloc(unint4 address, unint4 size);
    void trace_memFree(unint4 address);
    void trace_threadInfo(unint2 type, unint4 sp, unint4 address);
};

#endif /* TRACE_HH_ */
