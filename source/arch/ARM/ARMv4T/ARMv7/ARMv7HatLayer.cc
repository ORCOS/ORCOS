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

#include <ARMv7HatLayer.hh>
#include <ARMv7MMU.hh>
#include "kernel/Kernel.hh"
#include "inc/memio.h"
#include "process/Task.hh"

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

/********************************************
 *  FUNCTION IMPLEMENTATION
 ********************************************/

ARMv7HatLayer::ARMv7HatLayer() {

    LOG(HAL, INFO, "ARMv7HatLayer: ARMv7 Page Table Base: 0x%08x",((unint4)(&__PageTableSec_start) ));
    LOG(HAL, INFO, "ARMv7HatLayer: %d Page Tables (%d Kb)",((unint4) &__MaxNumPts),((unint4) &__MaxNumPts) * 16);

    /* create initial kernel mappings for all page tables */
    for (unint4 i = 0; i < ((unint4) &__MaxNumPts); i++)
    {
        mapKernel(7, i, false);
    }

}

ARMv7HatLayer::~ARMv7HatLayer() {

}

void* ARMv7HatLayer::map(void* logBaseAddr, void* physBaseAddr, size_t size, BitmapT protection, byte zsel, int pid, bool cache_inhibit) {

    createPT(logBaseAddr, physBaseAddr, size, protection, zsel, pid, cache_inhibit, true);
    return (physBaseAddr);
}

void* ARMv7HatLayer::map(void* phyBaseAddr, size_t size, BitmapT protection, byte zsel, int pid, bool cache_inhibit) {

    /* TODO: currently we search for a consecutive memory area.. however we should utilize the paging mechanism
     * to avoid stupid fragmentation.*/

    DISABLE_IRQS(irqstatus);

    /* get a free virtual address area of length size */
    unint4 ptStartAddr = 0;
    unint4* addr;
    unint4 t;

    ptStartAddr = ((unint4) (&__PageTableSec_start)) + pid * 0x4000;
    /* do search */

    unint4 area_size = 0;
    int area_start = -1;  // current consecutive free virtual memory area

    for (t = 0; t < 4096; t++)
    {
        addr = (unint4*) (ptStartAddr + 4 * t);

        // check if area used
        if ((*addr) != 0)
        {
            area_start = -1;
            area_size = 0;
        }
        else
        {
            if (area_start == -1)
            {  // area free and no area started yet
                area_start = t * SECTION_SIZE;
                area_size = SECTION_SIZE;
            }
            else
            {
                area_size += SECTION_SIZE;
                if (area_size >= size)
                    break;  // found a area to be mapped
            }
        }
    }

    /* no free mapping area found? */
    if (area_start == -1)
        return (0);

    /* else map this one */
    RESTORE_IRQS(irqstatus);

    this->map((void*) area_start, phyBaseAddr, size, protection, zsel, pid, cache_inhibit);
    return ((void*) area_start);
}

void ARMv7HatLayer::mapKernel(BitmapT protection, int pid, bool nonGlobal) {

    /* Map Kernel .text and .data (cachable part) */
    this->createPT((void*) &__LOADADDRESS,
                   (void*) &__LOADADDRESS,
                   (unint4) &__KERNELEND - (unint4) &__LOADADDRESS,
                   hatProtectionExecute | hatProtectionRead | hatProtectionWrite,
                   0,
                   pid,
                   !ICACHE_ENABLE, /* Cachable bit depends on global ICACHE_ENable define*/
                   false);

#if MEM_CACHE_INHIBIT
    /* Create the page with non cachable data */
    this->createPT((void*) &__cache_inihibit_start,
                   (void*) &__cache_inihibit_start,
                   (unint4) &__cache_inihibit_end - (unint4) &__cache_inihibit_start,
                   hatProtectionRead | hatProtectionWrite,
                   0,
                   pid,
                   true,
                   false);
#endif

    /* map all architecture specific areas */
    for (int i = 0; i < arch_kernelmappings.count; i++)
    {
        this->createPT((void*) arch_kernelmappings.mappings[i].vir_addr,
                       (void*) arch_kernelmappings.mappings[i].phy_addr,
                       arch_kernelmappings.mappings[i].size,
                       arch_kernelmappings.mappings[i].protection,
                       0,
                       pid,
                       arch_kernelmappings.mappings[i].flags  & hatCacheInhibit,
                       false);
    }

    if (theOS->getRamManager() != 0)
        theOS->getRamManager()->mapKernelPages(pid);
}

