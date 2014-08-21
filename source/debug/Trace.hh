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
#include "hal/CallableObject.hh"

#ifndef USE_TRACE
#define USE_TRACE 0
#endif

#define EVENT_THREAD_START  0x1
#define EVENT_THREAD_STOP   0x2
#define EVENT_THREAD_EXIT   0x3
#define EVENT_THREAD_REGISTER 0x4

#define EVENT_SYSCALL	    0x5
#define EVENT_THREAD_RETURN 0x6
#define EVENT_THREAD_STACK  0x7		/* traces the stack address of a thread at a given point */

#define EVENT_IRQ_ENTRY	    0x8
#define EVENT_IRQ_EXIT	    0x9

#define EVENT_MEM_ALLOC    0x10
#define EVENT_MEM_FREE	   0x11

typedef struct {
    unint4 stack_pointer;
    unint4 address;
}__attribute__((packed)) Trace_StackInfo;

typedef struct {
    unint4 address;
    unint4 size;
}__attribute__((packed)) Trace_MemInfo;

typedef union {
    Trace_StackInfo thread_info;
    Trace_MemInfo mem_info;
}__attribute__((packed)) TraceArgs;

// Total size: 20 bytes
typedef struct {
    TimeT timestamp;	// the cpu clock ticks
    /*	unint1 type;		// the type of event
     unint1 taskid;  	// the task this event belongs to
     unint1 sourceid;	// the source this event belongs to inside the task (might be a thread id)
     unint1 param1;		// a free parameter*/
    unint4 id;
    unint4 arg1;
    unint4 arg2;
    //TraceArgs arg;		// additional parameters for memory and thread stack informations
}__attribute__((packed, aligned(4))) Trace_Entry;

#if USE_TRACE

#define TRACE_THREAD_START(taskid, threadid) theOS->getTrace()->trace_addEntry(EVENT_THREAD_START,taskid,threadid)

#define TRACE_THREAD_STOP(taskid, threadid) theOS->getTrace()->trace_addEntry(EVENT_THREAD_STOP,taskid,threadid)

#define TRACE_THREAD_EXIT(taskid, threadid) theOS->getTrace()->trace_addEntry(EVENT_THREAD_EXIT,taskid,threadid)

#define TRACE_THREAD_REGISTER(taskid, threadid) theOS->getTrace()->trace_addEntry(EVENT_THREAD_REGISTER,taskid,threadid)

#define TRACE_IRQ_ENTRY(sourceid) theOS->getTrace()->trace_addEntry(EVENT_IRQ_ENTRY,0,sourceid)

#define TRACE_IRQ_EXIT(sourceid) theOS->getTrace()->trace_addEntry(EVENT_IRQ_EXIT,0,sourceid)

#define TRACE_MEMALLOC(address,size) theOS->getTrace()->trace_memAlloc(address,size)

#define TRACE_MEMFREE(address) theOS->getTrace()->trace_memFree(address)

#define TRACE_ADD_SOURCE(taskid, sourceid, name) theOS->getTrace()->addSource(taskid, sourceid, (const char*) name)

#define TRACE_REMOVE_SOURCE(taskid, sourceid) theOS->getTrace()->removeSource(taskid, sourceid)

#else

#define TRACE_THREAD_START(taskid, threadid)

#define TRACE_THREAD_STOP(taskid, threadid)

#define TRACE_THREAD_EXIT(taskid, threadid)

#define TRACE_THREAD_REGISTER(taskid, threadid)

#define TRACE_IRQ_ENTRY(sourceid)

#define TRACE_IRQ_EXIT(sourceid)

#define TRACE_MEMALLOC(address,size)

#define TRACE_MEMFREE(address)

#define TRACE_ADD_SOURCE(taskid, sourceid, name)

#define TRACE_REMOVE_SOURCE(taskid, sourceid)
#endif

class Trace: CallableObject {
private:

    /*
     * Initializes the trace entry with the current thread and task id and the current timestamp
     */
    void initEntry(unint2 number);

public:
    Trace();

    void init();

    ErrorT addSource(unint1 taskid, unint1 sourceid, const char* name);
    ErrorT removeSource(unint1 taskid, unint1 sourceid);
    void trace_memAlloc(unint4 address, unint4 size);
    void trace_memFree(unint4 address);
    void trace_addEntry(unint1 type, unint1 taskid, unint1 sourceid);

    void callbackFunc(void* param);
};

#endif /* TRACE_HH_ */
