/*
 * Trace.hh
 *
 *  Created on: 08.06.2009
 *     Copyright & Author: dbaldin
 */

#ifndef TRACE_HH_
#define TRACE_HH_

#include "SCLConfig.hh"
#include "inc/types.hh"
#include "hal/CallableObject.hh"

#ifdef HAS_Kernel_TracerCfd

#define USE_TRACE               1

#define EVENT_THREAD_START      0x1
#define EVENT_THREAD_STOP       0x2
#define EVENT_THREAD_EXIT       0x3
#define EVENT_THREAD_REGISTER   0x4

#define EVENT_SYSCALL           0x5
#define EVENT_THREAD_RETURN     0x6
#define EVENT_THREAD_STACK      0x7        /* traces the stack address of a thread at a given point */

#define EVENT_IRQ_ENTRY         0x8
#define EVENT_IRQ_EXIT          0x9

#define EVENT_MEM_ALLOC         0x10
#define EVENT_MEM_FREE          0x11

#define EVENT_MUTEX_ACQUIRE     0x12
#define EVENT_MUTEX_RELEASE     0x13

#define EVENT_CHANGE_PRIORITY   0x14

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

// Total size: 24 bytes
typedef struct {
    TimeT   timestamp;    // the cpu clock ticks
    unint4  id;           // id containing event type | task id | source id
    unint4  cnt;          // counter for event loss detection
    unint4  arg;          // argument
    unint4  arg2;         // argument2
}__attribute__((packed, aligned(4))) Trace_Entry;


#define TRACE_THREAD_START(taskid, threadid) theOS->getTracer()->trace_addEntry(EVENT_THREAD_START, taskid, threadid)

#define TRACE_THREAD_STOP(taskid, threadid) theOS->getTracer()->trace_addEntry(EVENT_THREAD_STOP, taskid, threadid)

#define TRACE_THREAD_EXIT(taskid, threadid) theOS->getTracer()->trace_addEntry(EVENT_THREAD_EXIT, taskid, threadid)

#define TRACE_THREAD_REGISTER(taskid, threadid) theOS->getTracer()->trace_addEntry(EVENT_THREAD_REGISTER, taskid, threadid)

#define TRACE_IRQ_ENTRY(sourceid) theOS->getTracer()->trace_addEntry(EVENT_IRQ_ENTRY, 0, sourceid)

#define TRACE_IRQ_EXIT(sourceid) theOS->getTracer()->trace_addEntry(EVENT_IRQ_EXIT, 0, sourceid)

#define TRACE_MEMALLOC(address, size) theOS->getTracer()->trace_memAlloc(address, size)

#define TRACE_MEMFREE(address) theOS->getTracer()->trace_memFree(address)

#define TRACE_ADD_SOURCE(taskid, sourceid, name) theOS->getTracer()->addSource(taskid, sourceid, (const char*) name)

#define TRACE_REMOVE_SOURCE(taskid, sourceid) theOS->getTracer()->removeSource(taskid, sourceid)

#define TRACE_MUTEX_ACQUIRE(taskid, sourceid, name) theOS->getTracer()->trace_addEntryStr(EVENT_MUTEX_ACQUIRE, taskid, sourceid, name)
#define TRACE_MUTEX_RELEASE(taskid, sourceid, name) theOS->getTracer()->trace_addEntryStr(EVENT_MUTEX_RELEASE, taskid, sourceid, name)

#define TRACE_CHANGE_PRIORITY(taskid, sourceid, priority) theOS->getTracer()->trace_addEntry(EVENT_CHANGE_PRIORITY, taskid, sourceid, priority)

#define TRACE_SYSCALL_ENTER(taskid, sourceid, syscallId) theOS->getTracer()->trace_addEntry(EVENT_SYSCALL, taskid, sourceid, syscallId)

#else

#define TRACE_THREAD_START(taskid, threadid)
#define TRACE_THREAD_STOP(taskid, threadid)
#define TRACE_THREAD_EXIT(taskid, threadid)
#define TRACE_THREAD_REGISTER(taskid, threadid)
#define TRACE_IRQ_ENTRY(sourceid)
#define TRACE_IRQ_EXIT(sourceid)
#define TRACE_MEMALLOC(address, size)
#define TRACE_MEMFREE(address)
#define TRACE_ADD_SOURCE(taskid, sourceid, name)
#define TRACE_REMOVE_SOURCE(taskid, sourceid)
#define TRACE_MUTEX_ACQUIRE(taskid, sourceid, name)
#define TRACE_MUTEX_RELEASE(taskid, sourceid, name)
#define TRACE_CHANGE_PRIORITY(taskid, sourceid, priority)
#define TRACE_SYSCALL_ENTER(taskid, sourceid, syscallId)

#endif

class Trace: CallableObject {
private:
    /*****************************************************************************
     * Method: initEntry(unint2 number)
     *
     * @description
     *******************************************************************************/
    void initEntry(unint2 number);

public:
    Trace();

    /*****************************************************************************
     * Method: init()
     *
     * @description
     *******************************************************************************/
    void init();

    /*****************************************************************************
     * Method: addSource(unint1 taskid, unint1 sourceid, const char* name)
     *
     * @description
     *******************************************************************************/
    ErrorT addSource(unint1 taskid, unint2 sourceid, const char* name);

    /*****************************************************************************
     * Method: removeSource(unint1 taskid, unint1 sourceid)
     *
     * @description
     *******************************************************************************/
    ErrorT removeSource(unint1 taskid, unint2 sourceid);

    /*****************************************************************************
     * Method: trace_memAlloc(unint4 address, unint4 size)
     *
     * @description
     *******************************************************************************/
    void trace_memAlloc(unint4 address, unint4 size);

    /*****************************************************************************
     * Method: trace_memFree(unint4 address)
     *
     * @description
     *******************************************************************************/
    void trace_memFree(unint4 address);

    /*****************************************************************************
     * Method: trace_addEntry(unint1 type, unint1 taskid, unint1 sourceid)
     *
     * @description
     *******************************************************************************/
    void trace_addEntry(unint1 type, unint1 taskid, unint2 sourceid, unint4 argument = 0);

    /*****************************************************************************
     * Method: trace_addEntryStr(unint1 type, unint1 taskid, unint2 sourceid, const char* argument)
     *
     * @description
     *******************************************************************************/
    void trace_addEntryStr(unint1 type, unint1 taskid, unint2 sourceid, const char* argument);
    /*****************************************************************************
     * Method: callbackFunc(void* param)
     *
     * @description
     *******************************************************************************/
    void callbackFunc(void* param);
};

#endif /* TRACE_HH_ */
