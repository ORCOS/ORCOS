/*
 * PagedRamMemManager.cc
 *
 *  Created on: 27.07.2013
 *      Author: dbaldin
 */

#include "PagedRamMemManager.hh"
#include "inc/error.hh"
#include <kernel/Kernel.hh>

extern Kernel* theOS;

extern void* __KERNELEND;
extern void* __RAM_END;
extern void* __RAM_START;

#if MEM_CACHE_INHIBIT
extern void* __cache_inihibit_end;
#endif

// check PAGESIZE define
#ifndef PAGESIZE
#error "PAGESIZE must be defined by the target architecture. Usually inside the HAT-Layer Implementation if existend."
#endif

#ifndef RAM_SIZE
#error "RAMSIZE must be defined by the target architecture."
#endif

#define NUM_PAGES ((RAM_SIZE / PAGESIZE) -2)

// map of used memory pages by tasks .. starting at the page after __KERNELEND
// this map must be big enough to cover all possible locations of tasks inside the memory
static unsigned char UsedPageTable[ NUM_PAGES ];
static unint4 MemStart;

PagedRamMemManager::PagedRamMemManager() {
	// mark all pages as unused

	for (int i = 0; i< NUM_PAGES; i++) {
		UsedPageTable[i] = -1;
	}

	// calculate free memory area (useable by tasks)
#if MEM_CACHE_INHIBIT
	MemStart = (unint4) alignCeil((char*) &__cache_inihibit_end,PAGESIZE);
#else
	MemStart = (unint4) alignCeil((char*) &__KERNELEND,PAGESIZE);
#endif
}

PagedRamMemManager::~PagedRamMemManager() {

}

ErrorT PagedRamMemManager::mapKernelPages(int pid) {

	for (int i = 0; i< NUM_PAGES; i++) {
		if (UsedPageTable[i] == 0) {
			theOS->getHatLayer()->map((void*) (MemStart + i*PAGESIZE),(void*) (MemStart + i*PAGESIZE),PAGESIZE,7,0,pid, false);
		}
	}

	return (cOk);
}


ErrorT PagedRamMemManager::markAsUsed(unint4 start, unint4 end, unint1 pid) {

	unint4 start_page =  (unint4) alignFloor((char*) start,PAGESIZE);
    unint4 end_page   =  (unint4) alignFloor((char*) end,PAGESIZE);

	unint4 pe_start = (start_page - MemStart) / PAGESIZE;
	unint4 pe_end   = (end_page - MemStart) / PAGESIZE;

    // mark the pages as used
    for (unint4 i = pe_start; i <= pe_end; i++) UsedPageTable[i] = pid;

    return (cOk);
}

ErrorT PagedRamMemManager::free(unint4 start, unint4 end) {
	unint4 start_page =  (unint4) alignFloor((char*) start,PAGESIZE);
	unint4 end_page   =  (unint4) alignFloor((char*) end,PAGESIZE);

	int pe_start = (start_page - MemStart) / PAGESIZE;
	int pe_end   = (end_page - MemStart) / PAGESIZE;

    // mark the pages as used
    for (int i = pe_start; i <= pe_end; i++) UsedPageTable[i] = -1;

    return (cOk);
}

ErrorT PagedRamMemManager::freeAll(unint1 pid) {
	for (int i = 0; i< NUM_PAGES; i++) {
		if (UsedPageTable[i] == pid) UsedPageTable[i] = -1;
	}

	return (cOk);
}

void* PagedRamMemManager::alloc(size_t size, unint1 pid) {
	unint4 area_size  = 0;
	int area_start = -1; // current consecutive free virtual memory area
	int i;

	for (i = 0; i < NUM_PAGES; i++)
	{
		// check if area used
		if ( UsedPageTable[i] != -1) {
			area_start = -1;
			area_size = 0;
		} else {
			if (area_start == -1) {  // area free and no area started yet
				area_start = MemStart + i*PAGESIZE;
				area_size = PAGESIZE;
			} else {
				area_size += PAGESIZE;
				if (area_size >= size) break;  // found a area to be mapped
			}
		}
	}

	if (area_start == -1) return 0;

	// mark as used
	markAsUsed(area_start, area_start + size, pid);

	return ((void*) area_start);
}


