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

#include <PPC405HatLayer.hh>
#include "powerpc.h"
#include "ppc405.h"
//#include <debug/Logger.hh>
#include "kernel/Kernel.hh"


extern void* _text_start;
extern void* _text_end;
extern void* _data_start;
extern void* __stack;
extern Kernel* theOS;

unint8 PPC405HatLayer::TLBEntries;

PPC405HatLayer::PPC405HatLayer(  )
{

        // 1:1 mapping of ISRs and Kernel code
        this->map( (void*) &_text_start, (void*) &_text_start, (int) &_text_end - (int) &_text_start, 5, 0, 0, !ICACHE_ENABLE );
        // 1:1 mapping of Kernel Heap and Kernel Stacks
        this->map( (void*) &_data_start, (void*) &_data_start, (int) &__stack - (int) &_data_start, 3, 0, 0, !DCACHE_ENABLE );
        // 1:1 mapping of memory-mapped I/O
        this->map( (void*) MMIO_START_ADDRESS, (void*) MMIO_START_ADDRESS, MMIO_AREA_LENGTH, 3, 0, 0, true );

}

PPC405HatLayer::~PPC405HatLayer() {
    this->unmapAll(0);
}


void* PPC405HatLayer::map( void* logBaseAddr, void* physBaseAddr, size_t size, BitmapT protection, byte zsel, int pid,
        bool cache_inhibit ) {
    PPC405TlbEntry tlbe;
    int index;
    ErrorT status = 0;

    // search a free TLB entry
    status = getFreeEntry( &index );
    if ( isError(status) ) {
        return (void*) -1;
    }

    // set TID field
    tlbe.setProcID( pid );

    // set protection rights
    if ( protection & hatProtectionRead ) {
        tlbe.setValid();
    }
    if ( protection & hatProtectionWrite ) {
        tlbe.setWritable();
    }
    if ( protection & hatProtectionExecute ) {
        tlbe.setExecutable();
    }

    // set I flag
    if ( cache_inhibit )
        tlbe.setCacheInhibit();
    else
        tlbe.setNoCacheInhibit();

    // set SIZE field
    tlbe.setSize( getCorrectSizeEntry( size ) );
    // set EPN field
    tlbe.setEffectivePage( (BitmapT) logBaseAddr );
    // set RPN field
    tlbe.setRealPage( (BitmapT) physBaseAddr );

    BitmapT realPage = tlbe.getRealPage();

    // set ZSEL field
    if ( zsel == 0 ) {
        tlbe.setZoneSelect( 0xd );
    }
    else if ( zsel == 1 ) {
        tlbe.setZoneSelect( 0xc );
    }
    else if ( zsel == 2 ) {
        tlbe.setZoneSelect( 0x8 );
    }
    else if ( zsel == 3 ) {
        tlbe.setZoneSelect( 0x7 );
    }

    // write TLB entry to TLB
    PPC405MMU::writeEntry( index, &tlbe );

    // update the Bitmaps representing the free and used TLB entries
    setEntry( index );

    // size of entry bigger than maximal page size 16 MB => create more entries
    if ( size > MAX_PAGE_SIZE ) {
        // calculate next logical starting address
        void* nextLogAddr = (void*) ( tlbe.getEffectivePage() + 0x1000000 );
        // calculate next physical starting address
        void* nextPhysAddr = (void*) ( tlbe.getRealPage() + 0x1000000 );
        // create new TLB entry
        this->map( nextLogAddr, nextPhysAddr, size - MAX_PAGE_SIZE, protection, zsel , pid , cache_inhibit );
    }

    return ((void*) realPage);
}

ErrorT PPC405HatLayer::unmap( void* logBaseAddr, unint1 tid ) {
	// TODO: we currently ignore tid which is erroneous
    PPC405TlbEntry tlbe;
    int index;
    ErrorT status = 0;

    // search TLB entry with the given logical address
    status = PPC405MMU::searchEntry( (unint) logBaseAddr, index );
    if ( isError(status) ) {
        return (status);
    }

    // save entry in tlbe
    PPC405MMU::readEntry( index, &tlbe );

    // invalidate
    tlbe.setNotValid();

    status = releaseEntry( index );

    // write invalidated entry to TLB
    return (PPC405MMU::writeEntry( index, &tlbe ));
}

ErrorT PPC405HatLayer::unmapAll(unint1 tid) {
	// TODO: we currently ignore tid which is erroneous
    PPC405TlbEntry tlbe;
    int index = 0;

    // for all entries created by this instance: invalidate
    while ( TLBEntries != 0 ) {
        if ( isSet(TLBEntries, index ) ) {
            PPC405MMU::readEntry( index , &tlbe );
            tlbe.setNotValid();
            releaseEntry( index );
            PPC405MMU::writeEntry( index , &tlbe );
        }
        index++;
    }

    return cOk;
}

