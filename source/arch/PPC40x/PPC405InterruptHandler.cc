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

#include "powerpc.h"
#include <PPC405InterruptHandler.hh>
#include <kernel/Kernel.hh>
#include <memtools.hh>
#include <sprintf.hh>

extern Kernel* theOS;
extern PPC405InterruptHandler* thePPC405InterruptHandler;

extern "C" void IRQ0x0c00(void);
extern "C" void IRQ0x1000(void);
extern "C" void IRQ0x1020(void);
extern "C" void IRQ0x1010(void);
extern "C" void IRQ0x0500(void);
extern "C" void IRQ0x0200(void);
extern "C" void IRQ0x0700(void);
extern "C" void IRQ0x1100(void);
extern "C" void IRQ0x1200(void);

// Global Variables to speed up access to these objects
extern Thread*  pCurrentRunningThread;
extern Task*    pCurrentRunningTask;
extern unint8   lastCycleStamp;

/*--------------------------------------------------------------------------*
 ** PPC405InterruptHandler::PPC405InterruptHandler
 *---------------------------------------------------------------------------*/
PPC405InterruptHandler::PPC405InterruptHandler() {

    // disable data chaching since we want to insert instructions into the ram
    // if we dont disable data caching here the data written may never reach main memory
    // since the instruction fetch on that address will return 0 and not the code inserted here
    int4 dccrval;

    asm volatile (
            // Save DCCR
            "   mfspr    %0,1018;"
            "   li        %%r11,0;"
            "   sync;"
            "   isync;"
            "   mtspr     1018,%%r11;"
            "   sync;"
            "   isync;"
            : "=r" (dccrval)
            :
            : "11"
    );

    // install syscall handler
    memcpy((void*) 0xc00, (void*) &IRQ0x0c00,(size_t) 0x4);

    memcpy((void*) 0x200, (void*) &IRQ0x0200,(size_t) 0x8);

    memcpy((void*) 0x700, (void*) &IRQ0x0700,(size_t) 0x8);

    // install externalirq handler
    memcpy((void*) 0x500, (void*) &IRQ0x0500,(size_t) 0x4);

    // install pit timer handler
    memcpy((void*) 0x1000, (void*) &IRQ0x1000,(size_t) 0x4);

    // install fit timer handler
    memcpy((void*) 0x1010, (void*) &IRQ0x1010,(size_t) 0x4);

    // install watchdog handler
    memcpy((void*) 0x1020, (void*) &IRQ0x1020,(size_t) 0x4);

    // install tlb data miss handler
    memcpy((void*) 0x1100, (void*) &IRQ0x1100,(size_t) 0x4);

    // install tlb instruction miss handler
    memcpy((void*) 0x1200, (void*) &IRQ0x1200,(size_t) 0x4);


    asm volatile (
            // Restore DCCR
            "   sync;"
            "   isync;"
            "    mtspr      1018,%0;"
            "   sync;"
            "   isync;"
            :
            : "r" (dccrval)
            :
    );
}

/*--------------------------------------------------------------------------*
 ** PPC405InterruptHandler::~PPC405InterruptHandler
 *---------------------------------------------------------------------------*/
PPC405InterruptHandler::~PPC405InterruptHandler() {
}

