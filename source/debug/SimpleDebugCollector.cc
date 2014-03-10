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
#include <db/LinkedListDatabase.hh>
#include <process/Task.hh>

extern Kernel* theOS;

// reading the device is not thread safe. fopen() will take care of it
#define MAX_LENGTH_OUTPUT_MSG 700
static char outputMsg[ MAX_LENGTH_OUTPUT_MSG ];

SimpleDebugCollector::SimpleDebugCollector() :
    CharacterDeviceDriver( cStreamDevice, false, "info" ) {

}

ErrorT SimpleDebugCollector::readBytes( char *bytes, unint4 &length ) {

    static unint lastReadOffset = 0;
    const char* newline = "\r";
    char tmpBuf[ 100 ];

    if ( !lastReadOffset ) {
        // our output was read completely, so we construct a new one
        // build the result msg
        for ( unint i = 0; i < sizeof( outputMsg ); i++ ) {
            // zero out Msg
            outputMsg[ i ] = 0x00;
        }

        LinkedListDatabase* llt = theOS->getTaskDatabase();
        unint numberoftasks = llt->getSize();

        // output number of running tasks
        sprintf(tmpBuf,"#Tasks: %d\rTaskID\tUsedMem\tOverhead\tFree\tThreads\r", numberoftasks );
        strcat(outputMsg, tmpBuf);

        // output list of tasks
        for (LinkedListDatabaseItem* lldi = llt->getHead(); lldi != 0; lldi = lldi->getSucc()) {
            Task* task = (Task*) lldi->getData();

            size_t freemem = 0;
            size_t overhead = 0;
            size_t usedmem       = task->getMemManager()->getUsedMemSize(overhead,freemem);
            sprintf(tmpBuf, "  %02d\t%7d\t", task->getId(),usedmem);
            strcat(outputMsg, tmpBuf);

            sprintf(tmpBuf, "%7d\t%d\t",overhead,freemem);
            strcat(outputMsg, tmpBuf);

            for (LinkedListDatabaseItem* t = task->getThreadDB()->getHead(); t != 0; t = t->getSucc()) {
                Thread* thread = (Thread*) t->getData();
                sprintf(tmpBuf, "%d %c%c%c%c%c%c   ",
                        thread->getId(),
                        thread->isNew() ? 'N' : '-',
                        thread->isReady() ? 'R' : '-',
                        thread->isBlocked() ? 'B': '-',
                        thread->isStopped() ? 'S': '-',
                        thread->hasTerminated() ? 'T': '-',
                        thread->isSleeping() ? 'A': '-'
                );
                strcat(outputMsg, tmpBuf);
            }
            strcat(outputMsg,newline);
        }

        Board_ClockCfdCl* clock = theOS->getClock();
        volatile TimeT passedTime = clock->getTimeSinceStartup();

        TimeT seconds_passed = (TimeT) (passedTime / CLOCK_RATE);
        TimeT milliseconds = ((TimeT) (passedTime - (seconds_passed * CLOCK_RATE))) / (CLOCK_RATE / 1000);

        char time[ 16 ];
        itoa( passedTime, time, 10 );
        sprintf( tmpBuf, "Time since startup: %6d,%03d s [Cyles: %s]",(unint4) seconds_passed,(unint4) milliseconds,time);
        strcat( outputMsg, tmpBuf );

        lastReadOffset = 0;
    }

    unint4 i;
    for ( i = 0; i < length; i++ ) {
        *( bytes + i ) = *( outputMsg + lastReadOffset++ );
        if ( ( *( bytes + i ) == 0x00 ) || ( lastReadOffset + 1 >= sizeof( outputMsg ) ) ) {
            *( bytes + i ) = 0x04; // ASCII for EOT or EOF
            lastReadOffset = 0;
            break; // quit this loop
        }
    }

    length = i;

    return (cOk);
}