ErrorT PPC405HatLayer::changeProtection( void* logBaseAddr, BitmapT newProtection ) {
    PPC405TlbEntry tlbe;
    int index;
    ErrorT status = 0;

    status = PPC405MMU::searchEntry( (unint) logBaseAddr, index );
    if ( isError(status) ) {
        return status;
    }

    PPC405MMU::readEntry( index, &tlbe );

    if ( newProtection & hatProtectionRead ) {
        tlbe.setValid();
    }
    else {
        tlbe.setNotValid();
    }
    if ( newProtection & hatProtectionWrite ) {
        tlbe.setWritable();
    }
    else {
        tlbe.setNotWritable();
    }
    if ( newProtection & hatProtectionExecute ) {
        tlbe.setExecutable();
    }
    else {
        tlbe.setNotExecutable();
    }

    return PPC405MMU::writeEntry( index, &tlbe );
}

BitmapT PPC405HatLayer::getProtection( void* logBaseAddr ) {

    PPC405TlbEntry tlbe;
    int index;
    BitmapT resProtection = 0;
    ErrorT status = 0;

    status = PPC405MMU::searchEntry( (unint) logBaseAddr, index );

    if ( isOk(status) ) {
        PPC405MMU::readEntry( index, &tlbe );

        if ( tlbe.isValid() ) {
            resProtection |= hatProtectionRead;
        }
        if ( tlbe.isWritable() ) {
            resProtection |= hatProtectionWrite;
        }
        if ( tlbe.isExecutable() ) {
            resProtection |= hatProtectionExecute;
        }
    }
    return resProtection;
}

ErrorT PPC405HatLayer::clearProtection( void* logBaseAddr ) {
    return PPC405HatLayer::changeProtection( logBaseAddr, 0 );
}

ErrorT PPC405HatLayer::enableHAT() {
    void* sp; // the current stack pointer from r1
    void* old_sp; // the old stack pointer, saved in the stack frame

    // get the old stack pointer from the stack frame
    asm volatile(
            "sync;" // perform context synchronization
            "lwz %0, 0(%%r1);" // set old_sp = old stack pointer from the stack frame (which is always saved at 0(r1))
            : "=r" (old_sp)
            :
            :
    );

    // translate old stack pointer to logical address
    old_sp = this->getLogicalAddress( old_sp );

    asm volatile(
            "stw %0, 0(%%r1);" // store logical address of old stack pointer at position in stack frame
            :
            : "r" (old_sp)
            :
    );

    // get stackpointer
    asm volatile(
            "mr %0, %%r1;" // get current stackpointer
            : "=r" (sp)
            :
            :
    );

    // translate stack pointer to logical address
    sp = this->getLogicalAddress( sp );

    asm volatile(
            "mr  	1, %0;"             // replace stack pointer with its logical address
            "mfmsr  0;"                 // get content of MSR register
            "ori 	0,0, 1<<5 | 1<<4;"
            "mtmsr  0;"                 // move back to MSR register (virtual memory is now enabled!)
            "isync;"                    // perform context synchronization
            "sync;"
            :
            : "r" (sp)
            :
    );

    return cOk;
}

ErrorT PPC405HatLayer::disableHAT() {
    register void* sp;
    register void* old_sp;

    // get old stack pointer
    asm volatile(
            "sync;"             // perform context synchronization
            "lwz %0, 0(%%r1);"  // get old stack pointer in stack frame
            : "=r" (old_sp)
            :
            :
    );

    // calculate physical address of old stack pointer
    old_sp = this->getPhysicalAddress( old_sp );

    // write back the physical address of the old stack pointer
    asm volatile(
            "stw %0, 0(%%r1);" // write back physical address of old stack pinter to stack frame
            :
            : "r" (old_sp)
            :
    );
    // get current stack pointer

    asm volatile(
            "sync;"             // perform context synchronization
            "mr %0, %%r1;"      // get stack pointer
            : "=r" (sp)
            :
            :
    );

    // calculate physical address of stack pointer
    sp = this->getPhysicalAddress( sp );

    asm volatile(
            "mr  	1, %0;"         // write back physical address of stack pointer to r1
            "mfmsr  0;"             // get MSR register content
            "li 	21, 1<<5 | 1<<4 ;"      // load to r21 the corresponding bitmask to set the IR field of the MSR (100000)
            "not 	21,21;"         // negate the result: 1111 1111 1111 1111 1111 1111 1100 1111
            "and 	0,0,21;"        // clear the DR and IR field in MSR content -> old MSR content AND 1111 1111 1111 1111 1111 1111 1100 1111 -> old MSR content with IR and DR disabled
            "mtmsr  0;"             // move back to MSR register -> Virtual Memory is now disabled!
            "isync;"                // perform context synchronization
            "sync;"
            :
            : "r" (sp)
            :
    );

    return cOk;
}

