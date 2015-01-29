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

/********************************************
 *  INCLUDES
 ********************************************/

#include "ARMv7HatLayer.hh"
#include "ARMv7PtEntry.hh"
#include "kernel/Kernel.hh"
#include "inc/memio.h"
#include "process/Task.hh"
#include "assemblerFunctions.hh"

/********************************************
 *  EXTERNAL SYMBOLS
 ********************************************/

/* Start / End of Cache Inhibit kernel heap area */
extern void* __cache_inihibit_start;
extern void* __cache_inihibit_end;

/* Page table placement for tasks */
extern void* __PageTableSec_start;
extern void* __PageTableSec_end;
extern void* __MaxNumPts;

/* Kernel Start / End */
extern void* __LOADADDRESS;
extern void* __KERNELEND;

/* Architecture specific additional mappings */
extern t_archmappings arch_kernelmappings;

extern Task* pCurrentRunningTask;
extern Kernel* theOS;

#define GETBITS(a, UP, LOW) ((a & (( (1 << (UP - LOW + 1)) -1) << LOW)) >> LOW)

/********************************************
 *  FUNCTION IMPLEMENTATION
 ********************************************/

ARMv7HatLayer::ARMv7HatLayer() {
    LOG(HAL, INFO, "ARMv7HatLayer: ARMv7 Page Table Base: 0x%08x", ((unint4)(&__PageTableSec_start)));
    LOG(HAL, INFO, "ARMv7HatLayer: %d Page Tables (%d Kb)", ((unint4) &__MaxNumPts), ((unint4) &__MaxNumPts) * 16);

    /* create initial kernel mappings for all page tables */
    for (unint4 i = 0; i < ((unint4) &__MaxNumPts); i++) {
        mapKernel(i);
    }
}

ARMv7HatLayer::~ARMv7HatLayer() {
}

/***********************************************
 * Method: ARMv7HatLayer::map(void* logBaseAddr,
 *                            void* physBaseAddr,
 *                            size_t size,
 *                            BitmapT protection,
 *                            byte zsel,
 *                            int pid,
 *                            bool cache_inhibit)
 *
 * @description
 *
 * Creates a new virtual memory mapping for the given PID with given logical and physical address.
 *
 * @params
 * \param logBaseAddr   The logical address of the mapping
 * \param phyBaseAddr   The physical address of the mapping
 * \param size          The length in Bytes of the mapped area
 * \param protection    The access rights of the mapping (RWX)
 * \param zsel          The zone the mapping belongs to (unused for ARM)
 * \param pid           The PID the mapping is created for
 * \param cache_inhibit If true inhibits caching inside this memory region. Usefully for MMIO areas.
 *
 * @returns
 * void*                The physical addreess.
 *
 **********************************************/
void* ARMv7HatLayer::map(void* logBaseAddr, void* physBaseAddr, size_t size, BitmapT protection, byte zsel, int pid, int cacheMode) {
    createPT(logBaseAddr, physBaseAddr, size, protection, zsel, pid, cacheMode, true);
    return (physBaseAddr);
}

/***********************************************
 * Method: ARMv7HatLayer::map(void* physBaseAddr,
 *                            size_t size,
 *                            BitmapT protection,
 *                            byte zsel,
 *                            int pid,
 *                            bool cache_inhibit)
 *
 * @description
 *
 * Creates a new virtual memory mapping for the given PID and the given physical address. The mapping will
 * use a free logical address region for the mapping and returns the logical base address of the created mapping.
 *
 * @params
 * \param logBaseAddr   The logical address of the mapping
 * \param phyBaseAddr   The physical address of the mapping
 * \param size          The length in Bytes of the mapped area
 * \param protection    The access rights of the mapping (RWX)
 * \param zsel          The zone the mapping belongs to (unused for ARM)
 * \param pid           The PID the mapping is created for
 * \param cache_inhibit If true inhibits caching inside this memory region. Usefully for MMIO areas.
 *
 * @returns
 * void*                The logical base address of the mapping.
 *
 **********************************************/
