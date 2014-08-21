/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2008 University of Paderborn

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "SimpleDebugCollector.hh"
#include <kernel/Kernel.hh>
#include <inc/sprintf.hh>
#include <inc/stringtools.hh>
#include <db/LinkedList.hh>
#include <process/Task.hh>
#include Kernel_Thread_hh

extern Kernel* theOS;

// reading the device is not thread safe. fopen() will take care of it
#define MAX_LENGTH_OUTPUT_MSG 4096
static char outputMsg[ MAX_LENGTH_OUTPUT_MSG];

SimpleDebugCollector::SimpleDebugCollector() :
        CharacterDevice(cStreamDevice, false, "info") {

}



ErrorT SimpleDebugCollector::readBytes(char *bytes, unint4 &length) {

    static unint lastReadOffset = 0;
    const char* newline = "\r";
    char tmpBuf[100];

    if (!lastReadOffset)
    {
        // our output was read completely, so we construct a new one
        // build the result msg
        for (unint i = 0; i < sizeof(outputMsg); i++)
        {
            // zero out Msg
            outputMsg[i] = 0x00;
        }

        LinkedList* llt = theOS->getTaskDatabase();

        // output list of tasks
        for (LinkedListItem* lldi = llt->getHead(); lldi != 0; lldi = lldi->getSucc())
        {
            Task* task      = (Task*) lldi->getData();
            size_t freemem  = 0;
            size_t overhead = 0;
            size_t usedmem  = 0;
            if (task->getId() == 0)
                usedmem  = task->getMemManager()->getUsedMemSize(overhead, freemem);

            int percent     = (usedmem  * 100) / (usedmem+freemem);
            sprintf(tmpBuf, "\r[%02d] %-20s\r  Memory    : %d / %d %d%% (OV: %d)\r", task->getId(),task->getName(), usedmem, usedmem+freemem, percent, overhead);
            strcat(outputMsg, tmpBuf);

            Board_ClockCfdCl* clock = theOS->getClock();
            TimeT passedTime        = clock->getClockCycles();

            for (LinkedListItem* t = task->getThreadDB()->getHead();  t != 0; t = t->getSucc())
            {
                Thread* thread          = (Thread*) t->getData();
                TimeT priority          = 0;
#ifdef HAS_PRIORITY
                PriorityThread* tprio   = (PriorityThread*) thread;
                priority                = tprio->effectivePriority;
#endif

                sprintf(tmpBuf, "  Thread %03d: PRIO %10u ",thread->getId(),(unint4) priority);
                strcat(outputMsg, tmpBuf);

                if (thread->isSleeping()) {
                    if (thread->signal == 0)
                        sprintf(tmpBuf, "SLEEPING %d cycles\r",thread->getSleepTime() - passedTime);
                    else
                        sprintf(tmpBuf, "WAITING ON SIGNAL %x\r",thread->signal);
                }
                if (thread->isNew()) {
                    if (thread->getSleepTime() > passedTime)
                        sprintf(tmpBuf, "TIMED in %d cycles\r",thread->getSleepTime() - passedTime );
                    else
                        sprintf(tmpBuf, "NEW\r");
                }
                if (thread->isBlocked())
                    sprintf(tmpBuf, "BLOCKED\r");
                if (thread->isReady())
                    sprintf(tmpBuf, "READY\r");
                if (thread->isStopped())
                    sprintf(tmpBuf, "STOPPED\r");
                if (thread->hasTerminated())
                    sprintf(tmpBuf, "ZOMBIE\r");

                strcat(outputMsg, tmpBuf);
            }
        }

        Board_ClockCfdCl* clock = theOS->getClock();
        TimeT passedTime        = clock->getClockCycles();

        TimeT seconds_passed    = (TimeT) (passedTime / CLOCK_RATE);
        TimeT milliseconds      = ((TimeT) (passedTime  - (seconds_passed * CLOCK_RATE))) / (CLOCK_RATE / 1000);

        char time[16];
        uitoa(passedTime, time, 10);
        sprintf(tmpBuf, "\rSeconds since startup: %6u,%03u s [Cyles: %s]", (unint4) seconds_passed, (unint4) milliseconds, time);
        strcat(outputMsg, tmpBuf);

        lastReadOffset = 0;
    }

    unint4 i;
    for (i = 0; i < length; i++)
    {
        *(bytes + i) = *(outputMsg + lastReadOffset++);
        if ((*(bytes + i) == 0x00) || (lastReadOffset + 1 >= sizeof(outputMsg)))
        {
            *(bytes + i) = 0x04;  // ASCII for EOT or EOF
            lastReadOffset = 0;
            break;  // quit this loop
        }
    }

    length = i;

    return (cOk );
}