void* PPC405HatLayer::getLogicalAddress( void* physAddr ) {
    int t;
    PPC405TlbEntry tlbe;
    BitmapT physBaseAddr;
    int size;
    int offset;

    // check for all entries been created by this instance:
    for ( t = 0; t < 64; t++ )
    {
        if ( isSet(TLBEntries, t ) ) {
                PPC405MMU::readEntry( t, &tlbe );
                // get physical start address of entry
                physBaseAddr = tlbe.getRealPage();
                // get size of entry
                size = 1024 << 2 * tlbe.getSize();
                // is physAddr inside of the mapped physical address space of this entry?
                if ( (BitmapT) physAddr >= physBaseAddr && (BitmapT) physAddr < physBaseAddr + size ) {
                    // get the offset between the given physical address and the starting address of this entry
                    offset = (BitmapT) physAddr - physBaseAddr;
                    // calculate the corresponding logical address
                    return ( (void*) ( tlbe.getEffectivePage() + offset ) );
                }

        }

    }


    return 0;
}

void* PPC405HatLayer::getPhysicalAddress( void* log_addr)
{
 int id = 0;
 if (isOk(PPC405MMU::searchEntry((unint)log_addr,id)))
 {
     BitmapT logBaseAddr;
     PPC405TlbEntry tlbe;
     int offset;

     PPC405MMU::readEntry( id, &tlbe );
     logBaseAddr = tlbe.getEffectivePage();
     offset = (BitmapT) log_addr - logBaseAddr;
     return ( (void*) ( tlbe.getRealPage() + offset ) );
 }
 else
     return 0;
}

void* PPC405HatLayer::getPhysicalAddress( void* logAddr, int tid ) {
    int t;
    PPC405TlbEntry tlbe;
    BitmapT logBaseAddr;
    int size;
    int offset;

    // check for all entries created by this instance:
    for ( t = 0; t < 64; t++ ) {
            if ( isSet(TLBEntries, t ) ) {
                PPC405MMU::readEntry( t, &tlbe );
                // get logical start address of entry
                logBaseAddr = tlbe.getEffectivePage();
                // get size of entry
                size = 1024 << 2 * tlbe.getSize();
                // is given logical address inside the address space of this entry?
                if ( (BitmapT) logAddr >= logBaseAddr && (BitmapT) logAddr < logBaseAddr + size && tid == tlbe.getProcID()) {
                    // get offset between given logical address and logical start address
                    offset = (BitmapT) logAddr - logBaseAddr;
                    // calculate corresponding physical address
                    return ( (void*) ( tlbe.getRealPage() + offset ) );
                }

            }

        }


    return 0;
}

ErrorT PPC405HatLayer::getFreeEntry( int* index ) {
    int t;

    for ( t = 0; t < 64; t++ ) {
            if ( ! isSet(TLBEntries, t ) ) {
                *index = t;
                return cOk;
            }
        }

    *index = -1;
    return cError;
}

ErrorT PPC405HatLayer::releaseEntry( int index ) {
        clearBit(TLBEntries, index );
        return cOk;
}

ErrorT PPC405HatLayer::setEntry( int index ) {
        setBit(TLBEntries, index );
        return cOk;
}


byte PPC405HatLayer::getCorrectSizeEntry( size_t s ) {
    size_t size = 1024;
    byte pgSize = 0;
    while ( s > size && pgSize < PPC405PageSize16Mb ) {
        pgSize++;
        size <<= 2;
    }

    return pgSize;
}

void PPC405HatLayer::handleMappingError() {
    int instruction;
    int accessTo;
    TaskIdT tid;

    asm volatile(
            // Extract the address of the instruction causing TLB Miss from SRR0
            "		mfspr  %0,26;"
            // Extract the address of the accessed data causing TLB Miss (only Data TLB Miss) from DEAR
            "		mfspr  %1,981;"
            // Extract TaskID
            "		mfspr  %2,945;"
            :  "=r" (instruction) , "=r" (accessTo), "=r" (tid)
            :
            :
    );

    LOG(ARCH,FATAL,(ARCH,FATAL,"TLB Miss occurred"));
    LOG(ARCH,FATAL,(ARCH,FATAL,"Instruction 0x%x: PID: %d DEAR: 0x%x \n",instruction,tid,accessTo));

    // could kill Task here if possible, at the moment: DO NOTHING
    while ( 1 )
        ;

}

void PPC405HatLayer::initialize() {
    TLBEntries = 0;
    PPC405MMU::invalidate();
    // setup the Zone Protection Register
}
