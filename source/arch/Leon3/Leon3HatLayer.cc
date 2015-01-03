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

#include <Leon3HatLayer.hh>
#include "kernel/Kernel.hh"

extern void* _text_start;
extern void* _text_end;
extern void* _data_end;
extern void* _data_start;
extern void* __stack;
extern Kernel* theOS;
extern Task* pCurrentRunningTask;

Leon3HatLayer::Leon3HatLayer() {

    // initialize context table with 32 entries (max tasks)
    contextTable = (unsigned int*) theOS->getMemManager()->alloc( 32*4, true, 32*4 );
    for (int i = 0; i < 32*4; i++){
        contextTable[i] = 0;
        }

    // set context table pointer register
    asm volatile(                       \
                "set 0x100, %%g6;"      \
                "srl %0, 4, %%g7;"    \
                "sta %%g7, [%%g6] 0x19;" \
                :                      \
                : "r" (contextTable) \
                :
        );

    // 1:1 mapping of ISRs and Kernel code
    this->map( (void*) (((unint4)&_text_start)-0x4000), (void*) (((unint4)&_text_start)-0x4000), (unint4) &_text_end - (unint4) (((unint4)&_text_start)-0x4000), 3, 0, 0, !ICACHE_ENABLE );

    // 1:1 mapping of Kernel Heap and Kernel Stacks
    this->map( (void*) &_data_start, (void*) &_data_start, (unint4) &__stack - (unint4) &_data_start + (unint4) 0x0000, 3, 0, 0, !DCACHE_ENABLE );
}

ErrorT Leon3HatLayer::enableHAT() {

     // enable mmu
    asm volatile(                       \
            "lda  [%%g0] 0x19,  %%g7;" \
            "or   %%g7,  0x1,   %%g7;" \
            "sta  %%g7,  [%%g0] 0x19;" \
            :                         \
            :                          \
            :
    );
    return cOk;
 }

ErrorT Leon3HatLayer::disableHAT() {

     // disable mmu
    asm volatile(                       \
            "lda [%%g0] 0x19, %%g7;"  \
            "set 0x1, %%g6;"           \
            "not %%g6;"               \
            "and %%g7,  %%g6, %%g7;"  \
            "sta %%g7,  [%%g0] 0x19;" \
            :                         \
            :                          \
            :
    );
    return cOk;
}

