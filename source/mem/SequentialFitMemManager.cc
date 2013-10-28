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

void SequentialFitMemManager::split( Chunk_Header* chunk, size_t size ) {

	// be sure the next address is at least 4 bytes aligned
	// chunk is aligned and SZ_HEADER is a multiple of 4 so this works
	size = (size_t) alignCeil((char*)size,4);

    //Chunk_Header *current_ch	=  (Chunk_Header*) ((unint4) chunk - sizeof(Chunk_Header));

    if ( chunk->size >= (size + sizeof(Chunk_Header) + MINIMUM_PAYLOAD) ) {
    	// split this chunk!

        // the new chunk containing the remaining unused memory of the payload is created
        Chunk_Header* next_ch 		= (Chunk_Header*) ( (unint4) chunk + size + sizeof(Chunk_Header) );

        // update the following chunk to set the prev pointer to the new chunk
        if (chunk->next_chunk != 0) {
        	chunk->next_chunk->prev_chunk = next_ch;
        }

        // Set its Header
        next_ch->next_chunk 	= chunk->next_chunk;
        next_ch->prev_chunk 	= chunk;
        next_ch->state 			= FREE;
        next_ch->size 			= chunk->size - size - sizeof(Chunk_Header);

        if (((unint4) next_ch->next_chunk <= (unint4) next_ch) && (next_ch->next_chunk != 0)) {
				//printf("Error in chunk list...");
				while(1);
        }

        chunk->next_chunk 	=  next_ch;
        chunk->state 		= OCCUPIED;
      //  chunk->size 		= ((unint4) next_ch - (unint4) chunk) - sizeof(Chunk_Header);
        chunk->size 		= size;

        if (((unint4) chunk->next_chunk <= (unint4) chunk) && ((unint4) chunk->next_chunk != 0)) {
			//printf("Error in chunk list...");
			while(1);
		}
    }
    else {
        // chunk is not splitted, only change state
        chunk->state = OCCUPIED;
    }
}

void SequentialFitMemManager::merge( Chunk_Header* chunk ) {

	//Chunk_Header *current_ch	=  (Chunk_Header*) ((unint4) chunk - sizeof(Chunk_Header));


	//printf("free: 0x%x [%d]\r",chunk,current_ch->size);
	// we are free anyway
	chunk->state = FREE;
	memset((void*) ((unint4) chunk + sizeof(Chunk_Header)), 0, ((unint4)chunk->next_chunk - (unint4)chunk) - sizeof(Chunk_Header));

	// policy: try merging it with the previous chunk first
	// if not existend merge with next if it is free
	// if that fails just mark this chunk as free
	// however, this will increase the used mem as the header stays reserved

	if (chunk->prev_chunk != 0) {
		Chunk_Header *prev_ch	=  chunk->prev_chunk;
		// change the size
		if (prev_ch->state == FREE) {

			prev_ch->next_chunk = chunk->next_chunk;
			//prev_ch->size 		+= (((unint4) chunk - current_ch->prev_chunk) - prev_ch->size) + current_ch->size;
			prev_ch->size 		+= chunk->size + sizeof(Chunk_Header);

			if (chunk->next_chunk != 0) {
				chunk->next_chunk->prev_chunk = prev_ch;
			}

			chunk = prev_ch;
		}
	}

	if (chunk->next_chunk != 0) {
		Chunk_Header *next_ch	=  chunk->next_chunk;

		// change the size
		if (next_ch->state == FREE) {

			chunk->next_chunk  = next_ch->next_chunk;
			chunk->size 	   += next_ch->size + sizeof(Chunk_Header);

			if (chunk->next_chunk != 0) {
				chunk->next_chunk->prev_chunk = chunk;
			}

		}

	}

}


