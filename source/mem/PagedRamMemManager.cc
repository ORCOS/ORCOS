/*
 * PagedRamMemManager.cc
 *
 *  Created on: 27.07.2013
 *    Copyright & Author: dbaldin
 */

/*******************************************************
 *  INCLUDES
 *******************************************************/
#include "PagedRamMemManager.hh"
#include "inc/error.hh"
#include "kernel/Kernel.hh"

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

    for (int i = 0; i < NUM_PAGES; i++) {
        UsedPageTable[i] = (unint1) -1;
    }

    /* calculate free memory area (useable by tasks)
     * in case of cache inhibit memory support the memory map must be
     * as follows:
     * kernel | cache inhibit area | free memory
     * with each of the areas being page aligned!
     */
#if MEM_CACHE_INHIBIT
    MemStart = (unint4) alignCeil(reinterpret_cast<char*>(&__cache_inihibit_end), PAGESIZE);
#else
    MemStart = (unint4) alignCeil(reinterpret_cast<char*>(&__KERNELEND), PAGESIZE);
#endif
}

PagedRamMemManager::~PagedRamMemManager() {
}

/*****************************************************************************
 * Method: PagedRamMemManager::mapKernelPages(int pid)
 *
 * @description
 *  Maps all pages used by the kernel, has pid 0, into the address space
 *  of the given pid.
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT PagedRamMemManager::mapKernelPages(int pid) {
    for (int i = 0; i < NUM_PAGES; i++) {
        if (UsedPageTable[i] == 0) {
            theOS->getHatLayer()->map(reinterpret_cast<void*>(MemStart + i * PAGESIZE), /* identical mapping */
                                      reinterpret_cast<void*>(MemStart + i * PAGESIZE), /* identical mapping */
                                      PAGESIZE,                                         /* size is the pagesize */
                                      7,                                                /* RWX, we might only use RW?*/
                                      0,                                                /* domain 0 */
                                      pid,                                              /* task pid */
                                      hatCacheWriteBack);
        }
    }

    return (cOk );
}

/*****************************************************************************
 * Method: PagedRamMemManager::markAsUsed(unint4 start, unint4 end, unint1 pid)
 *
 * @description
 *  Marks the memory area between start and end as used by the process space
 *  with given pid
 *
 * @params
 *  start       Physical starting page address
 *  end         Phyiscal end page address
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT PagedRamMemManager::markAsUsed(unint4 start, unint4 end, unint1 pid) {
    unint4 start_page   = (unint4) alignFloor(reinterpret_cast<char*>(start), PAGESIZE);
    unint4 end_page     = (unint4) alignFloor(reinterpret_cast<char*>(end), PAGESIZE);

    unint4 pe_start     = (start_page - MemStart) / PAGESIZE;
    unint4 pe_end       = (end_page - MemStart) / PAGESIZE;

    if (pe_end >= NUM_PAGES || pe_start >= NUM_PAGES) {
        ERROR("Invalid start - end region");
    }

    /* mark the pages as used */
    for (unint4 i = pe_start; i <= pe_end; i++)
        UsedPageTable[i] = pid;

    return (cOk );
}

/*****************************************************************************
 * Method: PagedRamMemManager::free(unint4 start, unint4 end)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT PagedRamMemManager::free(unint4 start, unint4 end) {
    unint4 start_page   = (unint4) alignFloor(reinterpret_cast<char*>(start), PAGESIZE);
    unint4 end_page     = (unint4) alignFloor(reinterpret_cast<char*>(end), PAGESIZE);

    int pe_start        = (start_page - MemStart) / PAGESIZE;
    int pe_end          = (end_page - MemStart) / PAGESIZE;

    if (pe_end >= NUM_PAGES || pe_start >= NUM_PAGES) {
        ERROR("Invalid start - end region");
    }

    // mark the pages as free
    for (int i = pe_start; i <= pe_end; i++)
        UsedPageTable[i] = (unint1) -1;

    return (cOk);
}

/*****************************************************************************
 * Method: PagedRamMemManager::freeAll(unint1 pid)
 *
 * @description
 *   Marks all pages used by address page pid as free again
 * @params
 *  pid         PID
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT PagedRamMemManager::freeAll(unint1 pid) {
    for (int i = 0; i < NUM_PAGES; i++) {
        if (UsedPageTable[i] == pid)
            UsedPageTable[i] = (unint1) -1;
    }

    return (cOk );
}

/*****************************************************************************
 * Method: PagedRamMemManager::alloc(size_t size, unint1 pid)
 *
 * @description
 * Tries to allocate a consecutive memory area of size "size" for
 * process with id pid.
 *
 *  Returns the physical memory address of the allocated region on success.
 *  Returns 0 if no area could be allocated.
 *
 * @params
 *
 * @returns
 *  void*         physical page
 *******************************************************************************/
void* PagedRamMemManager::alloc(size_t size, unint1 pid) {
    unint4 area_size = 0;
    int area_start = -1;  // current consecutive free virtual memory area
    int i;

    for (i = 0; i < NUM_PAGES; i++) {
        // check if area used
        if (UsedPageTable[i] != (unint1) -1) {
            area_start = -1;
            area_size = 0;
        } else {
            if (area_start == -1) {  // area free and no area started yet
                area_start = MemStart + i * PAGESIZE;
                area_size = PAGESIZE;
            } else {
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

    return (reinterpret_cast<void*>(area_start));
}