void* ARMv7HatLayer::map(void* phyBaseAddr, size_t size, BitmapT protection, byte zsel, int pid, int cacheMode) {
    /* TODO: currently we search for a consecutive memory area.. however we should utilize the paging mechanism
     * to avoid stupid fragmentation.*/

    myMutex.acquire();

    /* get a free virtual address area of length size */
    unint4 ptStartAddr = 0;
    unint4 t;

    ptStartAddr = ((unint4) (&__PageTableSec_start)) + pid * 0x4000;
    /* do search */

    unint4 area_size = 0;
    int area_start = -1;  // current consecutive free virtual memory area

    /* TODO: enhance this by using a list if free pages etc.
     * this area currently creates some unneeded latency */
    for (t = 0; t < 4096; t++) {
        unint4* addr = reinterpret_cast<unint4*>(ptStartAddr + 4 * t);

        // check if area used
        if ((*addr) != 0) {
            area_start = -1;
            area_size = 0;
        } else {
            if (area_start == -1) {  // area free and no area started yet
                area_start = t * SECTION_SIZE;
                area_size = SECTION_SIZE;
            } else {
                area_size += SECTION_SIZE;
                if (area_size >= size)
                    break;  // found a area to be mapped
            }
        }
    }

    myMutex.release();

    //RESTORE_IRQS(irqstatus);
    /* no free mapping area found? */
    if (area_start == -1)
        return (0);

    this->map(reinterpret_cast<void*>(area_start),
              phyBaseAddr,
              size,
              protection,
              zsel,
              pid,
              cacheMode);
    return (reinterpret_cast<void*>(area_start));
}

/***********************************************
 * Method: ARMv7HatLayer::mapKernel(int pidl)
 *
 * @description:
 *
 * Maps the kernel pages into the process with given PID.
 * The mapped areas are the kernel area, the cache inhibit data area
 * if used and the architecture dependent mappings provided by the
 * arch_kernelmappings structure for, e.g., MMIO devices.
 *
 * @params
 * \param pid           The PID the mapping is created for
 *
 **********************************************/
void ARMv7HatLayer::mapKernel(int pid) {
    /* Map Kernel .text and .data (cachable part) */
    this->createPT(reinterpret_cast<void*>(&__LOADADDRESS),      /* 1:1 mapping for kernel */
                   reinterpret_cast<void*>(&__LOADADDRESS),
                   (unint4) &__KERNELEND - (unint4) &__LOADADDRESS,
                   hatProtectionExecute | hatProtectionRead | hatProtectionWrite,
                   0,
                   pid,
                   hatCacheWriteBack,
                   false);

#if MEM_CACHE_INHIBIT
    /* Create the page with non cachable data.
     * Also 1:1 mapped as this belongs to the kernel  */
    this->createPT(reinterpret_cast<void*>(&__cache_inihibit_start),
                   reinterpret_cast<void*>(&__cache_inihibit_start),
                   (unint4) &__cache_inihibit_end - (unint4) &__cache_inihibit_start,
                   hatProtectionRead | hatProtectionWrite,
                   0,
                   pid,
                   hatCacheInhibit,
                   false);
#endif

    /* map all architecture specific areas */
    for (int i = 0; i < arch_kernelmappings.count; i++) {
        this->createPT(reinterpret_cast<void*>(arch_kernelmappings.mappings[i].vir_addr),
                       reinterpret_cast<void*>(arch_kernelmappings.mappings[i].phy_addr),
                       arch_kernelmappings.mappings[i].size,
                       arch_kernelmappings.mappings[i].protection,
                       0,
                       pid,
                       arch_kernelmappings.mappings[i].cacheMode,
                       false);
    }

    if (theOS->getRamManager() != 0)
        theOS->getRamManager()->mapKernelPages(pid);
}

/*****************************************************************************
 * Method: ARMv7HatLayer::createPT(void* logBaseAddr,
 *                                 void* physBaseAddr,
 *                                 size_t size,
 *                                 BitmapT protection,
 *                                 byte zsel,
 *                                 int pid,
 *                                 bool cache_inhibit,
 *                                 bool nonGlobal) {
 *
 * @description
 *  Creates the page table entry for the mapping specified by the paramters.
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
void* ARMv7HatLayer::createPT(void*     logBaseAddr,
                              void*     physBaseAddr,
                              size_t    size,
                              BitmapT   protection,
                              byte      zsel,
                              int       pid,
                              int       cacheMode,
                              bool      nonGlobal) {
    ARMv7PtEntry pte;
    pte.Clear();
    unint ptStartAddr = 0;

    // set up page tables
    pte.setXNBit(0x1);  // mark as not executeable

    // create descriptor
    if (nonGlobal == false) {       // kernel entries are global
        pte.setAP(0x1);             // only allow privileged access

        if (protection & hatProtectionExecute) {
            pte.setXNBit(0x0);
        }
    } else {
        // set protection rights for user mode
        if ((~(protection & hatProtectionRead)) && (~(protection & hatProtectionWrite))) {
            // no access
            pte.setAP(0x1);
        }
        if ((protection & hatProtectionRead) && (~(protection & hatProtectionWrite))) {
            // read only
            pte.setAP(0x2);
        }
        if ((protection & hatProtectionRead) && (protection & hatProtectionWrite)) {
            // read and write
            pte.setAP(0x3);
        }
        if (protection & hatProtectionExecute) {
            pte.setXNBit(0x0);
        }
    }

    pte.setnGBit(nonGlobal);
    // everything is in domain 0.. domain is always active and uses tlb permissions
    // to check if access is granted
    // kernel is always mapped in every page table with supervisor permissions required
    pte.setDomain(0);
    pte.setType(ptTypeSection);
    pte.setBaseAddr(ptTypeSection, physBaseAddr);

    /* Cachable bit depends on global ICACHE_ENable define*/
