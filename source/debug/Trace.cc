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

    char* trace= (char*) DEBUG_TRACE_LOCATION;

    // zero area!
    int i = 0;
    for (i = 0; i < DEBUG_TRACE_SIZE; i ++)
    {
        *(trace + i) = 0;
    }
}


void Trace::addExecutionTrace(unint1 threadid,unint8 time)
{
    *((unint8*) (DEBUG_TRACE_LOCATION)) = time;

    *((unint4*) (DEBUG_TRACE_LOCATION + 8)) = threadid;

    unint4 memusage = theOS->getMemManager()->getUsedMemSize();

    *((unint4*) (DEBUG_TRACE_LOCATION + 12)) = memusage;



    // for every task store some information
    int pos = 24;
    LinkedListDatabase* taskDB = theOS->getTaskDatabase();
    LinkedListDatabaseItem* lldi = taskDB->getHead();
    *((unint4*) (DEBUG_TRACE_LOCATION + 20)) = taskDB->getSize();

    while (lldi != 0)
    {
        Task* task = (Task*) lldi->getData();
        LinkedListDatabase* threadDB = task->getThreadDB();

        *((unint4*) (DEBUG_TRACE_LOCATION + pos)) = task->getMemManager()->getUsedMemSize();
        pos += 4;

        // store amount of threads
        *((unint4*) (DEBUG_TRACE_LOCATION + pos)) = threadDB->getSize();
        pos += 4;

        // for every thread
        LinkedListDatabaseItem* lldi2 = threadDB->getHead();

        while (lldi2 != 0)
        {
            Thread* thread = (Thread*) lldi2->getData();
            *((unint4*) (DEBUG_TRACE_LOCATION + pos)) = thread->getId();
            pos += 4;

            unint8 rel_deadline = 0;
            unint8 period = 0;

            #ifdef REALTIME
            RealTimeThread* t = (RealTimeThread*) thread;
            rel_deadline = t->relativeDeadline;
            period = t->period;
            #endif

            *((unint8*) (DEBUG_TRACE_LOCATION + pos)) = rel_deadline;
            pos += 8;
            *((unint8*) (DEBUG_TRACE_LOCATION + pos)) = period;
            pos += 8;

            lldi2 = lldi2->getSucc();
        }


        lldi = lldi->getSucc();
    }



    *((unint4*) (DEBUG_TRACE_LOCATION + 16)) = pos - 24;



}
