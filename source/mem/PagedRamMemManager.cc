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
#include "assemblerFunctions.hh"

/*******************************************************
 *  DEFINES
 *******************************************************/

#ifndef RAM_SIZE
#error "RAMSIZE must be defined by the target architecture."
#endif

#define MAPPING_PAGESIZE 0x100000
#define MAX_NUM_PAGES    ((RAM_SIZE / MAPPING_PAGESIZE) -2)


#if MAX_NUM_PAGES == 0
#error "MAX_NUM_PAGES must not be 0!"
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

struct page_map {
    /* PID this 1 MB page is used by. -1 for unused */
    unint1 pid;
    /* is this page subdivided into 4 KB micro pages? */
    unint1 uses_micropages;
    unint1 used_micropages;
    unint1 reserved;
    /* bitmap of used 4 KB pages inside this 1 MB page.
     * 256 entries. */
    unint4 micropages[8];
} __attribute__((packed));

/* map of used memory pages by tasks .. starting at the page after __KERNELEND
 this map must be big enough to cover all possible locations of tasks inside the memory */
static unint4   NumPages;
static page_map Pages[MAX_NUM_PAGES];
static unint4   MemStart;

/*******************************************************
 *  FUNCTIONS
 *******************************************************/

PagedRamMemManager::PagedRamMemManager() :
        CharacterDevice(true, "memmap") {
    /* mark all pages as unused */
    memset(&Pages[0], 0, sizeof(Pages));

    for (int i = 0; i < MAX_NUM_PAGES; i++) {
        Pages[i].pid = (unint1) -1;
    }

    m_lock = 0;

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

    NumPages = (((intptr_t)&__RAM_END) - MemStart) / MAPPING_PAGESIZE;
}

PagedRamMemManager::~PagedRamMemManager() {
}

/*****************************************************************************
 * Method: PagedRamMemManager::mapRAM(int pid)
 *
 * @description
 *  Maps all RAM pages into the address space 1:1 for kernel access
 *  into the Process with given PID. (only accessable from kernel mode)
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT PagedRamMemManager::mapRAM(int pid) {
    for (unint4 i = 0; i < NumPages; i++)
    {
        theOS->getHatLayer()->map(reinterpret_cast<void*>(MemStart + i * MAPPING_PAGESIZE), /* identical mapping */
                                               reinterpret_cast<void*>(MemStart + i * MAPPING_PAGESIZE), /* identical mapping */
                                               MAPPING_PAGESIZE, /* size is the pagesize */
                                               7, /* RWX, we might only use RW?*/
                                               0, /* domain 0 */
                                               pid, /* task pid */
                                               hatCacheWriteBack,
                                               true);
    }

    return (cOk);
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
    unint4 start_page = (unint4) alignFloor(reinterpret_cast<char*>(start), MAPPING_PAGESIZE);
    unint4 end_page   = (unint4) alignFloor(reinterpret_cast<char*>(end - 1), MAPPING_PAGESIZE);

    unint4 pe_start = (start_page - MemStart) / MAPPING_PAGESIZE;
    unint4 pe_end   = (end_page - MemStart) / MAPPING_PAGESIZE;

    if (pe_end >= NumPages || pe_start >= NumPages) {
        ERROR("Invalid start - end region");
    }

    SMP_SPINLOCK_GET(m_lock);
    /* mark the pages as used */
    for (unint4 i = pe_start; i <= pe_end; i++) {
        Pages[i].pid = pid;
        Pages[i].uses_micropages = 0;
        Pages[i].used_micropages = 0;
    }
    SMP_SPINLOCK_FREE(m_lock);

    return (cOk );
}