void* ARMv7HatLayer::createPT(void* logBaseAddr, void* physBaseAddr, size_t size, BitmapT protection, byte zsel, int pid, bool cache_inhibit, bool nonGlobal) {
    ARMv7PtEntry pte;
    pte.Clear();
    unint ptStartAddr = 0;

    // set up page tables
    pte.setXNBit(0x1);  // mark as not executeable

    // create descriptor
    if (nonGlobal == false)
    { 	// kernel entries are global
        pte.setAP(0x1); 		// only allow privileged access

        if (protection & hatProtectionExecute)
        {
            pte.setXNBit(0x0);
        }
    }
    else
    {
        // set protection rights for user mode
        if ((~(protection & hatProtectionRead ))
                && (~(protection & hatProtectionWrite )))
        {
            // no access
            pte.setAP(0x1);
        }
        if ((protection & hatProtectionRead )
                && (~(protection & hatProtectionWrite )))
        {
            // read only
            pte.setAP(0x2);
        }
        if ((protection & hatProtectionRead )
                && (protection & hatProtectionWrite ))
        {
            // read and write
            pte.setAP(0x3);
        }
        if (protection & hatProtectionExecute)
        {
            pte.setXNBit(0x0);
        }
    }

    pte.setnGBit(nonGlobal);
    //pte.setDomain(pid);
    // everything is in domain 0.. domain is always active and uses tlb permissions
    // to check if access is granted
    // kernel is always mapped in every page table with supervisor permissions required
    pte.setDomain(0);
    pte.setType( ptTypeSection);
    pte.setBaseAddr(ptTypeSection, physBaseAddr);

    if (!cache_inhibit)
    {
        /* caching is allowed */
        /*	TEX: 1CB
         *
         * C B
         * 0 0 = non cachable
         * 0 1 = write back, write allocate
         * 1 0 = write through, no write allocate
         * 1 1 = write back, no write allocate
         * */

        /* set l2 cache to write-back, write allocate */
        pte.setTex(0x5);
        /* set l1 cache to write-back, write allocate */
        pte.setCBit(0);
        pte.setBBit(1);
    }
    else
    {
        /* caching not allowed */
        if (nonGlobal)
        {
            /* non kernel page, we still use cache
             * however directly write through */
            pte.setTex(0x6);
            pte.setCBit(1);
            pte.setBBit(0);
        }
    }

    // write descriptor to page table in memory (index depending on logBaseAddr)
    // TODO make page table placement by the OS and get the address by task->getPageTable() method
    // also change this in startThread!
    ptStartAddr = ((unint) &__PageTableSec_start) + pid * 0x4000;

    OUTW(ptStartAddr + (((unint )logBaseAddr >> 20) << 2), pte.getDesc());

    // size of entry bigger than section size 1 MB => create more entries
    if (size > SECTION_SIZE)
    {
        // calculate next logical starting address
        void* nextLogAddr = (void*) ((unint) logBaseAddr + 0x100000);
        // calculate next physical starting address
        void* nextPhysAddr = (void*) ((unint) physBaseAddr + 0x100000);
        // create new TLB entry
        this->createPT(nextLogAddr, nextPhysAddr, size - SECTION_SIZE, protection, zsel, pid, cache_inhibit, nonGlobal);
    }

    return ((void*) physBaseAddr);
}

