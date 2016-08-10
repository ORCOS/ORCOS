/*
 * ARMv7Cache.cc
 *
 *  Created on: 09.12.2013
 *     Copyright & Author: dbaldin
 */

#include "ARMv7Cache.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

#define CP15ISB    asm volatile("mcr     p15, 0, %0, c7, c5, 4" : : "r" (0))
#define CP15DSB    asm volatile("mcr     p15, 0, %0, c7, c10, 4" : : "r" (0))


static char* cacheTypes[7] = {"No Cache", "I-Cache", "D-Cache" , "I+D Cache", "Uni-Cache" , "", ""};
static unint4 lineWords[7] = { 4, 8, 16, 32, 64, 128, 256 };

ARMv7Cache::ARMv7Cache() {
    line_len = 64;

    unint4 type, clid = 0;

    // read cache type
    asm volatile(
            "MRC p15, 0, %0, c0, c0, 1;"
            "MRC p15, 1, %1, c0, c0, 1; "  // Read CP15 Cache Level ID Register
            : "=&r" (type), "=&r" (clid)
            :
            :
    );

    // use CSIDR
    LoU = (clid >> 27) & 0x7;
    LoC = (clid >> 24) & 0x7;

    LOG(ARCH, INFO, "ARMv7Cache: Type: 0x%x, LoU: %u, LoC: %u", type, LoU, LoC);
    int level = 1;
    for (int i = 0; i < 6; i++) {
        LlC = i+1;
        unint1 cacheType = (clid >> (i*3)) & 0x7;
        if (cacheType == 0) {
            break;
        }
        unint4 size = 0;
        unint4 ccsidr;
        asm volatile(
                   "MCR p15, 2, %1, c0, c0, 0;"   // Select Cache Level
                   "ISB;"
                   "MRC p15, 1, %0, c0, c0, 0; "  // Read CP15 CCSIDR
                   : "=&r" (ccsidr)
                   : "r" (i)
                   :
           );
        unint4 lineLen =  lineWords[ccsidr & 0x7];
        unint4 numSets =  GETBITS(ccsidr, 27, 13) +1;
        unint4 assoc   =  GETBITS(ccsidr, 12, 3) + 1;
        size = (lineLen * sizeof(long) * numSets * assoc) / 1024;
        LOG(ARCH, INFO, "ARMv7Cache:   Level %u: %s %u KB (Linelen: %u, numSets: %u)", level, cacheTypes[cacheType], size, lineLen, numSets);
        level++;
        if (cacheType == 3) i++;
    }

    asm volatile("MCR p15, 2, %0, c0, c0, 0;" : : "r" (0) :);   // Select First Cache Level
}

ARMv7Cache::~ARMv7Cache() {
}


/*****************************************************************************
 * Method: ARMv7Cache::invalidate_data(void* start, void* end)
 *
 * @description
 *  Invalidates all data cache lines containing physical addresses between
 *  start and end.
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
void ARMv7Cache::invalidate_data(void* start, size_t len) {
    /* Invalidate the data cache by MVA*/

   if (len == 0)
       return;

    unint4 mva;
    unint4 start_address = (unint4) start;
    unint4 end_address   = start_address + len;

    LOG(ARCH, DEBUG, "ARMv7Cache::invalidate_data() 0x%08x - 0x%08x", start_address, end_address);

    if (start_address & (line_len - 1)) {
        clean_data((void*) start_address, line_len);
        /* move to next cache line */
        start_address = (start_address + line_len - 1) & ~(line_len - 1);
    }

    if (end_address & (line_len - 1)) {
        clean_data((void*)end_address, line_len);
        /* align to the beginning of this cache line */
        end_address &= ~(line_len - 1);
    }

    asm volatile("DMB");
    for (mva = start_address; mva <= end_address; mva = mva + line_len) {
        /* DCIMVAC - Invalidate data cache by MVA to PoC */
        asm volatile("mcr p15, 0, %0, c7, c6, 1" : : "r" (mva));
    }
    LOG(ARCH, DEBUG, "ARMv7Cache::invalidate_data() done");
    /* Full system DSB - make sure that the invalidation is complete */
    asm volatile("DSB ish");
    asm volatile("ISB");
}


/*****************************************************************************
 * Method: ARMv7Cache::clean_data(void* start, void* end)
 *
 * @description
 *  Cleans all data cache lines containing physical addresses between
 *  start and end causing the data to be written back to POC (main memory) if
 *  it has changed.
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
void ARMv7Cache::clean_data(void* start, size_t len) {

    if (len == 0)
        return;

    unint4 mva;
    unint4 start_address = (unint4) start;
    unint4 end_address = start_address + len;
    start_address &= ~(line_len-1);
    end_address   += line_len;
    end_address   &= ~(line_len-1);

    LOG(ARCH, DEBUG, "ARMv7Cache::clean_data() 0x%08x - 0x%08x", start_address, end_address);
    asm volatile("DSB");
    for (mva = start_address; mva <= end_address; mva = mva + line_len) {
        /* DCCIMVAC - Clean and invalidate data cache by MVA to PoC */
        asm volatile("mcr p15, 0, %0, c7, c14, 1" : : "r" (mva));
    }

    LOG(ARCH, DEBUG, "ARMv7Cache::clean_data() done");
    /* Full system DSB - make sure that the clean operation is complete */
    asm volatile("DSB");
}

/*****************************************************************************
 * Method: ARMv7Cache::invalidate_instruction(void* start, void* end)
 *
 * @description
 *  Invalidates all instruction cache lines containing physical addresses between
 *  start and end.
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
void ARMv7Cache::invalidate_instruction(void* start, size_t len) {
    unint4 mva;
    unint4 start_address = (unint4) start;
    unint4 end_address   = start_address + len;


    for (mva = start_address; mva <= end_address; mva = mva + line_len) {
        /* DCIMVAC - Invalidate data cache by MVA to PoC */
        asm volatile("mcr p15, 0, %0, c7, c5, 1" : : "r" (mva));
    }

    asm volatile("DSB ish");
    asm volatile("ISB");

}

/*****************************************************************************
 * Method: ARMv7Cache::invalidate(unint4 asid)
 *
 * @description
 *  Ensures that no valid cache line exists containing addresses from
 *  the given address space. For ARMv7 this unfortunatly means
 *  we must invalidate the whole cache ..
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
void ARMv7Cache::invalidate(unint4 asid) {
    /* unfortunatly we must invalidate the whole cache .. */
    asm volatile(
            "MOV r0, #0;"
            "MCR p15, 0, r0, c7 , c5, 0;"  /* invalidate whole instruction cache */
            :
            :
            : "r0"
    );
}