#if !DCACHE_ENABLE
    cacheMode = hatCacheInhibit;
#endif

      /* TEX: 1CB
       *
       * C B
       * 0 0 = non cachable
       * 0 1 = write back, write allocate
       * 1 0 = write through, no write allocate
       * 1 1 = write back, no write allocate
       * */
    switch (cacheMode) {
        case hatCacheWriteBack: {
            /* caching is allowed */
            /* set l2 cache to write-back, write allocate */
            pte.setTex(0x5);
            /* set l1 cache to write-back, write allocate */
            pte.setCBit(0);
            pte.setBBit(1);
            break;
        }
        case hatCacheWriteThrough: {
            /* set l2 cache to write-back, write allocate */
            pte.setTex(0x6);
            /* set l1 cache to write-back, write allocate */
            pte.setCBit(1);
            pte.setBBit(0);
            break;
        }
        case hatCacheMMIO: {
            /* set to shareable device */
            pte.setTex(0x0);
            pte.setCBit(0);
            pte.setBBit(1);
            //pte.setSBit(1);
            break;
        }
        case hatCacheInhibit:
        /* intentionally fall through*/
        default: {
            if (nonGlobal) {
                /* non kernel page, we still use cache
                 * however directly write through */
                pte.setTex(0x6);
                pte.setCBit(1);
                pte.setBBit(0);
            }
        }
    }

    // write descriptor to page table in memory (index depending on logBaseAddr)
    // TODO make page table placement by the OS and get the address by task->getPageTable() method
    // also change this in startThread!
    ptStartAddr = ((unint) &__PageTableSec_start) + pid * 0x4000;

    OUTW(ptStartAddr + (((unint)logBaseAddr >> 20) << 2), pte.getDesc());

    // size of entry bigger than section size 1 MB => create more entries
    if (size > SECTION_SIZE) {
        // calculate next logical starting address
        void* nextLogAddr   = reinterpret_cast<void*>((unint) logBaseAddr + 0x100000);
        // calculate next physical starting address
        void* nextPhysAddr  = reinterpret_cast<void*>((unint) physBaseAddr + 0x100000);
        // create new TLB entry
        this->createPT(nextLogAddr, nextPhysAddr, size - SECTION_SIZE, protection, zsel, pid, cacheMode, nonGlobal);
    }

    return (reinterpret_cast<void*>(physBaseAddr));
}