/*****************************************************************************
 * Method: PagedRamMemManager::free_physical(unint4 start, unint4 end)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT PagedRamMemManager::free_physical(intptr_t start, intptr_t end) {
    LOG(MEM, DEBUG, "PagedRamMemManager::free_physical() %x - %x", start, end);

    // be sure addresses are at least 4 KB aligned to fit smallest page
    start =  (unint4) alignFloor(reinterpret_cast<char*>(start), 4096);
    end   =  (unint4) alignCeil (reinterpret_cast<char*>(end)  , 4096);

    unint4 start_page = (unint4) alignFloor(reinterpret_cast<char*>(start)  , MAPPING_PAGESIZE);
    unint4 end_page   = (unint4) alignFloor(reinterpret_cast<char*>(end - 1), MAPPING_PAGESIZE);
    unint4 pe_start   = (start_page - MemStart) / MAPPING_PAGESIZE;
    unint4 pe_end     = (end_page - MemStart)   / MAPPING_PAGESIZE;

    // sanity checks
    if (end <= start || ((unint4) start) < MemStart || pe_end >= NumPages || pe_start >= NumPages)
    {
        LOG(MEM, ERROR, "PagedRamMemManager::free() Invalid start - end region : %x-%x", start_page, end_page);
        return (cInvalidArgument);
    }

    if (end - start >= MAPPING_PAGESIZE) {
        SMP_SPINLOCK_GET(m_lock);
        // mark the pages as free
        for (unint4 i = pe_start; i <= pe_end; i++) {
            Pages[i].pid = (unint1) -1;
            Pages[i].uses_micropages = 0;
            Pages[i].used_micropages = 0;
            for (int j = 0; j < 8; j++) {
                Pages[i].micropages[j] = 0;
            }
        }
        SMP_SPINLOCK_FREE(m_lock);
    } else {
        // free sub pages
        if (pe_start != pe_end) {
            LOG(MEM, ERROR, "PagedRamMemManager::free() invalid cross page start and end address : %x-%x", start_page, end_page);
            return (cInvalidArgument);
        }

        start =  (unint4) alignFloor(reinterpret_cast<char*>(start), 4096);
        end   =  (unint4) alignCeil (reinterpret_cast<char*>(end), 4096);
        unint4 page_offset = (start - start_page) / 4096;
        unint4 num_pages   = (end - start) / 4096;

        LOG(MEM, DEBUG, "PagedRamMemManager::free() micropages: %x-%x. num_pages: %u, page_offset: %u", start, end, num_pages, page_offset);

        if (Pages[pe_start].uses_micropages != 1) {
            LOG(MEM, WARN, "PagedRamMemManager::free() invalid page region: %x-%x. Region not allocated.", start, end);
            return (cInvalidArgument);
        }

        if (Pages[pe_start].used_micropages < num_pages) {
            LOG(MEM, ERROR, "PagedRamMemManager::free() invalid page region: %x-%x. Num allocated pages %u < %u pages of region. ", start, end, Pages[pe_start].used_micropages, num_pages);
            return (cInvalidArgument);
        }

        SMP_SPINLOCK_GET(m_lock);
        Pages[pe_start].used_micropages -= num_pages;
        // clear bits
        for (unint4 i = page_offset; i < page_offset+num_pages; i++) {
            // clear bit
            int index    = i / 32;
            int indexbit = i - (index * 32);
            Pages[pe_start].micropages[index] &= ~(1 << (31 - indexbit));
        }
        // check if complete page is now free again
        if (Pages[pe_start].used_micropages == 0) {
            // free page
            Pages[pe_start].pid = (unint1) -1;
            Pages[pe_start].uses_micropages = 0;
            Pages[pe_start].used_micropages = 0;
            for (int j = 0; j < 8; j++) {
                Pages[pe_start].micropages[j] = 0;
            }
        }
        SMP_SPINLOCK_FREE(m_lock);
    }

    return (cOk);
}

/*****************************************************************************
 * Method: free_logical(unint4 start, unint4 end)
 *
 * @description
 *  Marks the memory pages from start to end as free.
 *
 * @params
 *  start       logical starting page address
 *  end         logical end page address
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT PagedRamMemManager::free_logical(intptr_t start, intptr_t end) {
    LOG(MEM, DEBUG, "PagedRamMemManager::free_logical() %x - %x", start, end);

    if (end <= start)
    {
        return (cInvalidArgument);
    }

    // be sure addresses are at least 4 KB aligned to fit smallest page
    start =  (unint4) alignFloor(reinterpret_cast<char*>(start), 4096);
    end   =  (unint4) alignCeil (reinterpret_cast<char*>(end), 4096);
    LOG(MEM, DEBUG, "PagedRamMemManager::free_logical() aligned: %x - %x", start, end);

    int4 length        = end - start;
    intptr_t address   = start;
    int4 mappingSize   = MAPPING_PAGESIZE;

    while (length > 0)
    {
        while (length >= mappingSize)
        {
            intptr_t phy_address = (intptr_t) theOS->getHatLayer()->getPhysicalAddress((void*) address);
            if (phy_address == 0)
            {
                LOG(MEM, ERROR, "PagedRamMemManager::free_logical() Invalid logical address %x", address);
                return (cError);
            }
            if (isError(free_physical(phy_address, phy_address + mappingSize))) {
                return (cError);
            }
            address += mappingSize;
            length  -= mappingSize;
        }
        if (mappingSize == MAPPING_PAGESIZE)
        {
            mappingSize  = 0x10000;
        } else {
            mappingSize  = 0x1000;
        }
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
    SMP_SPINLOCK_GET(m_lock);
    for (unint4 i = 0; i < NumPages; i++) {
        if (Pages[i].pid == pid) {
            Pages[i].pid = (unint1) -1;
            Pages[i].uses_micropages = 0;
            Pages[i].used_micropages = 0;

            for (int j = 0; j < 8; j++) {
                Pages[i].micropages[j] = 0;
            }
        }
    }
    SMP_SPINLOCK_FREE(m_lock);
    return (cOk );
}

/*****************************************************************************
 * Method: PagedRamMemManager::allocSubPages(int page, size_t size)
 *
 * @description
 *  Tries to allocate a physical page inside the referenced
 *  1 MB page. Allowed page sizes are 4 KB and 64 KB.
 *
 *  MUST BE CALLED WITH m_lock HELD!
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

    if (Pages[page].used_micropages < (256 - numpages)) {
        /* we got a chance of allocating this size */
        /* get first free bit.. */
        int index = -1;
        for (int i = 0; i < 8; i++) {
            int freeindex = __builtin_clz(~(Pages[page].micropages[i]));
            if (freeindex < 32) {
                index = freeindex + (i * 32);
                break;
            }
        }
        if (index == -1)
        {
            return (0);
        }
        if (size == 0x1000) {
            // allocate that page and return
            Pages[page].used_micropages += numpages;
            Pages[page].uses_micropages = 1;
            int indexentry = index / 32;
            int indexbit   = index - (indexentry * 32);
            Pages[page].micropages[indexentry] |= (1 << (31 - indexbit));
            return (MemStart + page * MAPPING_PAGESIZE + index * 4096);
        }
        // 64 KB page size
        // align to next allocation size 64KB
        index += (15);
        index &= ~(15);

        // search for 16 consecutive free 4KB pages
        while (index < 256)
        {
            int bitmask = 0xffff0000;
            if (index & 0x10)
            {
                bitmask = 0xffff;
            }
            int indexentry = index / 32;
            if (!(Pages[page].micropages[indexentry] & bitmask))
            {
                //found a free 64 KB area inside this page
                Pages[page].used_micropages += numpages;
                Pages[page].uses_micropages = 1;
                Pages[page].micropages[indexentry] |= bitmask;
                return (MemStart + page * MAPPING_PAGESIZE + index * 4096);
            }
            index += 16;
        }
        /* no area found .. return 0 */
    }
    return (0);
}