ErrorT ARMv7HatLayer::unmap(void* logBaseAddr, unint1 tid) {

    // be sure we have the page address
    unint4 logpageaddr = ((unint4) logBaseAddr) >> 20;

    unint4 ptStartAddr = 0;

    unint4 pid;

    if (tid == 0)
    {
        if (pCurrentRunningTask == 0)
            pid = 0;
        else
            pid = pCurrentRunningTask->getId();
    }
    else
        pid = tid;

    ptStartAddr = ((unint4) (&__PageTableSec_start)) + pid * 0x4000;

    OUTW(ptStartAddr + (((unint )logBaseAddr >> 20) << 2), 0);

    logpageaddr = (logpageaddr << 20) | pid;

    // invalidate tlb entry
    asm volatile(
            "MCR p15, 0, %0, c8, c5, 1;"  // Invalidate Inst-TLB by MVA
            "MCR p15, 0, %0, c8, c6, 1;"// Invalidate Data-TLB by MVA
            :
            : "l" (logpageaddr)
            : "r0"
    );

    return (cOk );
}

ErrorT ARMv7HatLayer::unmapAll(int pid) {

    unint4 ptStartAddr = 0;
    unint4* addr;

    ptStartAddr = ((unint4) (&__PageTableSec_start)) + pid * 0x4000;

    for (unint4 t = 0; t < 4096; t++)
    {
        addr = (unint4*) (ptStartAddr + 4 * t);
        *addr = 0;
    }

    // be sure Kernel is mapped
    mapKernel(7, pid, false);

    unint4 asid = pid | pid << 8;

    // invalidate tlb of asid
    // invalidate tlb entry
    asm volatile(
            "MCR p15, 0, %0, c8, c5, 2;"  // Invalidate Inst-TLB by ASID
            "MCR p15, 0, %0, c8, c6, 2;"// Invalidate Data-TLB by ASID
            :
            : "r" (asid)
            : "r0"
    );

    return (cOk );
}

ErrorT ARMv7HatLayer::enableHAT() {

    // enable MMU: set SCTLR bit 0 to 1
    asm volatile(
            /*".align 4;"
             "mov    r0,pc;"
             "bx     r0;"
             ".code 32;"*/

            //"MRC p15, 0, r0, c2, c0, 2;"	// Read CP15 Translation Table Base Control Register
            //"LDR r1, =0xFFFFFFF8;"
            //"AND r0, r0, r1;"
            "MOV r0,#0;"// TTBRC = 0, only use TTBR0 for translation

            "MCR p15,0,r0,c13,c0,0 ;"// write FCSE PID Register 0 for kernel

            "MCR p15, 0, r0, c2, c0, 2;"// Write Translation Table Base Control Register

            "MRC p15, 0, r1, c2, c0, 0;"// Read
            "LDR r0, =0xFFFFC000;"
            "BIC r1, r1, r0;"
            "ORR r1, r1, %0;"
            "MCR p15, 0, r1, c2, c0, 0;"// Write Translation Table Base Register 0

            "MOV r0, #0;"
            "MCR p15,0,r0,c13,c0,1;"// Write CP15 Context ID Register

            "MOV r1, #0x1;"// only activate domain 0
            "MCR p15, 0, r1, c3, c0, 0 ;"// Write Domain Access Control Register

            "MOV r0, #0;"
            "MCR p15, 0, r0, c8, c5, 0;"// Invalidate Inst-TLB
            "MCR p15, 0, r0, c8, c6, 0;"// Invalidate Data-TLB

            "MRC p15, 0, r1, c1, c0, 0;"// read CP15 Register 1
            "ORR r1, r1, #0x1;"
            "MCR p15, 0, r1, c1, c0, 0;"// enable MMUs

            /*"add r0, pc,#1;"
             "bx  r0;"
             ".code 16;"*/
            :
            : "r" (&__PageTableSec_start)
            : "r0","r1"
    );

    return (cOk );
}

ErrorT ARMv7HatLayer::disableHAT() {
    // set FSCE PID to zero
    // disable MMU
    asm volatile(
            /*	".align 4;"
             "mov    r0,pc;"
             "bx     r0;"
             ".code 32;"*/

            "MRC p15, 0, r1, c1, c0, 0;"  // read CP15 Register 1
            "BIC r1, r1, #0x1;"
            "MCR p15, 0, r1, c1, c0, 0;"// disabled

            /*"add r0, pc,#1;"
             "bx  r0;"
             ".code 16;"*/
            :
            :
            : "r0","r1"
    );
    return (cOk );
}