/*****************************************************************************
 * Method: ARMv7HatLayer::unmap(void* logBaseAddr, unint1 tid)
 *
 * @description
 *  Unmaps the given logical address from the address space tid.
 *  Removes the page table entry and invalidates the TLB containing the address
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT ARMv7HatLayer::unmap(void* logBaseAddr, unint1 tid) {
    // be sure we have the page address
    unint4 logpageaddr = ((unint4) logBaseAddr) >> 20;

    unint4 ptStartAddr = 0;

    unint4 pid;

    if (tid == 0) {
        if (pCurrentRunningTask == 0)
            pid = 0;
        else
            pid = pCurrentRunningTask->getId();
    } else {
        pid = tid;
    }

    ptStartAddr = ((unint4) (&__PageTableSec_start)) + pid * 0x4000;

    OUTW(ptStartAddr + (((unint)logBaseAddr >> 20) << 2), 0);

    logpageaddr = (logpageaddr << 20) | pid;

    // invalidate tlb entry
    asm volatile(
            "MCR p15, 0, %0, c8, c5, 1;"    // Invalidate Inst-TLB by MVA
            "MCR p15, 0, %0, c8, c6, 1;"    // Invalidate Data-TLB by MVA
            :
            : "l" (logpageaddr)
            : "r0"
    );

    return (cOk);
}

/*****************************************************************************
 * Method: ARMv7HatLayer::unmapAll(int pid)
 *
 * @description
 *  Removes all mappings of the address space pid including
 *  invalidation of TLB entries of the address space.
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT ARMv7HatLayer::unmapAll(int pid) {
    unint4 ptStartAddr  = 0;
    ptStartAddr         = ((unint4) (&__PageTableSec_start)) + pid * 0x4000;
    unint4* addr        = reinterpret_cast<unint4*> (ptStartAddr);

    /* invalidate the page table */
    for (unint4 t = 0; t < 4096; t++) {
        *addr = 0;
        addr++;
    }

    /* be sure Kernel is mapped */
    mapKernel(pid);

    unint4 asid = pid | pid << 8;

    /* invalidate tlb of asid
     * invalidate tlb entry */
    asm volatile(
            "MCR p15, 0, %0, c8, c5, 2;"  // Invalidate Inst-TLB by ASID
            "MCR p15, 0, %0, c8, c6, 2;"// Invalidate Data-TLB by ASID
            :
            : "r" (asid)
            : "r0"
    );

    return (cOk );
}

/*****************************************************************************
 * Method: ARMv7HatLayer::enableHAT()
 *
 * @description
 *  Enables the hardware translation unit. The processor
 *  will now treat all addresses as logical ones. If no mapping exists
 *  fauls will be generated.
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT ARMv7HatLayer::enableHAT() {
    /* enable MMU: set SCTLR bit 0 to 1 */
    asm volatile(
            //"MRC p15, 0, r0, c2, c0, 2;"    // Read CP15 Translation Table Base Control Register
            //"LDR r1, =0xFFFFFFF8;"
            //"AND r0, r0, r1;"
            "MOV r0,#0;"                    // TTBRC = 0, only use TTBR0 for translation

            "MCR p15,0,r0,c13,c0,0 ;"       // write FCSE PID Register 0 for kernel

            "MCR p15, 0, r0, c2, c0, 2;"    // Write Translation Table Base Control Register

            "MRC p15, 0, r1, c2, c0, 0;"    // Read
            //"LDR r0, =0xFFFFC000;"
            "MOVW r0, #:lower16:0xFFFFC000;"
            "MOVT r0, #:upper16:0xFFFFC000;"
            "BIC r1, r1, r0;"
            "ORR r1, r1, %0;"
            "MCR p15, 0, r1, c2, c0, 0;"    // Write Translation Table Base Register 0

            "MOV r0, #0;"
            "MCR p15,0,r0,c13,c0,1;"        // Write CP15 Context ID Register

            "MOV r1, #0x1;"                 // only activate domain 0
            "MCR p15, 0, r1, c3, c0, 0 ;"   // Write Domain Access Control Register

            "MOV r0, #0;"
            "MCR p15, 0, r0, c8, c5, 0;"    // Invalidate Inst-TLB
            "MCR p15, 0, r0, c8, c6, 0;"    // Invalidate Data-TLB

            "MRC p15, 0, r1, c1, c0, 0;"    // read CP15 Register 1
            "ORR r1, r1, #0x1;"
            "MCR p15, 0, r1, c1, c0, 0;"    // enable MMUs
            :
            : "r" (&__PageTableSec_start)
            : "r0","r1"
    );

    return (cOk );
}


/*****************************************************************************
 * Method: ARMv7HatLayer::disableHAT()
 *
 * @description
 *  Disables the hardware translation unit.
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT ARMv7HatLayer::disableHAT() {
    // set FSCE PID to zero
    // disable MMU
    asm volatile(
            "MRC p15, 0, r1, c1, c0, 0;"  // read CP15 Register 1
            "BIC r1, r1, #0x1;"
            "MCR p15, 0, r1, c1, c0, 0;"// disabled
            :
            :
            : "r0","r1"
    );
    return (cOk);
}

/*****************************************************************************
 * Method: ARMv7HatLayer::getLogicalAddress(void* physAddr)
 *
 * @description
 *  Translates a given physical address to its logical address
 *  inside the current address space.
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
void* ARMv7HatLayer::getLogicalAddress(void* physAddr) {
    /* search in page table */
    unint4 ptStartAddr = 0;
    unint4* logAddr = 0;
    unint4 t;
    unint4 pid;

    if (pCurrentRunningTask == 0)
        pid = 0;
    else
        pid = pCurrentRunningTask->getId();

    ptStartAddr = ((unint4) (&__PageTableSec_start)) + pid * 0x4000;

    unint4* addr = reinterpret_cast<unint4*>(ptStartAddr);

    for (t = 0; t < 4096; t++) {
        if (((unint4) physAddr >> 20) == (*addr >> 20)) {
            break;
        }
        addr++;
    }

    /* no address found? */
    if (t == 4096)
        return (reinterpret_cast<void*>(0xffffffff));

    logAddr = reinterpret_cast<unint4*>((t << 20) * 4);
    return (reinterpret_cast<void*> (logAddr + (((unint) physAddr << 12) >> 12)));
}

