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
extern t_archmappings   arch_kernelmappings;
extern Task*            pCurrentRunningTask;
extern Kernel*          theOS;

/* 1 MB of area usable for page tables to map pages of size < 1 MB*/
typedef struct {
    unint1 usage[1024];
} SubsectionPageTable;

static SubsectionPageTable* pageTables;
static IDMap<1023>          freeSubPages;

#define GETBITS(a, UP, LOW) ((a & (( (1 << (UP - LOW + 1)) -1) << LOW)) >> LOW)

/********************************************
 *  FUNCTION IMPLEMENTATION
 ********************************************/

ARMv7HatLayer::ARMv7HatLayer() : myMutex("ARMv7HatLayer") {
    LOG(HAL, INFO, "ARMv7HatLayer: ARMv7 Page Table Base: 0x%08x", ((unint4)(&__PageTableSec_start)));
    LOG(HAL, INFO, "ARMv7HatLayer: %d Page Tables (%d Kb)", ((unint4) &__MaxNumPts), ((unint4) &__MaxNumPts) * 16);

    // allocate one 1 MB area for the subsection pagetables
    // the first KB of the area is used as a mapping table (see SubsectionPageTable)
    pageTables = (SubsectionPageTable*) theOS->getRamManager()->alloc_physical(0x100000, 0);
    if (pageTables == 0) {
        LOG(HAL, ERROR, "ARMv7HatLayer: Could not allocate memory for sub section size page tables");
    } else {
        memsetlong(pageTables, 0, 0x100000);
        for (int i = 0; i < 1023; i++) {
            pageTables->usage[i] = (unint1) -1;
        }
    }

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
 *                            bool cache_inhibit,
 *                            int priviligeOnly = false)
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
 * void*                The logical address.
 *
 **********************************************/
