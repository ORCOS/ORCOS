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


#ifndef RAM_SIZE
#error "RAMSIZE must be defined by the target architecture."
#endif


#define MAPPING_PAGESIZE 0x100000

#define NUM_PAGES ((RAM_SIZE / MAPPING_PAGESIZE) -2)

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

struct page_map {
    /* pid this 1 MB page is used by. -1 for unused */
    unint1 pid;
    /* is this page subdivided into 4 KB micro pages? */
    unint1 uses_micropages;
    unint1 used_micropages;
    unint1 reserved;
    /* bitmap of used 4 KB pages inside this 1 MB page.
     * 256 entries. */
    unint4 micropages[8];
};

/* map of used memory pages by tasks .. starting at the page after __KERNELEND
 this map must be big enough to cover all possible locations of tasks inside the memory */
static page_map pages[NUM_PAGES];
static unint4 MemStart;

/*******************************************************
 *  FUNCTIONS
 *******************************************************/

PagedRamMemManager::PagedRamMemManager() {
    /* mark all pages as unused */
    memset(&pages[0], 0, sizeof(pages));

    for (int i = 0; i < NUM_PAGES; i++) {
         pages[i].pid = (unint1) -1;
    }

    /* calculate free memory area (useable by tasks)
     * in case of cache inhibit memory support the memory map must be
     * as follows:
     * kernel | cache inhibit area | free memory
     * with each of the areas being page aligned!
     */
#if MEM_CACHE_INHIBIT
    MemStart = (unint4) alignCeil(reinterpret_cast<char*>(&__cache_inihibit_end), MAPPING_PAGESIZE);
#else
    MemStart = (unint4) alignCeil(reinterpret_cast<char*>(&__KERNELEND), MAPPING_PAGESIZE);
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
        if (pages[i].pid == 0) {
            theOS->getHatLayer()->map(reinterpret_cast<void*>(MemStart + i * MAPPING_PAGESIZE), /* identical mapping */
                                      reinterpret_cast<void*>(MemStart + i * MAPPING_PAGESIZE), /* identical mapping */
                                      MAPPING_PAGESIZE,                                         /* size is the pagesize */
                                      7,                                                        /* RWX, we might only use RW?*/
                                      0,                                                        /* domain 0 */
                                      pid,                                                      /* task pid */
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
ErrorT PagedRamMemManager::markAsUsed(intptr_t start, intptr_t end, unint1 pid) {
    unint4 start_page   = (unint4) alignFloor(reinterpret_cast<char*>(start), MAPPING_PAGESIZE);
    unint4 end_page     = (unint4) alignFloor(reinterpret_cast<char*>(end-1), MAPPING_PAGESIZE);

    unint4 pe_start     = (start_page - MemStart) / MAPPING_PAGESIZE;
    unint4 pe_end       = (end_page - MemStart) / MAPPING_PAGESIZE;

    if (pe_end >= NUM_PAGES || pe_start >= NUM_PAGES) {
        ERROR("Invalid start - end region");
    }

    /* mark the pages as used */
    for (unint4 i = pe_start; i <= pe_end; i++) {
        pages[i].pid = pid;
        pages[i].uses_micropages = 0;
        pages[i].used_micropages = 0;
    }

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
ErrorT PagedRamMemManager::free(intptr_t start, intptr_t end) {
    LOG(MEM, DEBUG, "PagedRamMemManager::free() %x - %x", start, end);

    if (end - start >= MAPPING_PAGESIZE) {
        unint4 start_page   = (unint4) alignFloor(reinterpret_cast<char*>(start), MAPPING_PAGESIZE);
        unint4 end_page     = (unint4) alignFloor(reinterpret_cast<char*>(end-1), MAPPING_PAGESIZE);

        int pe_start        = (start_page - MemStart) / MAPPING_PAGESIZE;
        int pe_end          = (end_page - MemStart) / MAPPING_PAGESIZE;

        if (pe_end >= NUM_PAGES || pe_start >= NUM_PAGES) {
            ERROR("Invalid start - end region");
        }

        // mark the pages as free
        for (int i = pe_start; i <= pe_end; i++) {
            pages[i].pid = (unint1) -1;
            pages[i].uses_micropages = 0;
            pages[i].used_micropages = 0;
            for (int j = 0; j < 8; j++) {
                pages[i].micropages[j] = 0;
            }
        }
    } else {
        // TODO free sub pages
    }

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
        if (pages[i].pid == pid) {
            pages[i].pid = (unint1) -1;
            pages[i].uses_micropages = 0;
            pages[i].used_micropages = 0;

            for (int j = 0; j < 8; j++) {
                pages[i].micropages[j] = 0;
            }
        }
    }

    return (cOk);
}

/*****************************************************************************
 * Method: PagedRamMemManager::allocContiguous(size_t size, unint1 pid)
 *
 * @description
 * Tries to allocate a physically contiguous memory area of size "size" for
 * process with id pid.
 *
 *  Returns the physical memory address of the allocated region on success.
 *  Returns 0 if no area could be allocated.
 *
 * @params
 *   logical_address address the area shall be mapped to if != 0
 *
 * @returns
 *  void*        logical address the area is mapped to (if no mmu the physical address is returned)
 *******************************************************************************/
intptr_t PagedRamMemManager::mapContiguous(size_t size, unint1 pid, intptr_t logical_address) {
    /* mapping must be contiguous */

    if (size < 0x100000) {
       if (size <= 0x1000) {
           size = 0x1000;
       } else {
           size = 0x10000;
       }
    }

    int physical_area_start   = alloc_physical(size, pid);
    if (physical_area_start == 0)
        return (0);

     logical_address = (intptr_t) theOS->getHatLayer()->map(
                                    reinterpret_cast<void*>(logical_address),                 /* map at logical address , maybe 0 */
                                    reinterpret_cast<void*>(physical_area_start),             /* map this physical area */
                                    size,                                                     /* size is the pagesize */
                                    7,                                                        /* RWX, we might only use RW?*/
                                    0,                                                        /* domain 0 */
                                    pid,                                                      /* task pid */
                                    hatCacheWriteBack);
    return (logical_address);
}


/*****************************************************************************
 * Method: PagedRamMemManager::allocSubPages(int page, size_t size)
 *
 * @description
 *  Tries to allocate a physical page inside the referenced
 *  1 MB page. Allowed page sizes are 4 KB and 64 KB.
 *
 * @params
 *  start       page index
 *  size        size: 4KB or 64 KB
 *
 * @returns
 *  intptr_t    0  :  failure.
 *             > 0 : physical start address of the area allocated
 *******************************************************************************/
intptr_t PagedRamMemManager::allocSubPages(int page, size_t size) {
    if (size != 0x1000 && size != 0x10000) {
        LOG(MEM, ERROR, "allocSubPages only supports 4KB or 64 KB pages! page size %u unsupported!", size)
        return (0);
    }
    int numpages = (size / 4096);

    if (pages[page].used_micropages < (256-numpages)) {
        /* we got a chance of allocating this size */
        /* get first free bit.. afterwards traverse bits to check if numpages directly
         * following are also free*/
        int index = -1;
        for (int i = 0; i < 8; i++) {
            int freeindex = __builtin_clz(~(pages[page].micropages[i]));
            if (freeindex < 32) {
                index = freeindex + (i * 32);
                break;
            }
        }

        index += (numpages-1);
        index &= ~(numpages-1);

        bool found = true;
        while (index < 256) {
            found = true;
            for (int i = 1; i < numpages; i++) {
                /* test all following bits */
                int indexentry = (index+i) / 32;
                int indexbit   = (index+i) - (indexentry * 32);
                if (pages[page].micropages[indexentry] & (1 << (31-indexbit))) {
                    /* page used.. search again*/
                    index = index + i;
                    index += (numpages-1);
                    index &= ~(numpages-1);
                    found = false;
                    break;
                }
            }
            if (found) goto alloc;
        }

        /* no area found .. return 0*/
        return (0);

        alloc:
        /* mark bits as used */
        pages[page].used_micropages += numpages;
        pages[page].uses_micropages = 1;
        for (int i = 0; i < numpages; i++) {
            int indexentry = (index+i) / 32;
            int indexbit   = (index+i) - (indexentry * 32);
            pages[page].micropages[indexentry] |= (1 << (31-indexbit));
        }
        return (MemStart + page * MAPPING_PAGESIZE + index * 4096);
    } else {
        return (0);
    }
}

/*****************************************************************************
 * Method: PagedRamMemManager::alloc_physical(size_t size, unint1 pid)
 *
 * @description
 *  Tries to allocate a physical area of size 'size' for the address space of pid.
 *  The algorithm is implemented to minimize the number of mappings required to map the area.
 *  Thus, sizes > 1 MB will be mapped using ONLY 1 MB pages. Example: 1,2 MB will be allocated by using
 *  two contiguous 1 MB pages. High fragmentation can be introduces this way, thus it is the
 *  responsibility of the caller to ensure proper allocations.
 *
 *  The size is always rounded up to the next higher page size.
 *  Examples:
 *  40 KB allocations will result in 64 KB page allocations.
 *  20 Bytes allocations in 4 KB allocations.
 *  70 KB in 1 MB allocations.
 *
 * @params
 *  size        size to be allocated for the phiyscally contiguous memory region.
 *  pid         address space identifier this area shall belong to
 *
 * @returns
 *  intptr_t    0  :  failure.
 *             > 0 : physical start address of the area allocated
 *******************************************************************************/
intptr_t PagedRamMemManager::alloc_physical(size_t size, unint1 pid) {
    unint4 area_size     = 0;
    int i;

    /* round up allocation size */
    if (size < 0x100000) {
        if (size <= 0x1000) {
            size = 0x1000;
        } else {
            size = 0x10000;
        }
    }

    /* if size is bigger than 1 MB use multiple 1 MB pages*/
    if (size >= 0x100000) {
        intptr_t area_start  = (intptr_t) -1;  // current consecutive memory area

        for (i = 0; i < NUM_PAGES; i++) {
             if (pages[i].pid != (unint1) -1) {
                 /* area used .. reset start */
                 area_start = (intptr_t) -1;
                 area_size  = 0;
             } else {
                 /* free area */
                 if (area_start == (intptr_t) -1) {  // area free and no area started yet
                     area_start = MemStart + i * MAPPING_PAGESIZE;
                     area_size  = MAPPING_PAGESIZE;
                 } else {
                     area_size += MAPPING_PAGESIZE;
                 }
                 if (area_size >= size)
                     break;  // found a area to be mapped
             }
        }

        /* no free contiguous area found? */
        if (area_start == (intptr_t) -1) {
            LOG(MEM, ERROR, "PagedRamMemManager::alloc_physical() Out of Memory: requested size: %u", size);
             return (0);
        }

        /* mark as used */
        markAsUsed(area_start, area_start + area_size, pid);

        LOG(MEM, DEBUG, "Allocated physical page %x (requested size: %u)", area_start, size);
        return (area_start);
    } else {
        intptr_t addr = 0;
        for (int page = 0; page < NUM_PAGES; page++) {
            /* check if area is free... if so we can directly allocate from it */
            if (pages[page].pid == (unint1) -1) {
                /* get this page */
                pages[page].pid = pid;
                pages[page].uses_micropages = 0;
                pages[page].used_micropages = 0;
                addr = allocSubPages(page, size);
                break;
            }
            /* check if process already owns page */
            if (pages[page].pid == pid && pages[page].uses_micropages) {
                /* try to further allocate small 4KB pages from it.. */
                addr = allocSubPages(page, size);
                if (addr != 0) {
                    /* success*/
                    break;
                }
            }
        }
        if (addr != 0) {
            LOG(MEM, DEBUG, "Allocated physical page %x (requested size: %u)", addr, size);
        } else {
            LOG(MEM, ERROR, "PagedRamMemManager::alloc_physical() Out of Memory: requested size: %u", size);
        }
        return (addr);
    }
}

/*****************************************************************************
 * Method: PagedRamMemManager::alloc_logical(size_t size, unint1 pid, intptr_t logical_address)
 *
 * @description
 *  Tries to allocate a logically contiguous area of size 'size' for the address space of pid.
 *  Tries to allocate the given size with the least number of pages used for mapping.
 *  The area is logically contiguous, however may be physically not contiguous.
 *  If physically contiguous areas shall be guaranteed use alloc_physical instead.
 *
 *  Example:
 *    size = 1070 KB
 *    Allocates one 1 MB Page + one 64 KB Page + two 4 KB pages.
 *
 *
 * @params
 *  size              size to be allocated for the logically contiguous memory region.
 *  pid               address space identifier this area shall belong to
 *  logical_address   logical start address the area shall be mapped to. If 0 a free logical
 *                    address will be chosen.
 *
 * @returns
 *  intptr_t    0  : failure.
 *             > 0 : logical start address of the area allocated
 *******************************************************************************/
intptr_t PagedRamMemManager::alloc_logical(size_t size, unint1 pid, intptr_t logical_address) {
    intptr_t ret_address = logical_address;
    while (size > MAPPING_PAGESIZE) {
        /* first get mappings with size MAPPING_PAGESIZE*/
        intptr_t phys_addr      = alloc_physical(MAPPING_PAGESIZE, pid);
        if (phys_addr == 0) {
            LOG(MEM, ERROR, "Error allocating %u bytes for mapping %x of size %u", MAPPING_PAGESIZE, logical_address, size);
            return (0);
        }
        intptr_t mapped_address = (intptr_t) theOS->getHatLayer()->map(
                                       reinterpret_cast<void*>(logical_address),       /* map at logical address , maybe 0 */
                                       reinterpret_cast<void*>(phys_addr),             /* map this physical area */
                                       MAPPING_PAGESIZE,                               /* size is the pagesize */
                                       hatProtectionRead | hatProtectionWrite,         /* RW */
                                       0,                                              /* domain 0 */
                                       pid,                                            /* task pid */
                                       hatCacheWriteBack);
        if (mapped_address == 0) {
            /* error mapping */
            return (0);
        }
        size -= MAPPING_PAGESIZE;
        if (logical_address == 0) {
            logical_address = mapped_address;
            ret_address     = mapped_address;
        }
        logical_address += MAPPING_PAGESIZE;
    }

    while (size > 0) {
        /* map sub pages */
        while (size > 0x010000) {
            intptr_t phys_addr = alloc_physical(0x010000, pid);
            if (phys_addr == 0) {
               LOG(MEM, ERROR, "Error allocating %u bytes for mapping %x of size %u", 0x010000, logical_address, size);
               return (0);
            }
            intptr_t mapped_address = (intptr_t) theOS->getHatLayer()->map(
                                                 reinterpret_cast<void*>(logical_address),       /* map at logical address , maybe 0 */
                                                 reinterpret_cast<void*>(phys_addr),             /* map this physical area */
                                                 0x010000,                                       /* size is the pagesize */
                                                 hatProtectionRead | hatProtectionWrite,         /* RW */
                                                 0,                                              /* domain 0 */
                                                 pid,                                            /* task pid */
                                                 hatCacheWriteBack);
          if (mapped_address == 0) {
              /* error mapping */
              return (0);
          }
          size -= 0x010000;
          if (logical_address == 0) {
              logical_address = mapped_address;
              ret_address     = mapped_address;
          }
          logical_address += 0x010000;
        }

        while (size > 0) {
            intptr_t phys_addr = alloc_physical(0x01000, pid);
            if (phys_addr == 0) {
               LOG(MEM, ERROR, "Error allocating %u bytes for mapping %x of size %u", 0x01000, logical_address, size);
               return (0);
            }
            intptr_t mapped_address = (intptr_t) theOS->getHatLayer()->map(
                                                 reinterpret_cast<void*>(logical_address),       /* map at logical address , maybe 0 */
                                                 reinterpret_cast<void*>(phys_addr),             /* map this physical area */
                                                 0x01000,                                        /* size is the pagesize */
                                                 hatProtectionRead | hatProtectionWrite,         /* RW, we might only use RW?*/
                                                 0,                                              /* domain 0 */
                                                 pid,                                            /* task pid */
                                                 hatCacheWriteBack);
          if (mapped_address == 0) {
              /* error mapping */
              return (0);
          }
          if (size > 0x01000) {
              size -= 0x01000;
          } else {
              size = 0;
          }
          if (logical_address == 0) {
              logical_address = mapped_address;
              ret_address     = mapped_address;
          }
          logical_address += 0x01000;
        }
    }

    return (ret_address);
}
