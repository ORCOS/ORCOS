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

#include "SCLConfig.hh"
#include <kernel/Kernel.hh>
#include "inc/newlib/newlib_helper.hh"
#include MemoryManagerCfd_hh
#include <assembler.h>
#include "inc/memtools.hh"
#include "inc/memio.h"

//! global reference to the kernel object
Kernel* theOS = 0;

#ifdef HAS_MemoryManager_HatLayerCfd
// logical address of our Memory Manager
MemoryManagerCfdCl* logMM = 0;
#endif

// heap_start and heap_end address used to clear the memory
extern void* _heap_start;
extern void* _heap_end;

void abort() {
 for (;;) {};
}


/*!
 * \ingroup startup
 * The main "C" entry point for the OS after the assembler startup code has been executed.
 */
extern "C"void kernelmain()
{
    // first of all clear the OS heap so we get a clean working system
    volatile void* addr = (void*) &_heap_start;

    // use this address if you want to clear the whole os heap
    // this may take quite a while
    volatile int* endaddr = (int*) &_heap_end;

    // use this address if you want to clear just the used part of the heap
    // much faster!
    //void* endaddr = (void*) ((unint4) &_heap_start + 0x2000);

    // now clear the memory area
    while (addr < endaddr)
    {
        *((unint4*) addr) = 0;
        addr = (void*) (((unint4) addr) + 4);
    }

    theOS = 0;

    // create MM working directly with the real physical addresses
    MemoryManagerCfdCl* memMan = new(&_heap_start) MemoryManagerCfdCl(&_heap_start  +  sizeof(MemoryManagerCfdCl) ,&_heap_end);

    // use the MM at its logical address for creating the kernel object
    theOS = (Kernel*) memMan->alloc(sizeof(Kernel)+ 16);
    theOS->setMemManager(memMan);

    // Initialize the device drivers
    theOS->initialize();

    // we shouldn't get here!
    while (true) {NOP;}
}