void* Leon3HatLayer::map( void* logBaseAddr, void* physBaseAddr, size_t size, BitmapT protection, byte zsel, int pid,  bool cache_inhibit ) {
    // determine mapping size
    unint4 mappingSize = getMappingLevel(size);
    unint4 pageAddress = 0;
    unint4 tableEntry = 0;
    unint4* l1Table = 0;

    // level 0 entry

     LOG( KERNEL, DEBUG,(KERNEL, DEBUG, "HatLayer: Mapping 0x%x to 0x%x. Mapping Size: 0x%x, Size: 0x%x", logBaseAddr, physBaseAddr, mappingSize, size) );

    if (mappingSize < CONTEXT_SIZE) {
        if(contextTable[pid] == 0) {
            // create a new mmu context table
            l1Table = (unint4*) theOS->getMemManager()->alloc(4*256, true, 4*256);
            tableEntry = createPtd((unint4)l1Table);
            contextTable[pid] = tableEntry;

            if (pid > 0) {
                // each task context needs a mapping of the kernel memory areas (access only in supervisor mode)
                // kernel mapping
                this->map( (void*) (((unint4)&_text_start)-0x4000), (void*) (((unint4)&_text_start)-0x4000), (unint4) &_text_end - (unint4) (((unint4)&_text_start)-0x4000), 3, 0, pid, !ICACHE_ENABLE );


                // 1:1 mapping of Kernel Heap and Kernel Stacks
                this->map( (void*) &_data_start, (void*) &_data_start, (unint4) &__stack - (unint4) &_data_start + (unint4) 0x0000, 3, 0, pid, !DCACHE_ENABLE );
            }

            // 1:1 mapping of memory-mapped I/O
            this->map( (void*) MMIO_START_ADDRESS, (void*) MMIO_START_ADDRESS, MMIO_AREA_LENGTH, 3, 0, pid, true );

        #if AIS
            this->map( (void*) 0x49000000, (void*) 0x49000000, 256, 3, 0, pid, true );

            // AIS Error Hardware mapping
            this->map( (void*) 0xc0000000, (void*) 0xc0000000, 0x10000000, 3, 0, pid, true );
        #endif

        #ifdef HAS_Board_SHMCfd
            // map shared memory area
            this->map( (void*) SHM_START, (void*) SHM_START, 0x8000, 3, 0, pid, true );
        #endif

        #ifdef HAS_Board_ETHCfd
            // map greth buffers to a 256 kb page
            this->map( (void*) 0x4F000000,(void*) 0x4F000000, 0x40000 ,7,3,pid, !ICACHE_ENABLE);
        #endif

        } else {
            unint4 ptd = contextTable[pid];
            ptd &= 0xFFFFFFFC;
            ptd = ptd << 4;
            l1Table = (unint4*)ptd;
        }

    } else {
        tableEntry = createPte(0x80000000 & (unint)physBaseAddr, protection, zsel, cache_inhibit);
        contextTable[pid] = tableEntry;
        //return (void*)((unint)physBaseAddr & 0x80000000);
        return physBaseAddr;
    }

    pageAddress = addRegionEntry(logBaseAddr, physBaseAddr, mappingSize, protection, zsel, cache_inhibit, l1Table);

    if (size > mappingSize) {

        size = size - (size_t)mappingSize;
        map((void*)((unint4)logBaseAddr + ((unint4) mappingSize)), (void*)((unint4)physBaseAddr + ((unint4) mappingSize)), size, protection, zsel, pid, cache_inhibit );
    }
    asm volatile(                       \
                    "set 0x400, %%g7;"        \
                    "sta %%g0, [%%g7] 0x18;"  \
                    :                         \
                    :                          \
                    :
            );
    return (void*)pageAddress;
}

ErrorT Leon3HatLayer::unmap( void* logBaseAddr ) {

    LOG( KERNEL, DEBUG,(KERNEL, DEBUG, "HatLayer: Unmap 0x%x", logBaseAddr) );
    int pid = 0;
    GETPID(pid);

    unint4 ptd = contextTable[pid];
    ptd = ptd & 0xFFFFFFFC;
    ptd = ptd << 4;
    unint4* l1Table = (unint4*)ptd;


    asm volatile(                       \
                        "set 0x400, %%g7;"        \
                        "sta %%g0, [%%g7] 0x18;"  \
                        :                         \
                        :                          \
                        :
                );

    unint4 l1Index = ((unint4)logBaseAddr) & 0xFF000000;
    l1Index = l1Index >> 24;
    unint4 tableEntry = l1Table[l1Index];
    if ((tableEntry & 0x3) == 2) {
        l1Table[l1Index] = 0;
        return cOk;
    }

    unint4* l2Table = (unint4*) (((tableEntry & 0xFFFFFFFC)) << 4);
    unint4 l2Index = ((unint4)logBaseAddr) & 0x00FC0000;
    l2Index = l2Index >> 18;
    tableEntry = l2Table[l2Index];

    if ((tableEntry & 0x3) == 2) {
            l2Table[l2Index] = 0;
            return cOk;
        }

    unint4* l3Table = (unint4*) (((tableEntry & 0xFFFFFFFC)) << 4);
    unint4 l3Index = ((unint4)logBaseAddr) & 0x0003F000;
    l3Index = l3Index >> 12;
    l3Table[l3Index] = 0;

    return cOk;

}

