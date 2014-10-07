/*
 * PagedRamMemManager.cc
 *
 *  Created on: 27.07.2013
 *      Author: dbaldin
 */

/*******************************************************
 *  INCLUDES
 *******************************************************/
#include "PagedRamMemManager.hh"
#include "inc/error.hh"
#include <kernel/Kernel.hh>

/*******************************************************
 *  DEFINES
 *******************************************************/

/* check PAGESIZE define */
#ifndef PAGESIZE
#error "PAGESIZE must be defined by the target architecture. Usually inside the HAT-Layer Implementation if exists."
#endif

#ifndef RAM_SIZE
#error "RAMSIZE must be defined by the target architecture."
#endif

#define NUM_PAGES ((RAM_SIZE / PAGESIZE) -2)

#if NUM_PAGES == 0
#error "NUM_PAGES must not be 0!"
#endif

/*******************************************************
 *  VARIABLES
 *******************************************************/
extern Kernel* theOS;
extern void* __KERNELEND;
extern void* __RAM_END;
extern void* __RAM_START;

#if MEM_CACHE_INHIBIT
extern void* __cache_inihibit_end;
#endif

/* map of used memory pages by tasks .. starting at the page after __KERNELEND
 this map must be big enough to cover all possible locations of tasks inside the memory */
static unint1 UsedPageTable[ NUM_PAGES];
static unint4 MemStart;

/*******************************************************
 *  FUNCTIONS
 *******************************************************/

PagedRamMemManager::PagedRamMemManager() {
    // mark all pages as unused

    for (int i = 0; i < NUM_PAGES; i++)
    {
        UsedPageTable[i] = (unint1) -1;
    }

    /* calculate free memory area (useable by tasks)
     * in case of cache inhibit memory support the memory map must be
     * as follows:
     * kernel | cache inhibit area | free memory
     * with each of the areas being page aligned!
     */
#if MEM_CACHE_INHIBIT
    MemStart = (unint4) alignCeil((char*) &__cache_inihibit_end, PAGESIZE);
#else
    MemStart = (unint4) alignCeil((char*) &__KERNELEND,PAGESIZE);
#endif

}

PagedRamMemManager::~PagedRamMemManager() {

}

ErrorT PagedRamMemManager::mapKernelPages(int pid) {

    for (int i = 0; i < NUM_PAGES; i++)
    {
        if (UsedPageTable[i] == 0)
        {
            theOS->getHatLayer()->map((void*) (MemStart + i * PAGESIZE), /* identical mapping */
            (void*) (MemStart + i * PAGESIZE),
            PAGESIZE, /* size is the pagesize */
            7, /* RWX, we might only use RW?*/
            0, /* domain 0 */
            pid, /* task pid */
            true); /* global == only for kernel*/
        }
    }

    return (cOk );
}

ErrorT PagedRamMemManager::markAsUsed(unint4 start, unint4 end, unint1 pid) {

    unint4 start_page   = (unint4) alignFloor((char*) start, PAGESIZE);
    unint4 end_page     = (unint4) alignFloor((char*) end, PAGESIZE);

    unint4 pe_start     = (start_page - MemStart) / PAGESIZE;
    unint4 pe_end       = (end_page - MemStart) / PAGESIZE;

    if (pe_end >= NUM_PAGES || pe_start >= NUM_PAGES) {
        ERROR("Invalid start - end region");
    }

    // mark the pages as used
    for (unint4 i = pe_start; i <= pe_end; i++)
        UsedPageTable[i] = pid;

    return (cOk );
}

ErrorT PagedRamMemManager::free(unint4 start, unint4 end) {
    unint4 start_page   = (unint4) alignFloor((char*) start, PAGESIZE);
    unint4 end_page     = (unint4) alignFloor((char*) end, PAGESIZE);

    int pe_start        = (start_page - MemStart) / PAGESIZE;
    int pe_end          = (end_page - MemStart) / PAGESIZE;

    if (pe_end >= NUM_PAGES || pe_start >= NUM_PAGES) {
           ERROR("Invalid start - end region");
    }

    // mark the pages as free
    for (int i = pe_start; i <= pe_end; i++)
        UsedPageTable[i] = (unint1) -1;

    return (cOk );
}

ErrorT PagedRamMemManager::freeAll(unint1 pid) {
    for (int i = 0; i < NUM_PAGES; i++)
    {
        if (UsedPageTable[i] == pid)
            UsedPageTable[i] = (unint1) -1;
    }

    return (cOk );
}

void* PagedRamMemManager::alloc(size_t size, unint1 pid) {
    unint4 area_size = 0;
    int area_start = -1;  // current consecutive free virtual memory area
    int i;

    for (i = 0; i < NUM_PAGES; i++)
    {
        // check if area used
        if (UsedPageTable[i] != (unint1) -1)
        {
            area_start = -1;
            area_size = 0;
        }
        else
        {
            if (area_start == -1)
            {  // area free and no area started yet
                area_start = MemStart + i * PAGESIZE;
                area_size = PAGESIZE;
            }
            else
            {
                area_size += PAGESIZE;
                if (area_size >= size)
                    break;  // found a area to be mapped
            }
        }
    }

    if (area_start == -1)
        return (0);

    // mark as used
    markAsUsed(area_start, area_start + size, pid);

    return ((void*) area_start);
}

