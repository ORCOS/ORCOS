/*
 * Trace.cc
 *
 *  Created on: 08.06.2009
 *      Author: dbaldin
 */

#if USE_TRACE

#include "Trace.hh"
#include "kernel/Kernel.hh"
#include "inc/memtools.hh"
#include "comm/Socket.hh"
#include "inet.h"

extern Kernel* theOS;
extern Kernel_ThreadCfdCl* pCurrentRunningThread;
extern Task* pCurrentRunningTask;

#ifndef DEBUG_TRACE_LOCATION
#define DEBUG_TRACE_LOCATION 0
#endif

#ifndef DEBUG_TRACE_SIZE
#define DEBUG_TRACE_SIZE 0
#endif

/**
 * 40 KB of memory for trace
 */
#define NUM_TRACE_ENTRIES 1024

typedef struct
{
    unint4 TraceID;
    unint4 SystemId;
    Trace_Entry buffer[NUM_TRACE_ENTRIES];
}__attribute__((packed, aligned(4))) Trace_Buffer;

static Trace_Buffer trace_buffer1;
static Trace_Buffer trace_buffer2;

static unint2 current_entry;
static unint4 num_entries;
static Trace_Buffer* current_buffer;

typedef struct
{
    unint1 taskid;
    unint1 sourceid;
    char name[10];
}__attribute__((packed)) Trace_Source;

typedef struct
{
    unint4 PlatformID;			 // ARM, PPC e.t.c
    unint4 SystemID;// to be assigned by the host
    unint4 clockTicksPerSecond;// the number of clock ticks per second
}__attribute__((packed)) PlatformInfo;

static PlatformInfo platformInfo;

static Trace_Source sources[50];

static Socket* commandSocket;
static Socket* streamSocket;

#define cTCP 						0x6
#define cIPV4 						0x800
#define cUDP      					17

#define cGETPLATFORMINFO			0x1
#define cSETDESTINATION				0x2
#define cGETSOURCES					0x3
#define cSTARTDATASTREAM			0x4
#define cSTOPDATASTREAM				0x5

#define COMMAND_THREAD				0
#define STREAM_THREAD				1

static char rcvbuf[100];

static bool streaming = false;
WorkerThread* streamThread;

Trace::Trace()
{
    current_entry = 0;
    current_buffer = &trace_buffer1;
    num_entries = 0;
    streamThread = 0;
    memset(sources,0,sizeof(sources));

    trace_buffer1.TraceID = 0x23551a3f;
    trace_buffer2.TraceID = 0x23551a3f;

    platformInfo.clockTicksPerSecond = CLOCK_RATE;
    platformInfo.SystemID = 0;
    platformInfo.PlatformID = PLATFORM;

    streaming = false;
}

sockaddr streamDest;

void Trace::init()
{

    sockaddr addr;
    addr.port_data = 46;
    addr.sa_data = 0;

    // setup the two sockets
    commandSocket = new Socket(cIPV4,SOCK_STREAM,cTCP);
    commandSocket->bind(&addr);

    streamSocket = new Socket(cIPV4,SOCK_DGRAM,cUDP);
    addr.port_data = 0;
    addr.sa_data = 0;
    streamSocket->bind(&addr);

    TimedFunctionCall* jobparam = new TimedFunctionCall;
    jobparam->objectptr = this; /* call this object */
    jobparam->parameterptr = COMMAND_THREAD; /* store the index of the request */
    jobparam->time = theOS->getClock()->getTimeSinceStartup() + 200 ms; /* call the first time in 200 ms */

    // use one workerthread "permanently" for the command socket handler
    theOS->getWorkerTask()->addJob(TimedFunctionCallJob, 0,jobparam, 250000);

}