unint4 Leon3HatLayer::addRegionEntry(void* logBaseAddr, void* physBaseAddr, unint4 mappingSize, BitmapT protection, byte zsel, bool cache_inhibit, unint4* l1Table) {
    // get the index for region entry
    unint4 l1Index = ((unint4)logBaseAddr) & 0xFF000000;
    l1Index = l1Index >> 24;

    unint4 tableEntry = 0;
    unint4* l2Table = 0;
    //if(l1Table[l1Index] == 0) {
        if (mappingSize == REGION_SIZE) {
            // create a table entry
            tableEntry = createPte(0xFF000000 & (unint)physBaseAddr, protection, zsel, cache_inhibit);
            l1Table[l1Index] = tableEntry;
            // no other entries needed
            return (unint4)physBaseAddr &  (unint4) 0xFF000000;
        } else {
            if ((l1Table[l1Index] == 0)){
                // create a table descriptor  (reference to next level table)
                l2Table = (unint4*) theOS->getMemManager()->alloc(4*256, true, 4*256);
            } else {
                l2Table = (unint4*) (((l1Table[l1Index] & 0xFFFFFFFC)) << 4);
            }
            tableEntry = createPtd((unint4)l2Table);
            l1Table[l1Index] = tableEntry;
        }
    //}
    return addSegmentEntry(logBaseAddr, physBaseAddr, mappingSize, protection, zsel, cache_inhibit, l2Table);

}

unint4 Leon3HatLayer::addSegmentEntry(void* logBaseAddr, void* physBaseAddr, unint4 mappingSize, BitmapT protection, byte zsel, bool cache_inhibit, unint4* l2Table) {
    unint4 l2Index = ((unint4)logBaseAddr) & 0x00FC0000;
    l2Index = l2Index >> 18;

    unint4 tableEntry = 0;
    unint4* l3Table = 0;
    //if(l2Table[l2Index] == 0) {
        if (mappingSize == SEGMENT_SIZE) {
            // create a table entry
            tableEntry = createPte(0xFFFC0000 & (unint)physBaseAddr, protection, zsel, cache_inhibit);
            l2Table[l2Index] = tableEntry;
            return (unint4)physBaseAddr &  (unint4)0xFFFC0000;
        } else {
            if (l2Table[l2Index] == 0) {
                // create a table descriptor  (reference to next level table)
                l3Table = (unint4*) theOS->getMemManager()->alloc(4*64, true, 4*64);
            } else {
                l3Table = (unint4*) (((l2Table[l2Index] & 0xFFFFFFFC)) << 4);
            }
            tableEntry = createPtd((unint4)l3Table);
            l2Table[l2Index] = tableEntry;
        }
    //}
    return addPageEntry(logBaseAddr, physBaseAddr, mappingSize, protection, zsel, cache_inhibit, l3Table);
}

unint4 Leon3HatLayer::addPageEntry(void* logBaseAddr, void* physBaseAddr, unint4 mappingSize, BitmapT protection, byte zsel, bool cache_inhibit, unint4* l3Table) {
    unint4 l3Index = ((unint4)logBaseAddr) & 0x0003F000;
    l3Index = l3Index >> 12;
    unint4 tableEntry = createPte(0xFFFFFF00 & (unint)physBaseAddr, protection, zsel, cache_inhibit);
    l3Table[l3Index] = tableEntry;
    return (unint4)physBaseAddr & (unint4)0xFFFFFF00;

}

unint4 Leon3HatLayer::getMappingLevel(size_t size) {
    /*if (size <= PAGE_SIZE) {
        return PAGE_SIZE;
    }

    if (size >= CONTEXT_SIZE) {
        return CONTEXT_SIZE;
    }

    if (size >= REGION_SIZE) {
        return REGION_SIZE;
    }

    if (size >= SEGMENT_SIZE) {
        return SEGMENT_SIZE;
    }

    if (size > PAGE_SIZE) {
        return PAGE_SIZE;
    }*/

        if (size <= PAGE_SIZE) {
            return PAGE_SIZE;
        } else if (size <= SEGMENT_SIZE) {
            return SEGMENT_SIZE;
        }else if (size <= REGION_SIZE) {
            return REGION_SIZE;
        } else if (size <= CONTEXT_SIZE) {
            //return CONTEXT_SIZE;
            return REGION_SIZE;
        } else {
            // error: size bigger than 2 gb
            return CONTEXT_SIZE;
        }

}