/*****************************************************************************
 * Method: PagedRamMemManager::alloc_physical(size_t size, unint1 pid)
 *
 * @description
 *  Tries to allocate a physical area of size 'size' for the address space of pid.
 *  The algorithm is implemented to minimize the number of mappings required to map the area.
 *  Thus, sizes > 1 MB will be mapped using ONLY 1 MB pages. Example: 1.2 MB will be allocated by using
 *  two contiguous 1 MB pages. High fragmentation can be introduces this way, thus it is the
 *  responsibility of the caller to ensure proper allocations.
 *
 *  The size is always rounded up to the next higher page size.
 *  Examples:
 *  40 KB allocations will result in one 64 KB page allocation.
 *  20 Bytes allocation in 4 KB allocations.
 *  70 KB in a 1 MB allocations.
 *
 * @params
 *  size        size to be allocated for the physically contiguous memory region.
 *  pid         address space identifier this area shall belong to
 *
 * @returns
 *  intptr_t    0  :  failure.
 *             > 0 : physical start address of the area allocated
 *******************************************************************************/
intptr_t PagedRamMemManager::alloc_physical(size_t size, unint1 pid) {
    unint4 area_size = 0;
    unint4 i;

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
        intptr_t area_start = (intptr_t) -1;  // current consecutive memory area

        SMP_SPINLOCK_GET(m_lock);
        for (i = 0; i < NumPages; i++)
        {
            if (Pages[i].pid != (unint1) -1) {
                /* area used .. reset start */
                area_start = (intptr_t) -1;
                area_size = 0;
            } else {
                /* free area */
                if (area_start == (intptr_t) -1) {  // area free and no area started yet
                    area_start = MemStart + i * MAPPING_PAGESIZE;
                    area_size = MAPPING_PAGESIZE;
                } else {
                    area_size += MAPPING_PAGESIZE;
                }
                if (area_size >= size)
                    break;  // found a area to be mapped
            }
        }
        SMP_SPINLOCK_FREE(m_lock);

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
        SMP_SPINLOCK_GET(m_lock);
        for (unint4 page = 0; page < NumPages; page++)
        {
            /* check if area is free... if so we can directly allocate from it */
            if (Pages[page].pid == (unint1) -1)
            {
                /* get this page */
                Pages[page].pid = pid;
                Pages[page].uses_micropages = 0;
                Pages[page].used_micropages = 0;
                addr = allocSubPages(page, size);
                break;
            }
            /* check if process already owns page */
            if (Pages[page].pid == pid && Pages[page].uses_micropages)
            {
                /* try to further allocate small 4KB or 64KB pages from it.. */
                addr = allocSubPages(page, size);
                if (addr != 0)
                {
                    /* success*/
                    break;
                }
            }
        }
        SMP_SPINLOCK_FREE(m_lock);

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

    // first map 1 MB pages as long as possible
    while (size > MAPPING_PAGESIZE)
    {
        /* first get mappings with size MAPPING_PAGESIZE*/
        intptr_t phys_addr = alloc_physical(MAPPING_PAGESIZE, pid);
        if (phys_addr == 0)
        {
            LOG(MEM, ERROR, "Error allocating %u bytes for mapping %x of size %u", MAPPING_PAGESIZE, logical_address, size);
            return (0);
        }
        intptr_t mapped_address = (intptr_t) theOS->getHatLayer()->map(reinterpret_cast<void*>(logical_address), /* map at logical address , maybe 0 */
                                                                       reinterpret_cast<void*>(phys_addr), /* map this physical area */
                                                                       MAPPING_PAGESIZE, /* size is the pagesize */
                                                                       hatProtectionRead | hatProtectionWrite, /* RW */
                                                                       0, /* domain 0 */
                                                                       pid, /* task pid */
                                                                       hatCacheWriteBack);
        if (mapped_address == 0)
        {
            /* error mapping */
            // TODO .. we should unmap all pages mapped before during this alloc_logical operation
            return (0);
        }
        size -= MAPPING_PAGESIZE;
        if (logical_address == 0)
        {
            logical_address = mapped_address;
            ret_address = mapped_address;
        }
        logical_address += MAPPING_PAGESIZE;
    }

    while (size > 0)
    {
        /* map sub pages.. first allocate 64 KB pages as long as possible */
        while (size > 0x010000)
        {
            intptr_t phys_addr = alloc_physical(0x010000, pid);
            if (phys_addr == 0)
            {
                LOG(MEM, ERROR, "Error allocating %u bytes for mapping %x of size %u", 0x010000, logical_address, size);
                return (0);
            }
            intptr_t mapped_address = (intptr_t) theOS->getHatLayer()->map(reinterpret_cast<void*>(logical_address), /* map at logical address , maybe 0 */
                                                                           reinterpret_cast<void*>(phys_addr), /* map this physical area */
                                                                           0x010000, /* size is the pagesize */
                                                                           hatProtectionRead | hatProtectionWrite, /* RW */
                                                                           0, /* domain 0 */
                                                                           pid, /* task pid */
                                                                           hatCacheWriteBack);
            if (mapped_address == 0)
            {
                /* error mapping */
                return (0);
            }
            size -= 0x010000;
            if (logical_address == 0)
            {
                // remember first mapped address
                logical_address = mapped_address;
                ret_address     = mapped_address;
            }

            logical_address += 0x010000;
        }

        // now map 4 KB pages as long as needed
        while (size > 0)
        {
            intptr_t phys_addr = alloc_physical(0x01000, pid);
            if (phys_addr == 0)
            {
                LOG(MEM, ERROR, "Error allocating %u bytes for mapping %x of size %u", 0x01000, logical_address, size);
                return (0);
            }
            intptr_t mapped_address = (intptr_t) theOS->getHatLayer()->map(reinterpret_cast<void*>(logical_address), /* map at logical address , maybe 0 */
                                                                           reinterpret_cast<void*>(phys_addr), /* map this physical area */
                                                                           0x01000, /* size is the pagesize */
                                                                           hatProtectionRead | hatProtectionWrite, /* RW, we might only use RW?*/
                                                                           0, /* domain 0 */
                                                                           pid, /* task pid */
                                                                           hatCacheWriteBack);
            if (mapped_address == 0)
            {
                /* error mapping */
                return (0);
            }
            if (size > 0x01000)
            {
                size -= 0x01000;
            } else {
                size = 0;
            }
            if (logical_address == 0)
            {
                logical_address = mapped_address;
                ret_address = mapped_address;
            }

            logical_address += 0x01000;
        }
    }

    return (ret_address);
}