void* ARMv7HatLayer::map(void* logBaseAddr, void* physBaseAddr, size_t size, BitmapT protection, byte zsel, int pid, int cacheMode, int privilegeOnly) {
    if (logBaseAddr != 0) {
        return (createPT(logBaseAddr, physBaseAddr, size, protection, zsel, pid, cacheMode, !privilegeOnly));
    } else {
        return (map(physBaseAddr, size, protection, zsel, pid, cacheMode, privilegeOnly));
    }
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
void* ARMv7HatLayer::map(void* phyBaseAddr, size_t size, BitmapT protection, byte zsel, int pid, int cacheMode, int privilegeOnly) {
    myMutex.acquire();

    /* get a free virtual address area of length size */
    unint4 ptStartAddr = 0;
    unint4 t;

    ptStartAddr = ((unint4) (&__PageTableSec_start)) + pid * 0x4000;
    /* do search */
    int area_start   = -1;  // current consecutive free virtual memory area
    unint4 area_size = 0;

    if (size >= 0x100000) {
        /* TODO: enhance this by using a list of free pages etc.
         * this area currently creates some unneeded latency */
        for (t = 1024; t < 2048; t++) {
            unint4* addr = reinterpret_cast<unint4*>(ptStartAddr + 4 * t);

            // check if area used
            if ((*addr) != 0) {
                area_start = -1;
                area_size  = 0;
            } else {
                if (area_start == -1) {  // area free and no area started yet
                    area_start = t * SECTION_SIZE;
                    area_size  = SECTION_SIZE;
                } else {
                    area_size += SECTION_SIZE;
                }
                if (area_size >= size) {
                  goto done;
                }
            }
        }
    } else {
        /* search for an area of unused 4 KB logical address pages*/
        for (t = 1024; t < 2048; t++) {
            area_size  = 0;
            unint4* addr = reinterpret_cast<unint4*>(ptStartAddr + 4 * t);
            if (*addr == 0) {
                /* free logical address.. use this page for 4 KB subpage mappings */
                area_start = t * SECTION_SIZE;
                break;
            } else {
                /* used .. but maybe using 4 KB pages.. check entry */
                ARMv7PtEntry_t    pte;
                pte.raw_bytes = *addr;

                 if ((pte.raw_bytes & 0x3) == 0x2) {
                     /* section.. try next logical 1MB address */
                 } else {
                     /* page table entry! try allocate some 4 KB logical pages in this 1 MB area  */
                     unint4 pageTable = (pte.raw_bytes & 0xFFFFFC00);
                     if (pageTable == 0) {
                         LOG(HAL, ERROR, "ARMv7HatLayer::map(): invalid page table for logical address %x", t * SECTION_SIZE);
                         return (0);
                     }

                     for (int i = 0; i < 256; i++) {
                         unint4 pageTableEntry = INW(pageTable + (i << 2));
                         if (pageTableEntry != 0) {
                             area_start = -1;
                             area_size  = 0;
                             if (size > SMALL_PAGE_SIZE && ((((intptr_t)phyBaseAddr) & ~(LARGE_PAGE_SIZE-1)) == 0)) {
                                 /* jump to next 64 KB aligned page */
                                 i = (i + 16) & (~15);
                             }
                         } else {
                             if (area_start == -1) {
                                 area_start =  (t * SECTION_SIZE) + i * 0x001000;
                             }
                             area_size += 0x001000;
                         }
                         if (area_size >= size) {
                            goto done;
                         }
                     } /* for all 4 KB pages  */
                 } /* section | pagetable? */
            } /* free entry? */
        } /* for all upper 2 GB 1 MB pages*/
    } /* Section ? */

    done:

    myMutex.release();

    /* no free mapping area found? */
    if (area_start == -1) {
        LOG(HAL, ERROR, "ARMv7HatLayer::map(): Could not find a free logical address region to map %x", phyBaseAddr);
        dumpPageTable(pid);
        return (0);
    }

    return (createPT(reinterpret_cast<void*>(area_start), phyBaseAddr, size, protection, zsel, pid, cacheMode, !privilegeOnly));
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
                   SECTION_SIZE /*(unint4) &__KERNELEND - (unint4) &__LOADADDRESS,*/,
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
                   SECTION_SIZE, /*(unint4) &__cache_inihibit_end - (unint4) &__cache_inihibit_start,*/
                   hatProtectionRead | hatProtectionWrite,
                   0,
                   pid,
                   hatCacheInhibit,
                   false);
#endif

    if (pageTables != 0) {
        this->createPT(reinterpret_cast<void*>(pageTables),
                       reinterpret_cast<void*>(pageTables),
                       0x100000,
                       hatProtectionRead | hatProtectionWrite,
                       0,
                       pid,
                       hatCacheInhibit,
                       false);
    }

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

    /* MAP complete RAM 1:1 into kernel for fast access and to
     * allow drivers to work on physical addresses for HW accesses */
    if (theOS->getRamManager() != 0)
        theOS->getRamManager()->mapRAM(pid);


    /* remap pagetables .. to be sure cache inhibit is set! */
    if (pageTables != 0) {
        this->createPT(reinterpret_cast<void*>(pageTables),
                       reinterpret_cast<void*>(pageTables),
                       0x100000,
                       hatProtectionRead | hatProtectionWrite,
                       0,
                       pid,
                       hatCacheInhibit,
                       false);
    }
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
 *  Creates the page table entry for the mapping specified by the parameters.
 *  Maps the area with the smallest page size enclosing the area.
 *  Example: map size 68 Kb => creates an 1 MB mapping as this is the
 *  smallest size enclosing the area.
 *  A better mapping must be enforced by the algorithm calling this method.
 *  In this example the map method should be calling using a 64 Kb size and an
 *  additional 4 Kb size directly following the area.
 *
 * @params
 *
 * @returns
 *  void*         Logical address or 0 on error
 *******************************************************************************/
void* ARMv7HatLayer::createPT(void*     logBaseAddr,
                              void*     physBaseAddr,
                              size_t    size,
                              BitmapT   protection,
                              byte      zsel,
                              int       pid,
                              int       cacheMode,
                              bool      nonGlobal) {
    ARMv7PtEntry_t    pte;
    pte.raw_bytes     = 0;
    unint ptStartAddr = 0;

    LOG(HAL, DEBUG, "ARMv7HatLayer::createPT(): Requested map %x -> %x, size: %u, pid: %d", physBaseAddr, logBaseAddr, size, pid);

    // write descriptor to page table in memory (index depending on logBaseAddr)
    // TODO make page table placement by the OS and get the address by task->getPageTable() method
    // also change this in startThread!
    ptStartAddr = ((unint) &__PageTableSec_start) + pid * 0x4000;

    /* Cachable bit depends on global ICACHE_ENable define*/
#if !DCACHE_ENABLE
    cacheMode = hatCacheInhibit;
#endif

    if (size <= LARGE_PAGE_SIZE) {
        if (pageTables == 0) {
            LOG(HAL, ERROR, "ARMv7HatLayer::createPT(): Cannot map %x -> %x, size: %u: No page tables available.", physBaseAddr, logBaseAddr, size);
            return (0);
        }
        /* map small page */
        /* check if for this logical 1 MB page a page table exists */
        unint4 l1entry   = INW(ptStartAddr + (((unint)logBaseAddr >> 20) << 2));
        unint4 pageTable = (l1entry & 0xFFFFFC00);
        if (pageTable == 0) {
           /* no page assigned yet */
            int FreePageIndex = freeSubPages.getNextID();
            if (FreePageIndex == -1) {
                LOG(HAL, ERROR, "ARMv7HatLayer::createPT(): No free page table available!");
                return (0);
            }
            /*for (int i = 0; i < 1023; i++) {
                if (pageTables->usage[i] == (unint1) -1) {
                    pageTables->usage[i] = pid;
                    pageTable = ((unint4) pageTables) + 1024 * (i+1);
                    break;
                }
            }
            if (pageTable == 0) {
                LOG(HAL, ERROR, "ARMv7HatLayer::createPT(): No free page table available!");
                return (0);
            }*/
            pageTable    = ((unint4) pageTables) + 1024 * (FreePageIndex+1);
            unint4 entry = (pageTable & 0xFFFFFC00) | 0x1;
            LOG(HAL, TRACE, "ARMv7HatLayer::createPT(): Using pagetable %x, entry %x", pageTable, entry);
            OUTW(ptStartAddr + (((unint)logBaseAddr >> 20) << 2), entry);
        } else {
            /* check if entry is not already used for some non pagetable mapping */
            if ((l1entry & 0x3) == 0x2) {
                LOG(HAL, ERROR, "ARMv7HatLayer::createPT(): Cannot map %x -> %x, size: %u: Logical Address already mapped by section.", physBaseAddr, logBaseAddr, size);
                return (0);
            }
        }

        LOG(HAL, TRACE, "ARMv7HatLayer::createPT(): Setting entry in pagetable %x", pageTable);

        if (size <= SMALL_PAGE_SIZE) {
            /* SMALL PAGE (4 KB) */
            pte.small_page.type   = 0x1;
            pte.small_page.XN_bit = 1;
            if (nonGlobal == false) {       // kernel entries are global
                 pte.small_page.AP1_bits = 0x1; // only allow privileged access

                   if (protection & hatProtectionExecute) {
                       pte.small_page.XN_bit = 0;
                   }
            } else {
               // set protection rights for user mode
               if ((~(protection & hatProtectionRead)) && (~(protection & hatProtectionWrite))) {
                   // no access
                   pte.small_page.AP1_bits = 1;
               }
               if ((protection & hatProtectionRead) && (~(protection & hatProtectionWrite))) {
                   // read only
                   pte.small_page.AP1_bits = 0x2;
               }
               if ((protection & hatProtectionRead) && (protection & hatProtectionWrite)) {
                   // read and write
                   pte.small_page.AP1_bits = 0x3;
               }
               if (protection & hatProtectionExecute) {
                   pte.small_page.XN_bit = 0x0;
               }
            }

            pte.small_page.nG_bit = nonGlobal;
            pte.small_page.base_address = ((unint) physBaseAddr) >> 12;
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
                     pte.small_page.TEX_bits = 0x5;
                     /* set l1 cache to write-back, write allocate */
                     pte.small_page.C_bit = 0;
                     pte.small_page.B_bit = 1;
                     break;
                 }
                 case hatCacheWriteThrough: {
                     /* set l2 cache to write-back, write allocate */
                     pte.small_page.TEX_bits = 0x6;
                     /* set l1 cache to write-back, write allocate */
                     pte.small_page.C_bit = 1;
                     pte.small_page.B_bit = 0;

                     break;
                 }
                 case hatCacheMMIO: {
                     /* set to shareable device */
                     pte.small_page.TEX_bits = 0x0;
                     pte.small_page.C_bit = 0;
                     pte.small_page.B_bit = 1;
                     //pte.setSBit(1);
                     break;
                 }
                 case hatCacheInhibit:
                 /* intentionally fall through*/
                 default: {
                     if (nonGlobal) {
                         /* non kernel page, we still use cache
                          * however directly write through */
                         pte.small_page.TEX_bits = 0x6;
                         pte.small_page.C_bit = 1;
                         pte.small_page.B_bit = 0;
                     }
                 }
             }
             /* write entry */
             OUTW(pageTable + ((((unint)logBaseAddr >> 12) & 0xFF) << 2), pte.raw_bytes);
        } else {
            /* LARGE PAGE (64 KB) */
             pte.large_page.type   = 0x1;
             pte.large_page.XN_bit = 1;
             if (nonGlobal == false) {       // kernel entries are global
                  pte.large_page.AP1_bits = 0x1; // only allow privileged access

                    if (protection & hatProtectionExecute) {
                        pte.large_page.XN_bit = 0;
                    }
             } else {
                // set protection rights for user mode
                if ((~(protection & hatProtectionRead)) && (~(protection & hatProtectionWrite))) {
                    // no access
                    pte.large_page.AP1_bits = 1;
                }
                if ((protection & hatProtectionRead) && (~(protection & hatProtectionWrite))) {
                    // read only
                    pte.large_page.AP1_bits = 0x2;
                }
                if ((protection & hatProtectionRead) && (protection & hatProtectionWrite)) {
                    // read and write
                    pte.large_page.AP1_bits = 0x3;
                }
                if (protection & hatProtectionExecute) {
                    pte.large_page.XN_bit = 0x0;
                }
             }

             pte.large_page.nG_bit = nonGlobal;
             pte.large_page.base_address = ((unint) physBaseAddr) >> 16;
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
                      pte.large_page.TEX_bits = 0x5;
                      /* set l1 cache to write-back, write allocate */
                      pte.large_page.C_bit = 0;
                      pte.large_page.B_bit = 1;
                      break;
                  }
                  case hatCacheWriteThrough: {
                      /* set l2 cache to write-back, write allocate */
                      pte.large_page.TEX_bits = 0x6;
                      /* set l1 cache to write-back, write allocate */
                      pte.large_page.C_bit = 1;
                      pte.large_page.B_bit = 0;

                      break;
                  }
                  case hatCacheMMIO: {
                      /* set to shareable device */
                      pte.large_page.TEX_bits = 0x0;
                      pte.large_page.C_bit = 0;
                      pte.large_page.B_bit = 1;
                      //pte.setSBit(1);
                      break;
                  }
                  case hatCacheInhibit:
                  /* intentionally fall through*/
                  default: {
                      if (nonGlobal) {
                          /* non kernel page, we still use cache
                           * however directly write through */
                          pte.large_page.TEX_bits = 0x6;
                          pte.large_page.C_bit = 1;
                          pte.large_page.B_bit = 0;
                      }
                  }
              }
              /* write the 16 identical entries */
              int index = (((unint)logBaseAddr >> 12) & ~0xF) & 0xFF;
              for (int i = 0; i < 16; i++) {
                  OUTW(pageTable + ((index+i) << 2), pte.raw_bytes);
              }
        }
    } else {
        pte.section.type   = 0x2;
        pte.section.XN_bit = 1;
        // create descriptor
        if (nonGlobal == false) {       // kernel entries are global
            pte.section.AP1_bits = 0x1; // only allow privileged access

              if (protection & hatProtectionExecute) {
                  pte.section.XN_bit = 0;
              }
          } else {
              // set protection rights for user mode
              if ((~(protection & hatProtectionRead)) && (~(protection & hatProtectionWrite))) {
                  // no access
                  pte.section.AP1_bits = 1;
              }
              if ((protection & hatProtectionRead) && (~(protection & hatProtectionWrite))) {
                  // read only
                  pte.section.AP1_bits = 0x2;
              }
              if ((protection & hatProtectionRead) && (protection & hatProtectionWrite)) {
                  // read and write
                  pte.section.AP1_bits = 0x3;
              }
              if (protection & hatProtectionExecute) {
                  pte.section.XN_bit = 0x0;
              }
          }

          pte.section.nG_bit = nonGlobal;
          // everything is in domain 0.. domain is always active and uses tlb permissions
          // to check if access is granted
          // kernel is always mapped in every page table with supervisor permissions required
          pte.section.domain = 0;
          pte.section.base_address = ((unint) physBaseAddr) >> 20;
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
                  pte.section.TEX_bits = 0x5;
                  /* set l1 cache to write-back, write allocate */
                  pte.section.C_bit = 0;
                  pte.section.B_bit = 1;
                  break;
              }
              case hatCacheWriteThrough: {
                  /* set l2 cache to write-back, write allocate */
                  pte.section.TEX_bits = 0x6;
                  /* set l1 cache to write-back, write allocate */
                  pte.section.C_bit = 1;
                  pte.section.B_bit = 0;

                  break;
              }
              case hatCacheMMIO: {
                  /* set to shareable device */
                  pte.section.TEX_bits = 0x0;
                  pte.section.C_bit = 0;
                  pte.section.B_bit = 1;
                  //pte.setSBit(1);
                  break;
              }
              case hatCacheInhibit:
              /* intentionally fall through*/
              default: {
                  if (nonGlobal) {
                      /* non kernel page, we still use cache
                       * however directly write through */
                      pte.section.TEX_bits = 0x6;
                      pte.section.C_bit = 1;
                      pte.section.B_bit = 0;
                  }
              }
          }

          OUTW(ptStartAddr + (((unint)logBaseAddr >> 20) << 2), pte.raw_bytes);

          // size of entry bigger than section size 1 MB => create more entries
          if (size > SECTION_SIZE) {
              // calculate next logical starting address
              void* nextLogAddr   = reinterpret_cast<void*>((unint) logBaseAddr + SECTION_SIZE);
              // calculate next physical starting address
              void* nextPhysAddr  = reinterpret_cast<void*>((unint) physBaseAddr + SECTION_SIZE);
              // create new TLB entry
              this->createPT(nextLogAddr, nextPhysAddr, size - SECTION_SIZE, protection, zsel, pid, cacheMode, nonGlobal);
          }
    }


    return (reinterpret_cast<void*>(logBaseAddr));
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
    unint4 logpageaddr = 0;
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
    ARMv7PtEntry_t    pte;
    pte.raw_bytes = INW(ptStartAddr + (((unint)logBaseAddr >> 20) << 2));

    if ((pte.raw_bytes & 0x3) == 0x2) {
        /* section */
        logpageaddr = ((unint4) logBaseAddr) >> 20;
        logpageaddr = (logpageaddr << 20) | pid;
        OUTW(ptStartAddr + (((unint)logBaseAddr >> 20) << 2), 0);
    } else {
        /* page table entry! */
        unint4 pageTable = (pte.raw_bytes & 0xFFFFFC00);
        if (pageTable == 0) {
            LOG(HAL, ERROR, "ARMv7HatLayer::unmap(): invalid page table for logical address %x", logBaseAddr);
            return (0);
        }

        unint4 pageTableEntry = INW(pageTable + ((((unint)logBaseAddr >> 12) & 0xFF) << 2));
        if ((pageTableEntry & 0x3) == 0x1) {
            /* large page */
            logpageaddr = ((unint4) logBaseAddr) >> 16;
            logpageaddr = (logpageaddr << 16) | pid;

            int index = (((unint)logBaseAddr >> 12) & ~0xF) & 0xFF;
            for (int i = 0; i < 16; i++) {
                OUTW(pageTable + ((index+i) << 2), 0);
            }
        } else {
            /* small page */
            logpageaddr = ((unint4) logBaseAddr) >> 12;
            logpageaddr = (logpageaddr << 12) | pid;
            OUTW(pageTable + ((((unint)logBaseAddr >> 12) & 0xFF) << 2), 0);
        }

        // check if area is free again
        // we might do this to allow more 1MB pages to be mapped...
        // however we keep it as it is.. this area can then
        // only be used for small pages in the future again
       /* for (int i = 0; i < 256; i++) {
            if (INW(pageTable + (i<<2))) break;
        }*/
        // if we get here the whole 1 MB page is free again ... lets make it available
        // again

    }


    /* also directly invalidate TLB */
    /* invalidate tlb entry by mva */
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

    if (pageTables) {
        for (int i = 0; i < 1023; i++) {
            if (pageTables->usage[i] == pid) {
                /* clear page table */
                memset((void*) (((intptr_t)pageTables) + 1024 * (i+1)), 0, 1024);
                pageTables->usage[i] = -1;
                freeSubPages.freeID(i);
            }
        }
    }

    /* be sure Kernel is mapped */
    mapKernel(pid);

    unint4 asid = pid | pid << 8;

    /* invalidate tlb of asid
     * invalidate tlb entry */
    asm volatile(
            "MCR p15, 0, %0, c8, c5, 2;"  // Invalidate Inst-TLB by ASID
            "MCR p15, 0, %0, c8, c6, 2;"  // Invalidate Data-TLB by ASID
            :
            : "r" (asid)
            : "r0"
    );

    return (cOk);
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

    // TODO: update this to work with page tables
    // currently this method is not used

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
 *  Translates a given logical address to its physical address
 *  inside the current address space.
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
void* ARMv7HatLayer::getPhysicalAddress(void* log_addr) {
    void* ret;

    asm volatile(

            "MCR  p15,0,%1,c7,c8,0;"  // Write CP15 VA to Priviliged Read VA to PA Translation Register
            "MOV  r0, #0x0;"
            "MCR  p15,0,r0,c7,c5,4;"  // Ensure completion of the CP15 write (ISB not working)
            "MRC  p15,0,r1,c7,c4,0;"  // Read CP15 PA from Physical Address Register
            "MOVW r0, #:lower16:0x00000FFF;"
            "MOVT r0, #:upper16:0x00000FFF;"
            "BIC  r1, r1, r0;"
            "MOVW r0, #:lower16:0xFFFFF000;"
            "MOVT r0, #:upper16:0xFFFFF000;"
            "BIC  %1, %1, r0;"
            "ORR  %0, r1, %1;"

            : "=r" (ret)
            : "r" ((unint)log_addr)
            : "r0", "r1"
    );

    /*
    if (ret == 0) {
        LOG(ARCH, ERROR, "ARMv7HatLayer::getPhysicalAddress() Address could not be translated: %x", log_addr);
        unint4 pid;
        if (pCurrentRunningTask == 0)
            pid = 0;
        else
            pid = pCurrentRunningTask->getId();
        this->dumpPageTable(pid);
    }*/

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

            ARMv7PtEntry_t    pte;
            pte.raw_bytes = (*addr);

             if ((pte.raw_bytes & 0x3) == 0x2) {
                 /* section */
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
             } else {
                 /* page table entry! */
                 unint4 pageTable = (pte.raw_bytes & 0xFFFFFC00);
                 if (pageTable == 0) {
                     LOG(HAL, ERROR, "ARMv7HatLayer::dumpPageTable(): invalid page table for logical address %x", t * 0x100000);
                 } else {

                     for (int i = 0; i < 256; i++) {
                         unint4 pageTableEntry = INW(pageTable + (i << 2));
                         pte.raw_bytes = pageTableEntry;
                         if (pageTableEntry == 0) {
                             continue;
                         }
                         if ((pageTableEntry & 0x3) == 0x1) {
                             /* large page */
                             unint4 logAddr = (t * 0x100000) + (i * LARGE_PAGE_SIZE);
                             unint4 phyaddr = pte.large_page.base_address << 16;
                             LOG(ARCH, ERROR, "0x%8x - 0x%8x : 0x%8x - 0x%8x dom :%d XN: %d AP2: %d AP10: %d TEX: %x C: %x B: %x",
                                                logAddr,
                                                logAddr + LARGE_PAGE_SIZE - 1,
                                                phyaddr,
                                                phyaddr + LARGE_PAGE_SIZE - 1,
                                                0,
                                                pte.large_page.XN_bit,
                                                pte.large_page.AP2_bit,
                                                pte.large_page.AP1_bits,
                                                pte.large_page.TEX_bits,
                                                pte.large_page.C_bit,
                                                pte.large_page.B_bit);
                             i += 16;
                         } else {
                             /* small page */
                            unint4 logAddr = (t * 0x100000) + (i * SMALL_PAGE_SIZE);
                            unint4 phyaddr = pte.small_page.base_address << 12;
                            LOG(ARCH, ERROR, "0x%8x - 0x%8x : 0x%8x - 0x%8x dom :%d XN: %d AP2: %d AP10: %d TEX: %x C: %x B: %x",
                                               logAddr,
                                               logAddr + SMALL_PAGE_SIZE - 1,
                                               phyaddr,
                                               phyaddr + SMALL_PAGE_SIZE - 1,
                                               0,
                                               pte.small_page.XN_bit,
                                               pte.small_page.AP2_bit,
                                               pte.small_page.AP1_bits,
                                               pte.small_page.TEX_bits,
                                               pte.small_page.C_bit,
                                               pte.small_page.B_bit);
                         }
                     }
                 }
             }


        }
        addr++;
    }
}
