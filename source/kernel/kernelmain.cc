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
#include "kernel/Kernel.hh"
#include Kernel_MemoryManager_hh
#include "inc/memtools.hh"
#include "inc/memio.h"
#include "inc/types.hh"

/* global reference to the kernel object */
Kernel* theOS = 0;

/* heap_start and heap_end address used to clear the memory */
extern void* _heap_start;
extern void* _heap_end;

/* init section containing compiler generated initialization  code for variables etc */
extern void* __init_start;
extern void* __init_end;

/* heap_start and heap_end address used to clear the memory */
#if MEM_CACHE_INHIBIT
extern void* _heapi_start;
extern void* _heapi_end;
#endif

/*****************************************************************************
 * Method: void (*init_handler)(void)
 *******************************************************************************/
typedef  void (*init_handler)(void);

/*****************************************************************************
 * Method: abort()
 *
 * @description
 *  Abort handler. Should not be executed however might by used
 *  by compiler generated code.
 *
 *******************************************************************************/
void abort() {
    for (;;) {
    }
}

/*****************************************************************************
 * Method: kernelmain()
 *
 * @description
 *  The main "C" entry point for the OS after the assembler startup code has been executed.
 *******************************************************************************/
extern "C" __attribute__((used)) void kernelmain() {
    /* first of all clear the OS heap so we get a clean working system */
    size_t* addr = reinterpret_cast<size_t*>(&_heap_start);

    /* use this address if you want to clear the whole os heap
     *  this may take quite a while */
    size_t* endaddr = reinterpret_cast<size_t*>(&_heap_end);

    /* now clear the memory area */
    while (addr < endaddr) {
        *(addr) = 0;
        addr++;
    }

    size_t* init_start = reinterpret_cast<size_t*>(&__init_start);
    size_t* init_end   = reinterpret_cast<size_t*>(&__init_end);
    while (init_start < init_end) {
        /* execute the init code */
        init_handler initFunction = reinterpret_cast<init_handler>(*init_start);
        initFunction();
        init_start++;
    }

    theOS = 0;

#if MEM_CACHE_INHIBIT
    Kernel_MemoryManagerCfdCl* memMan = new (&_heap_start)
    Kernel_MemoryManagerCfdCl(&_heap_start + sizeof(Kernel_MemoryManagerCfdCl), &_heap_end, &_heapi_start, &_heapi_end);
#else
    // create MM working directly with the real physical addresses
    Kernel_MemoryManagerCfdCl* memMan = new(&_heap_start) Kernel_MemoryManagerCfdCl(&_heap_start + sizeof(Kernel_MemoryManagerCfdCl) , &_heap_end);
#endif

    /* use the MM at its logical address for creating the kernel object */
    theOS = reinterpret_cast<Kernel*>(memMan->alloc(sizeof(Kernel) + 16));
    theOS->setMemoryManager(memMan);

    /* Initialize the Kernel.. boot up the system*/
    theOS->initialize();

    __builtin_unreachable();
}

