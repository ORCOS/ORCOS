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

#include "SequentialFitMemManager.hh"
#include "inc/memtools.hh"


#include "kernel/Kernel.hh"


extern Kernel* theOS;
extern Task* pCurrentRunningTask;

void SequentialFitMemManager::split( unint4* chunk, size_t size ) {

	// be sure the next address is at least 4 bytes aligned
	// chunk is aligned and SZ_HEADER is a multiple of 4 so this works
	size = (size_t) alignCeil((char*)size,4);

    Chunk_Header *current_ch	=  (Chunk_Header*) ((unint4) chunk - sizeof(Chunk_Header));

    if ( current_ch->size >= (size + sizeof(Chunk_Header) + MINIMUM_PAYLOAD) ) {
    	// split this chunk!

        // the new chunk containing the remaining unused memory of the payload is created
        unint4* newchunk 		= (unint4*) ( (unint4) chunk + sizeof(Chunk_Header) + size );
        Chunk_Header *next_ch 	= (Chunk_Header*) ((unint4) newchunk - sizeof(Chunk_Header));

        // Set its Header
        next_ch->next_chunk 	= current_ch->next_chunk;
        next_ch->prev_chunk 	= (unint4) chunk;
        next_ch->state 			= FREE;
        next_ch->size 			= current_ch->size - size - sizeof(Chunk_Header);

        if (((unint4) next_ch->next_chunk <= (unint4) next_ch) && (next_ch->next_chunk != 0)) {
				//printf("Error in chunk list...");
				while(1);
        }

        current_ch->next_chunk 	= (unint4) newchunk;
        current_ch->state 		= OCCUPIED;
        current_ch->size 		= ((unint4) newchunk - (unint4) chunk) - sizeof(Chunk_Header);

        if (((unint4) current_ch->next_chunk <= (unint4) current_ch) && ((unint4) current_ch->next_chunk != 0)) {
			//printf("Error in chunk list...");
			while(1);
		}
    }
    else {
        // chunk is not splitted, only change state
        current_ch->state = OCCUPIED;
    }
}

void SequentialFitMemManager::merge( unint4* chunk ) {

	Chunk_Header *current_ch	=  (Chunk_Header*) ((unint4) chunk - sizeof(Chunk_Header));


	//printf("free: 0x%x [%d]\r",chunk,current_ch->size);
	// we are free anyway
	current_ch->state = FREE;
	memset(chunk, 0, ((unint4)current_ch->next_chunk - (unint4)chunk) - sizeof(Chunk_Header));

	// try to merge

	// TODO: merge

	// policy: try merging it with the previous chunk first
	// if not existend merge with next if it is free
	// if that fails just mark this chunk as free

	/*if (current_ch->prev_chunk != 0) {
		Chunk_Header *prev_ch	=  (Chunk_Header*) (current_ch->prev_chunk - sizeof(Chunk_Header));
		// change the size
		if (prev_ch->state == FREE) {

			prev_ch->next_chunk = current_ch->next_chunk;
			prev_ch->size 		+= (((unint4) chunk - current_ch->prev_chunk) - prev_ch->size) + current_ch->size;

			if (current_ch->next_chunk != 0) {
				Chunk_Header *next_ch	=  (Chunk_Header*) (current_ch->next_chunk - sizeof(Chunk_Header));
				next_ch->prev_chunk = current_ch->prev_chunk;
			}
		}
	}*/

}


bool SequentialFitMemManager::isValidChunkAddress( unint4* chunk ) {
    unint4* current = startChunk;

    while ( current != 0 ) {
        if ( chunk == current ) {
            return true;
        }
        else if ( chunk < current ) {
            return false;
        }

        Chunk_Header *current_ch =  (Chunk_Header*) ((unint4) current - sizeof(Chunk_Header));
        current = (unint4*) current_ch->next_chunk;
    }

    return false;
}

SequentialFitMemManager::SequentialFitMemManager( void* startAddr, void* endAddr) :
    MemManager( startAddr, endAddr ) {

	// we do not write into the memory area here as we might not be running
	// in the correct task context! intialization is done on demand!
	startChunk = 0;

}