/*****************************************************************************
 * Method: ARMv7HatLayer::getPhysicalAddress(void* log_addr)
 *
 * @description
 *  Translates a given logical address to its phyiscal address
 *  inside the current address space.
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
void* ARMv7HatLayer::getPhysicalAddress(void* log_addr) {
    void* ret;

    asm volatile(

            "MCR p15,0,%1,c7,c8,2;"  // Write CP15 VA to User Read VA to PA Translation Register
            "MOV r0, #0x0;"
            "MCR p15,0,r0,c7,c5,4;"// Ensure completion of the CP15 write (ISB not working)
            "MRC p15,0,r1,c7,c4,0;"// Read CP15 PA from Physical Address Register
            "MOVW r0, #:lower16:0x00000FFF;"
            "MOVT r0, #:upper16:0x00000FFF;"
            //"LDR r0, =0x00000FFF;"
            "BIC r1, r1, r0;"
            //"LDR r0, =0xFFFFF000;"
            "MOVW r0, #:lower16:0xFFFFF000;"
            "MOVT r0, #:upper16:0xFFFFF000;"
            "BIC %1, %1, r0;"
            "ORR %0, r1, %1;"

            : "=r" (ret)
            : "r" ((unint)log_addr)
            : "r0", "r1"
    );

    if (ret == 0) {
        LOG(ARCH, ERROR, "ARMv7HatLayer::getPhysicalAddress() Address could not be translated: %x", log_addr);
        unint4 pid;
        if (pCurrentRunningTask == 0)
            pid = 0;
        else
            pid = pCurrentRunningTask->getId();
        this->dumpPageTable(pid);
    }

    return (reinterpret_cast<void*>(ret));
}

/*****************************************************************************
 * Method: ARMv7HatLayer::initialize()
 *
 * @description
 *  Initializes the page tables.
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
void ARMv7HatLayer::initialize() {
    /* clear page table area and make entries fault entries */
    unint4* addr      =  reinterpret_cast<unint4*>(&__PageTableSec_start);
    unint4* endaddr    = reinterpret_cast<unint4*>(&__PageTableSec_end);

    while (addr < endaddr) {
        *addr = 0;
        addr++;
    }
}


/*****************************************************************************
 * Method: ARMv7HatLayer::dumpPageTable(int pid)
 *
 * @description
 *  Dumps the page table of the given address space
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
void ARMv7HatLayer::dumpPageTable(int pid) {
    unint4 ptStartAddr = 0;
    unint4 t;
    ptStartAddr = ((unint4) (&__PageTableSec_start)) + pid * 0x4000;

    unint4* addr = reinterpret_cast<unint4*>(ptStartAddr);

    /* iterate over page table an print used entries */
    for (t = 0; t < 4096; t++) {
        if ((*addr) != 0) {
            /* print this one as error as this is the only reason (error) we might be dumping this! */
            unint4 logAddr = t * 0x100000;
            unint4 phyaddr = (((*addr) >> 20) << 20);
            LOG(ARCH, ERROR, "0x%8x - 0x%8x : 0x%8x - 0x%8x dom :%d XN: %d AP2: %d AP10: %d TEX: %x CB: %x",
                logAddr,
                logAddr + 0x100000 - 1,
                phyaddr,
                phyaddr + 0x100000 - 1,
                GETBITS(*addr, 8, 5),
                GETBITS(*addr, 4, 4),
                GETBITS(*addr, 15, 15),
                GETBITS(*addr, 11, 10),
                GETBITS(*addr, 14, 12),
                GETBITS(*addr, 3, 2));
        }
        addr++;
    }
}