void* ARMv7HatLayer::getLogicalAddress(void* physAddr) {
    // search in page table
    unint4 ptStartAddr = 0;
    unint4* logAddr = 0;
    unint4* addr;
    unint4 t;
    unint4 pid;

    if (pCurrentRunningTask == 0)
        pid = 0;
    else
        pid = pCurrentRunningTask->getId();

    ptStartAddr = ((unint4) (&__PageTableSec_start)) + pid * 0x4000;

    // do entry search

    for (t = 0; t < 4096; t++)
    {
        addr = (unint4*) (ptStartAddr + 4 * t);
        if (((unint4) physAddr >> 20) == (*addr >> 20))
        {
            break;
        }
    }

    // no address found..
    if (t == 4096)
        return (0);

    logAddr = (unint4*) ((t << 20) * 4);
    return ((void*) (logAddr + (((unint) physAddr << 12) >> 12)));
}

void* ARMv7HatLayer::getPhysicalAddress(void* log_addr) {
    void* ret;

    asm volatile(

            "MCR p15,0,%1,c7,c8,2;"  // Write CP15 VA to User Read VA to PA Translation Register
            "MOV r0, #0x0;"
            "MCR p15,0,r0,c7,c5,4;"// Ensure completion of the CP15 write (ISB not working)
            "MRC p15,0,r1,c7,c4,0;"// Read CP15 PA from Physical Address Register
            "LDR r0, =0x00000FFF;"
            "BIC r1, r1, r0;"
            "LDR r0, =0xFFFFF000;"
            "BIC %1, %1, r0;"
            "ORR %0, r1, %1;"

            : "=r" (ret)
            : "r" ((unint)log_addr)
            : "r0", "r1"
    );

    if (ret == 0)
    {
        LOG(ARCH, ERROR, "ARMv7HatLayer::getPhysicalAddress() Address could not be translated: %x",log_addr);
        unint4 pid;
        if (pCurrentRunningTask == 0)
            pid = 0;
        else
            pid = pCurrentRunningTask->getId();
        this->dumpPageTable(pid);
    }

    return ((void*) ret);
}

void ARMv7HatLayer::initialize() {

    /* clear page table area and make entries fault entries */
    void* addr = (void*) &__PageTableSec_start;
    int* endaddr = (int*) &__PageTableSec_end;

    while (addr < endaddr)
    {
        *((unint4*) addr) = 0;
        addr = (void*) (((unint4) addr) + 4);
    }

}

#define GETBITS(a,UP,LOW) ((a & (( (1 << (UP - LOW + 1)) -1) << LOW)) >> LOW)

void ARMv7HatLayer::dumpPageTable(int pid) {

    unint4 ptStartAddr = 0;
    unint4* addr;
    unint4 t;

    ptStartAddr = ((unint4) (&__PageTableSec_start)) + pid * 0x4000;
    /* iterate over page table an print used entries */

    for (t = 0; t < 4096; t++)
    {
        addr = (unint4*) (ptStartAddr + 4 * t);

        if ((*addr) != 0)
        {
            /* print this one as error as this is the only reason (error) we might be dumping this! */
            unint4 logAddr = t * 0x100000;
            unint4 phyaddr = (((*addr) >> 20) << 20);
            LOG(ARCH, ERROR, "0x%8x - 0x%8x : 0x%8x - 0x%8x dom :%d XN: %d AP2: %d AP10: %d TEX: %x CB: %x", logAddr, logAddr + 0x100000 -1, phyaddr, phyaddr + 0x100000 -1, GETBITS(*addr,8,5), GETBITS(*addr,4,4), GETBITS(*addr,15,15), GETBITS(*addr,11,10), GETBITS(*addr,14,12), GETBITS(*addr,3,2));
        }

    }

}
