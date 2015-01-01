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

ARMv7Cache::ARMv7Cache() {
    line_len = 64;

    unint4 type, clid = 0;

    // read cache type
    asm volatile(
            "MRC p15, 0, %0, c0, c0, 1;"
            "MRC p15,1, %1,c0,c0,1; "  // Read CP15 Cache Level ID Register
            : "=&r" (type), "=&r" (clid)
            :
            :
    );

    LOG(ARCH, INFO, "ARMv7Cache: Type: 0x%x, CLID: 0x%x", type, clid);
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
void ARMv7Cache::invalidate_data(void* start, void* end) {
    /* Invalidate the whole data cache by MVA*/

    unint4 mva;
    unint4 start_address = (unint4) start;
    unint4 end_address = (unint4) end;
    /*
     * If start address is not aligned to cache-line do not
     * invalidate the first cache-line
     */
    if (start_address & (line_len - 1)) {
        LOG(ARCH, WARN, "ARMv7Cache::invalidate_data() %s - start address is not aligned - 0x%08x", __func__, start);
        /* move to next cache line */
        start_address = (start_address + line_len - 1) & ~(line_len - 1);
    }

    /*
     * If stop address is not aligned to cache-line do not
     * invalidate the last cache-line
     */
    if (end_address & (line_len - 1)) {
        LOG(ARCH, WARN, "ARMv7Cache::invalidate_data() %s - stop address is not aligned - 0x%08x", __func__, end);
        /* align to the beginning of this cache line */
        end_address &= ~(line_len - 1);
    }

    LOG(ARCH, INFO, "ARMv7Cache::invalidate_data() 0x%08x - 0x%08x", start_address, end_address);

    for (mva = start_address; mva < end_address; mva = mva + line_len) {
        /* DCIMVAC - Invalidate data cache by MVA to PoC */
        asm volatile("mcr p15, 0, %0, c7, c6, 1" : : "r" (mva));
    }
    LOG(ARCH, INFO, "ARMv7Cache::invalidate_data() done");
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
void ARMv7Cache::invalidate_instruction(void* start, void* end) {
    unint4 mva;
    unint4 start_address = (unint4) start;
    unint4 end_address = (unint4) end;
    /*
     * If start address is not aligned to cache-line do not
     * invalidate the first cache-line
     */
    if (start_address & (line_len - 1)) {
        LOG(ARCH, WARN, "ARMv7Cache::invalidate_instruction() %s - start address is not aligned - 0x%08x\n", __func__, start);
        /* move to next cache line */
        start_address = (start_address + line_len - 1) & ~(line_len - 1);
    }

    /*
     * If stop address is not aligned to cache-line do not
     * invalidate the last cache-line
     */
    if (end_address & (line_len - 1)) {
        LOG(ARCH, WARN, "ARMv7Cache::invalidate_instruction() %s - stop address is not aligned - 0x%08x\n", __func__, end);
        /* align to the beginning of this cache line */
        end_address &= ~(line_len - 1);
    }

    for (mva = start_address; mva < end_address; mva = mva + line_len) {
        /* DCIMVAC - Invalidate data cache by MVA to PoC */
        asm volatile("mcr p15, 0, %0, c7, c5, 1" : : "r" (mva));
    }

    /* Full system DSB - make sure that the invalidation is complete */
    //CP15DSB;
    /* ISB - make sure the instruction stream sees it */
    //CP15ISB;
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