unint4 Leon3HatLayer::createPtd(unint4 address) {
    address = (address >> 4);
    // set entry type
    address |= 0b01;
    return address;
}


unint4 Leon3HatLayer::bitmapToACC(BitmapT protection, byte zsel) {
    int acc = 0;
    if (zsel == 0){
        switch (protection) {
            case 0: acc = -1; break;
            case 4: acc = 0; break; // TODO: This allows read in usermode
            case 1: acc = 4; break; // TODO: This allows execution in usermode
            case 3: acc = 5; break;
            case 5: acc = 6; break;
            case 7: acc = 7; break;
            default: acc = 7; // no execute only; no write and execute only
        }

    }else {
        switch (protection) {
            case 0: acc = 7; break;
            case 4: acc = 0; break;
            case 1: acc = 4; break;
            case 3: acc = 1; break;
            case 5: acc = 2; break;
            case 7: acc = 3; break;
            default: acc = 3; // no execute only; no write and execute only
        }

    }

    //return acc;
    return 3;

}

unint4 Leon3HatLayer::createPte(unint4 address, BitmapT protection, byte zsel, bool cacheable) {
    address = (address >> 4);
    if (cacheable) {
        // set cache bit
        address |= 0x80;
    }
    int acc = bitmapToACC(protection, zsel);
    if (acc == -1) {
        return 0;
    }

    // set acc
    address |= (acc<<2);
    // set entry type
    address |= 0b10;

    return address;
}


void* Leon3HatLayer::getPhysicalAddress( void* logAddr ) {
    // walk context 0 table
    int pid = 0;
    GETPID(pid);
    unsigned int pte =  contextTable[pid];
    if ((pte & 0x3) == 2){
        return (void*) ((pte & 0xFFFFFF00) & ((unsigned int)logAddr & 0xFFF));
    }

    pte = pte & 0xFFFFFFFC;
    pte = pte << 4;
    unsigned int* l1Table = ( unsigned int*)pte;
    unsigned int l1Index = ((unsigned int)0xFF000000 & (unsigned int)logAddr);
    l1Index = l1Index >> 24;
    pte = l1Table[l1Index];
    if ((pte & 0x3) == 2){
        pte = pte & 0xFFFFFF00;
        pte = pte << 4;
        unsigned int offset = ((unsigned int) logAddr) & 0x00FFFFFF;
        pte = pte | offset;
        return (void*) pte;
        //return (void*) ((pte & 0xFFFFFF00) & ((unsigned int)logAddr & 0xFFF));
    }

    pte = pte & 0xFFFFFFFC;
    pte = pte << 4;
    unsigned int* l2Table = ( unsigned int*) pte;

    unsigned int l2Index = ((unsigned int)0x00FC0000 & (unsigned int)logAddr);
    l2Index = l2Index >> 18;
    pte = l2Table[l2Index];
    if ((pte & 0x3) == 2){
        pte = pte & 0xFFFFFF00;
        pte = pte << 4;
        unsigned int offset = ((unsigned int) logAddr) & 0x3FFFF;
        pte = pte | offset;
        return (void*) pte;
    }

    pte = pte & 0xFFFFFFFC;
    pte = pte << 4;
    unsigned int* l3Table = ( unsigned int*) pte;

    unsigned int l3Index = ((unsigned int)0x0003F000 & (unsigned int)logAddr);
    l3Index = l3Index >> 12;
    pte = l3Table[l3Index];
    pte = pte & 0xFFFFFF00;
    pte = pte << 4;
    unsigned int offset = ((unsigned int) logAddr) & 0xFFFFFFFF;
    pte = pte | offset;
    return (void*) pte;

    return 0;
}