// new version
void* SequentialFitMemManager::getFittingChunk( size_t size, bool aligned, unint4 align_val ) {

#if MEM_LAST_FIT == 1
	unint4* current_chunk = lastAllocatedChunk;
#else
	unint4* current_chunk = startChunk;
#endif

	if (align_val < 4) align_val = 4;

	// always aligned
	unint4 fragmentation;
	Chunk_Header *current_ch = 0;

	while (current_chunk != 0) {
		// calculate fragmentation amount due to alignment
		fragmentation = ((unint4) alignCeil((char*)current_chunk,align_val)) - (unint4) current_chunk;
		current_ch =  (Chunk_Header*) ((unint4) current_chunk - sizeof(Chunk_Header));

		// check if we can reuse this chunk
		if ((current_ch->state == FREE) && (current_ch->size >= (size + fragmentation))) {
			// found a free slot which is big enough including alignment

			// return directly if we do not have to adapt the chunk to the new alignment
			if (fragmentation == 0) return current_chunk;

			// move chunk to new alignment
			unint4* newchunk 	 =  (unint4*) alignCeil((char*)current_chunk,align_val);
			Chunk_Header *new_ch =  (Chunk_Header*) ((unint4) newchunk - sizeof(Chunk_Header));

			// adopt next pointer of prev chunk in list
			if (current_ch->prev_chunk != 0) {
				Chunk_Header *prev_ch 	= (Chunk_Header*) ((current_ch->prev_chunk)- sizeof(Chunk_Header));
				prev_ch->next_chunk 	= (unint4) newchunk;

				if ((unint4) prev_ch->next_chunk <= (unint4) prev_ch) {
					//printf("Error in chunk list..");
					while(1);
				}
			}

			// safe temporarily
			unint4 c_prev_chunk = current_ch->prev_chunk;
			unint4 c_s 			= current_ch->size - fragmentation;;
			unint4 c_next_ch 	= current_ch->next_chunk;

			// set values
			new_ch->prev_chunk 	= c_prev_chunk;
			new_ch->size 		= c_s;
			new_ch->state 		= OCCUPIED;
			new_ch->next_chunk	= c_next_ch;

			if (c_next_ch != 0) {
				Chunk_Header *next_ch 	= (Chunk_Header*) ((c_next_ch)- sizeof(Chunk_Header));
				next_ch->prev_chunk		= (unint4) newchunk;
			}

			if (current_chunk == startChunk)
				startChunk = newchunk;

			return newchunk;


		} // chunk fits

		// debug chunk list
		if (((unint4) current_ch->next_chunk != 0) && ((unint4) current_ch->next_chunk <= (unint4) current_chunk)) {
			printf("Error in chunk list...");

			int tid = 0;
			if (pCurrentRunningTask != 0)
				tid = pCurrentRunningTask->getId();

			int asid = -1;
			unint4 tbb0 = 0;
			unint4 paget =	((unint4)(&__PageTableSec_start)) + tid*0x4000;

			asm (
				"MRC p15,0,%0,c13,c0,1;" // ; Read CP15 Context ID Register
				"MRC p15,0,%1,c2,c0,0;" // ; Read CP15 Translation Table Base Register 0
				: "=&r" (asid), "=&r" (tbb0)
				:
				:
			);

			LOG(ARCH,ERROR,(ARCH,ERROR,"ASID: %d, TBB0: 0x%x, Task PT: 0x%x",asid,tbb0,paget));

			theOS->getHatLayer()->dumpPageTable(tid);
			// TODO: Handle by removing the task !
			while(1);
		}

		current_chunk = (unint4*) current_ch->next_chunk;

	}

	// no free memory left
	return 0;
}



void* SequentialFitMemManager::alloc( size_t size, bool aligned, unint4 align_val ) {

	// initialization is done here as we are running in the right context
	// and can access the correct memory region with VM
	// this takes some performance per mem alloc, however this is much safer!
	// be sure memory sement is never starting at 0!
	// address 0 -> x shall be used to detect null pointers!
	if (startChunk == 0) {
	    startChunk = (unint4*) ( (int) Segment.getStartAddr() + sizeof(Chunk_Header) );
	    startChunk = (unint4*) align( (byte*) startChunk, ALIGN_VAL);

#if MEM_LAST_FIT == 1
	    lastAllocatedChunk = startChunk;
#endif

	    Chunk_Header *start_ch =  (Chunk_Header*) ((unint4) startChunk - sizeof(Chunk_Header));
	    start_ch->state = FREE;
	    start_ch->next_chunk = 0;
	    start_ch->prev_chunk = 0;
	    start_ch->size = Segment.getSize();

	}

	// put some buffer between to avoid erronous program access
	//size = size + 40;

    unint4* addr = (unint4*) getFittingChunk( size, aligned, align_val );

    //printf("alloc: 0x%x [%d]\r",addr,size);


    if (( ((unint4)addr) & (align_val-1)) != 0)
    {
    	printf("Error in alignment...");
    	while(1) {};
    }

    if ( addr ) {
        split( addr, size );
        return addr;
    } else {
    	printf("No more memory left .. serious error .. halting..\r");

    	unint4 memused = this->getUsedMemSize();
    	printf("UsedMem: %d\r", memused);
    	while (1) {};
    }

    return 0;
}

ErrorT SequentialFitMemManager::free( void* chunk ) {

    if ( !isValidChunkAddress( (unint4*) chunk ) ) {
    	//printf("Trying to free invalid address %x",(unint4) chunk);
        return cNoValidChunkAddress;
    }

#if MEM_LAST_FIT == 1
    if((unint4*)chunk == lastAllocatedChunk) {
        lastAllocatedChunk = startChunk;
    }
#endif

    // merge this chunk with its neighbor chunks
    merge( (unint4*) chunk );
    return cOk;
}


size_t SequentialFitMemManager::getUsedMemSize(int* fragmentation) {
    size_t usedSize = 0;
    unint4* next = startChunk;

    int frag = 0;

    while ( next ) {
    	 Chunk_Header *current_ch	=  (Chunk_Header*) ((unint4) next - sizeof(Chunk_Header));

        if ( current_ch->state == OCCUPIED ) {
            usedSize = usedSize + ( size_t )current_ch->size + SZ_HEADER;
            frag += ((current_ch->next_chunk - SZ_HEADER) - (unint4) next) - current_ch->size;
        }
        else {
        	//frag += ( size_t )current_ch->size;
            usedSize = usedSize + SZ_HEADER;
        }

        if (((unint4) current_ch->next_chunk <= (unint4) current_ch) && ((unint4) current_ch->next_chunk != 0)) {
        	printf("error in chunk list...");
        	while(1);
        }

        next = (unint4*) current_ch->next_chunk;


    }

    if (fragmentation != 0) *fragmentation = frag;
    return usedSize;
}