/*****************************************************************************
 * Method: PagedRamMemManager::readBytes(char *bytes, unint4 &length)
 *
 * @description
 *  Allows reading the current page allocations in
 *  human readable ascii form.
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT PagedRamMemManager::readBytes(char *bytes, unint4 &length) {
    int RemainingLength = length;
    char* ptr = bytes;
    int len;
    char tmp[100];

    if (this->position == 0)
    {
        len = sprintf(tmp, "User RAM : 0x%08x-0x%08x %u MB\n", MemStart, MemStart + (NumPages * MAPPING_PAGESIZE) - 1, NumPages);
        if (len < RemainingLength) {
            memcpy(ptr, tmp, len);
            RemainingLength -= len;
            ptr += len;
        } else {
            this->position = -1;
            goto out;
        }
        len = sprintf(tmp, "Page Size: 0x%08x\n", MAPPING_PAGESIZE);
        if (len < RemainingLength) {
            memcpy(ptr, tmp, len);
            RemainingLength -= len;
            ptr += len;
        } else {
            this->position = -1;
            goto out;
        }
    }

    for (unint4 i = this->position & 0xffffff; i < NumPages; i++)
    {
        if (Pages[i].pid !=  (unint1) -1)
        {
            unint4 PageAddr = MemStart + i * MAPPING_PAGESIZE;
            len = sprintf(tmp, "[0x%08x-0x%08x] PID %u\n", PageAddr, PageAddr + MAPPING_PAGESIZE - 1, Pages[i].pid);
            if (len < RemainingLength) {
                memcpy(ptr, tmp, len);
                RemainingLength -= len;
                ptr += len;
            } else {
                goto out;
            }
            if (Pages[i].used_micropages > 0)
            {
                for (int j = this->position >> 24; j < 256; j++)
                {
                    int indexentry = j / 32;
                    int indexbit = (indexentry + j) - (indexentry * 32);
                    if (Pages[i].micropages[indexentry] & (1 << (31 - indexbit)))
                    {
                        len = sprintf(tmp, "  0x%08x (4K) used\n", PageAddr + j * 4096);
                        if (len < RemainingLength)
                        {
                            memcpy(ptr, tmp, len);
                            RemainingLength -= len;
                            ptr += len;
                        } else {
                            goto out;
                        }
                    }
                    this->position &= 0xffffff;
                    this->position |= j << 24;
                }
                this->position &= 0xffffff;
            }
        }
        this->position++;
    }

out:
    length = length - RemainingLength;
    return (cOk );
}