void Trace::callbackFunc( void* param )
{

    if (param == COMMAND_THREAD)
    {
        // wait for incoming tcp connection
        while (true)
        {
            int newSock = commandSocket->listen(pCurrentRunningThread);

            Socket* sessionSock = (Socket*) pCurrentRunningThread->getOwner()->getOwnedResourceById(newSock);
            printf("Incoming Trace control connection.\r");
            int len = sessionSock->recvfrom(pCurrentRunningThread,rcvbuf,100,MSG_WAIT,0);

            while (len >= 0)
            {
                // handle msg
                switch (rcvbuf[0])
                {
                    case cGETPLATFORMINFO:
                    {
                        printf("Sending platform info.\r");
                        // send platform info
                        sessionSock->sendto(&platformInfo,sizeof(platformInfo),(sockaddr*) 0);
                        break;
                    }
                    case cSETDESTINATION :
                    {
                        // port and ip are big endian encoded
                        unint2 port = (rcvbuf[1] << 8) | rcvbuf[2];
                        unint4 ipaddress = (rcvbuf[3] << 24) | (rcvbuf[4] << 16) | (rcvbuf[5] << 8) | (rcvbuf[6]);

                        printf("Setting destination : port: %u ip: %x\r",port,ipaddress);
                        streamDest.port_data = htons(port);
                        streamDest.sa_data = htonl(ipaddress);
                        break;
                    }
                    case cGETSOURCES:
                    {
                        printf("Sending sources.\r");
                        sessionSock->sendto(&sources,sizeof(sources),(sockaddr*) 0);
                        break;
                    }
                    case cSTARTDATASTREAM :
                    {
                        if (streaming) break;

                        streaming = true;

                        // start the workerthread for the stream
                        printf("Starting  data stream.\r");

                        if (streamThread == 0)
                        {
                            PeriodicFunctionCall* jobparam = new PeriodicFunctionCall;
                            jobparam->functioncall.objectptr = this; /* call this object */
                            jobparam->functioncall.parameterptr = (void*) STREAM_THREAD;
                            jobparam->functioncall.time = theOS->getClock()->getTimeSinceStartup() + 10 ms; /* call the first time in 200 ms */
                            jobparam->period = 50 ms;

                            // use one workerthread "permanently" for the command socket handler
                            streamThread = theOS->getWorkerTask()->addJob(PeriodicFunctionCallJob, 0,jobparam, 10000);
                        }

                        break;
                    }
                    case cSTOPDATASTREAM :
                    {
                        streaming = false;
                        printf("Stopping data stream.\r");
                    }

                }

                len = sessionSock->recvfrom(pCurrentRunningThread,rcvbuf,100,MSG_WAIT,0);
            }

            printf("Trace control connection closed.\r");
        }

    }
    else
    {
        // we must be the stream thread
        if (streaming)
        {
            // initiate a buffer switch
            Trace_Buffer* tosend_buffer = current_buffer;
            unint4 numEvents = num_entries;

            if (numEvents > NUM_TRACE_ENTRIES)
            numEvents = NUM_TRACE_ENTRIES;

            if (current_buffer == &trace_buffer1)
            current_buffer = &trace_buffer2;
            else
            current_buffer = &trace_buffer1;
            num_entries = 0;
            current_entry = 0;

            // send the events in udp packets of maximum size 1400
            unint4 sendlen = 0;
            int pos = 0;
            while (sendlen < numEvents)
            {

                int eventsToSend = 60;
                if (numEvents - sendlen < 60)
                eventsToSend = numEvents - sendlen;

                int bytes = sizeof(Trace_Entry) * (eventsToSend) + 8;

                // copy header infront
                if (pos > 0)
                memcpy((void*) ( (unint4) (&tosend_buffer->buffer[sendlen]) - 8),tosend_buffer,8);

                streamSocket->sendto((void*) ( (unint4) (&tosend_buffer->buffer[sendlen]) - 8),bytes,&streamDest);
                sendlen += eventsToSend;
                pos += 1;
            }

        }
    }

}

ErrorT Trace::addSource(unint1 taskid, unint1 sourceid, const char* name)
{

    // search for a free entry. sourceid == 0 may never be used
    for (int i = 0; i < 50; i++)
    {
        if (sources[i].sourceid == 0)
        {
            sources[i].taskid = taskid;
            sources[i].sourceid = sourceid;

            if (name != 0)
            {
                int len = strlen(name);
                if (len > 9) len = 9;
                memset(sources[i].name,0,10);
                memcpy(sources[i].name,name,len);
            }

            return (cOk);
        }
    }

    return (cError);
}

ErrorT Trace::removeSource(unint1 taskid, unint1 sourceid)
{

    // search for the entry
    for (int i = 0; i < 50; i++)
    {
        if (sources[i].sourceid == sourceid && sources[i].taskid == taskid)
        {
            sources[i].taskid = 0;
            sources[i].sourceid = 0;
            return (cOk);
        }
    }

    return (cError);
}

void Trace::initEntry(unint2 number)
{
    /*if (pCurrentRunningTask != 0)
     current_buffer->buffer[number].taskid = pCurrentRunningTask->getId();
     else
     current_buffer->buffer[number].taskid = 0;

     if (pCurrentRunningThread != 0)
     current_buffer->buffer[number].sourceid = pCurrentRunningThread->getId();
     else
     current_buffer->buffer[number].sourceid = 0;

     current_buffer->buffer[number].timestamp = (unint4) theOS->getClock()->getTimeSinceStartup();*/
}

void Trace::trace_memAlloc(unint4 address, unint4 size)
{
    /*	initEntry(current_entry);
     current_buffer->buffer[current_entry].type = EVENT_MEM_ALLOC;
     current_buffer->buffer[current_entry].arg.mem_info.address = address;
     current_buffer->buffer[current_entry].arg.mem_info.size 	  = size;
     current_entry = (current_entry + 1) & (NUM_TRACE_ENTRIES-1);
     num_entries++;*/
}

void Trace::trace_memFree(unint4 address)
{
    /*initEntry(current_entry);
     current_buffer->buffer[current_entry].type = EVENT_MEM_FREE;
     current_buffer->buffer[current_entry].arg.mem_info.address = address;
     current_buffer->buffer[current_entry].arg.mem_info.size 	  = 0;
     current_entry = (current_entry + 1) & (NUM_TRACE_ENTRIES-1);*/
}

void Trace::trace_addEntry(unint1 type, unint1 taskid, unint1 sourceid)
{
    current_buffer->buffer[current_entry].timestamp = theOS->getClock()->getTimeSinceStartup();
    current_buffer->buffer[current_entry].id = ((type ) | (taskid << 8) | (sourceid << 16));
    current_entry = (current_entry + 1) & (NUM_TRACE_ENTRIES-1);
    num_entries++;
}

#endif