bool SequentialFitMemManager::isValidChunkAddress( Chunk_Header* chunk ) {
    Chunk_Header* current = startChunk;

    while ( current != 0 ) {
        if ( chunk == current) {
            return true;
        }
        else if ( chunk < current) {
            return false;
        }

        current = current->next_chunk;
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
Chunk_Header* SequentialFitMemManager::getFittingChunk( size_t size, bool aligned, unint4 align_val ) {

#if MEM_LAST_FIT == 1
	Chunk_Header* current_chunk = lastAllocatedChunk;
#else
	Chunk_Header* current_chunk = startChunk;
#endif

	if (align_val < 4) align_val = 4;

	// always aligned
	unint4 fragmentation;
	//Chunk_Header *current_ch = 0;
	Chunk_Header* prev_chunk;

	while (current_chunk != 0) {
		// calculate fragmentation amount due to alignment
		fragmentation = ((unint4) alignCeil((char*) ((unint4) current_chunk + sizeof(Chunk_Header)),align_val))
					  - ((unint4) current_chunk + sizeof(Chunk_Header));

		// check if we can reuse this chunk
		// size must be able to handle the rearrangement of the chunk to fit its new alignment
		if ((current_chunk->state == FREE) && (current_chunk->size >= (size + fragmentation))) {
			// found a free slot which is big enough including alignment

			// return directly if we do not have to adapt the chunk to the new alignment
			if (fragmentation == 0) return current_chunk;

			// move chunk to new alignment
			unint4* newchunk 	 =  (unint4*) alignCeil((char*) ((unint4) current_chunk + sizeof(Chunk_Header)),align_val);
			Chunk_Header *new_ch =  (Chunk_Header*) ((unint4) newchunk - sizeof(Chunk_Header));

			// adopt next pointer of prev chunk in list
			if (current_chunk->prev_chunk != 0) {
				Chunk_Header *prev_ch 	= current_chunk->prev_chunk;
				prev_ch->next_chunk 	= new_ch;
				// let the previous chunk regain the fragmentation
				prev_ch->size 			+= fragmentation;
			}

			// safe temporarily
			Chunk_Header* c_prev_chunk  = current_chunk->prev_chunk;
			unint4 		  c_s 			= current_chunk->size - fragmentation;;
			Chunk_Header* c_next_ch 	= current_chunk->next_chunk;

			// set values
			new_ch->prev_chunk 	= c_prev_chunk;
			new_ch->size 		= c_s;
			new_ch->state 		= OCCUPIED;
			new_ch->next_chunk	= c_next_ch;

			if (c_next_ch != 0) {
				c_next_ch->prev_chunk		= new_ch;
			}

			if (current_chunk == startChunk)
				startChunk = new_ch;

			return new_ch;


		} // chunk fits

		// debug chunk list
		if (((unint4) current_chunk->next_chunk != 0) && ((unint4) current_chunk->next_chunk <= (unint4) current_chunk)) {
			printf("Error in chunk list...");

			// return 0 for tasks to show them allocation failed ..
			if (pCurrentRunningTask != 0)
				return 0;
			else
				while (1) {};
		}

		prev_chunk = current_chunk;
		current_chunk = current_chunk->next_chunk;

	}

	printf("Out of Memory");

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
	    startChunk = (Chunk_Header*) ( (int) Segment.getStartAddr());
	    startChunk = (Chunk_Header*) align( (byte*) startChunk, ALIGN_VAL);

#if MEM_LAST_FIT == 1
	    lastAllocatedChunk = startChunk;
#endif

	    startChunk->state = FREE;
	    startChunk->next_chunk = 0;
	    startChunk->prev_chunk = 0;
	    startChunk->size = Segment.getSize();

	}

	// put some buffer between to avoid erronous program access
	//size = size + 40;

    Chunk_Header* chunk =  getFittingChunk( size, aligned, align_val );
    void* addr = (void*) ((unint4) chunk + sizeof(Chunk_Header));

    if (( ((unint4)addr) & (align_val-1)) != 0)
    {
    	printf("Error in alignment...");
    	while(1) {};
    }

    if ( chunk != 0) {
        split( chunk, size );
        lastAllocatedChunk = chunk;
       // printf("MEM_ALLOC: %x (%d)\r",(unint4) addr, size);
        return addr;
    } else {
    	printf("No more memory left .. serious error .. halting..\r");
    	int overhead;
    	int fragmentation;
    	unint4 memused = this->getUsedMemSize(overhead,fragmentation);
    	printf("UsedMem: %d\r", memused);
    	printf("Overhead: %d\r", overhead);
    	printf("Fragmentation: %d\r", fragmentation);

    	// TODO: handle this diffrently... we dont want user tasks to be able to
    	// crash the kernel just because it allocats too much stuff inside the kernel
    	// by,e.g. reading in a huge fat directory ...

    	while (1) {};
    }

    return 0;
}

ErrorT SequentialFitMemManager::free( void* chunk ) {
   	//printf("MEM_FREE : %x\r",(unint4) chunk);

	Chunk_Header* chunk_head = (Chunk_Header*) ((unint4) chunk - sizeof(Chunk_Header));

    if ( !isValidChunkAddress( chunk_head ) ) {
    	printf("Trying to free invalid address %x\r",(unint4) chunk);
        return cNoValidChunkAddress;
    }

#if MEM_LAST_FIT == 1
    if(chunk_head == lastAllocatedChunk) {
        lastAllocatedChunk = startChunk;
    }
#endif

    // merge this chunk with its neighbor chunks
    merge( chunk_head);
    return cOk;
}


size_t SequentialFitMemManager::getUsedMemSize(int &overhead, int &free_mem) {
    size_t usedSize = 0;
    Chunk_Header* next = startChunk;

    free_mem = 0;
    overhead = 0;

    while ( next ) {
    	 overhead += sizeof(Chunk_Header);

        if ( next->state == OCCUPIED ) {
            usedSize = usedSize + ( size_t )next->size;
        }
        else {
            usedSize = usedSize + sizeof(Chunk_Header);
            free_mem += next->size;
        }

        if (((unint4) next->next_chunk <= (unint4) next) && ((unint4) next->next_chunk != 0)) {
        	printf("error in chunk list...");
        	while(1);
        }

        next = next->next_chunk;
    }

    return usedSize;
}