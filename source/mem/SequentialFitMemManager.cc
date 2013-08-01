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

/*
void SequentialFitMemManager::merge( unint4* chunk ) {
    // the previous chunk in the list
    unint4* prev = (unint4*) PREV(chunk);
    // the next chunk in the list
    unint4* next = (unint4*) NEXT(chunk);
    unint4* temp;

    if ( prev == 0 ) {
        if ( next == 0 ) {
            // only the start chunk exists
            STATE(chunk) = FREE;
        }
        else if ( STATE(next) == OCCUPIED ) {
            // the start chunk is released
            STATE(chunk) = FREE;
        }
        else {
            // merge the start chunk with its following chunk
            STATE(chunk) = FREE;
            SIZE(chunk) = SIZE(chunk) + SZ_HEADER + SIZE(next);
            NEXT(chunk) = NEXT(next);

            temp = (unint4*) NEXT(next);
            if ( temp != 0 ) {
                PREV(temp) = (unint4) chunk;
            }
        }
    }
    else if ( next == 0 ) {
        if ( STATE(prev) == OCCUPIED ) {
            // release the last chunk in the list
            STATE(chunk) = FREE;
        }
        else {
            // merge last chunk in list with its preceding chunk
            NEXT(prev) = 0;
            SIZE(prev) = SIZE(prev) + SZ_HEADER + SIZE(chunk);
        }
    }
    else {
        if ( STATE(prev) == FREE ) {
            if ( STATE(next) == FREE ) {
                // SCENARIO: FREE - RELEASE - FREE =>    FREE
                NEXT(prev) = NEXT(next);
                SIZE(prev) = SIZE(prev) + SZ_HEADER + SIZE(chunk) + SZ_HEADER + SIZE(next);

                temp = (unint4*) NEXT(next);
                if ( temp != 0 ) {
                    PREV(temp) = (unint4) prev;
                }
            }
            else {
                // SCENARIO: FREE - RELEASE - OCCUPIED =>   FREE     OCCUPIED
                NEXT(prev) = (unint4) next;
                SIZE(prev) = SIZE(prev) + SZ_HEADER + SIZE(chunk);

                PREV(next) = (unint4) prev;
            }
        }
        else {
            if ( STATE(next) == FREE ) {
                // SCENARIO: OCCUPIED - RELEASE - FREE => OCCUPIED     FREE
                STATE(chunk) = FREE;
                SIZE(chunk) = SIZE(chunk) + SZ_HEADER + SIZE(next);
                NEXT(chunk) = NEXT(next);

                temp = (unint4*) NEXT(next);
                if ( temp != 0 ) {
                    PREV(temp) = (unint4) chunk;
                }
            }
            else {
                // SCENARIO: OCCUPIED RELEASE OCCUPIED => OCCUPIED FREE OCUUPIED
                STATE(chunk) = FREE;
            }
        }
    }
}*/

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

	unint4* current_chunk = startChunk;

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


/*
#ifdef HAS_MemoryManager_FirstFitCfd
void* SequentialFitMemManager::getFittingChunk( size_t size, bool aligned, unint4 align_val ) {
    // First Fit Policy
    unint4* current = startChunk;
    // the next aligned address of this chunk
    unint4* current_aligned = (unint4*) align( (byte*) current, ALIGN_VAL);
    unint4* prev;
    unint4* next;
    unint4 old_next;
    unint4 old_prev;
    int1 old_state;
    unint4 old_size;

    if ( aligned ) {
        // we need aligned memory
        while ( current != 0 ) {
            // iterate list
            if ( SIZE(current) >= size && STATE(current) == FREE && current == current_aligned ) {
                // the start address of this chunk is already an aligned address, chunk is free and sufficient large => chunk is found
                return (void*) current;
            }
            else if ( current != current_aligned && SIZE(current) >= size + (unint4) current_aligned - (unint4) current && STATE(current) == FREE ) {
                // chunk is free and sufficient large (even if we start the chunk at the aligned address)
                prev = (unint4*) PREV(current);
                next = (unint4*) NEXT(current);

                // we reduce this chunk for the difference of the necessary alignment, the size of the previous chunk is augmented

                // change header of previous chunk
                NEXT(prev) = (unint4) current_aligned;
                SIZE(prev) = SIZE(prev) + (unint4) current_aligned - (unint4) current;

                // change (if exists) header of following chunk
                if ( next != 0 ) {
                    PREV(next) = (unint4) current_aligned;
                }

                // shift and change header of old chunk
                old_next = NEXT(current);
                old_prev = PREV(current);
                old_state = STATE(current);
                old_size = SIZE(current);

                NEXT(current_aligned) = old_next;
                PREV(current_aligned) = old_prev;
                SIZE(current_aligned) = old_size - (unint4) current_aligned + (unint4) current;
                STATE(current_aligned) = old_state;

                return (void*) current_aligned;
            }

            current = (unint4*) NEXT(current);
            current_aligned = (unint4*) align( (byte*) current, ALIGN_VAL);
        }

        return 0;

    }
    else {
        while ( current != 0 ) {
            // iterate list
            if ( SIZE(current) >= size && STATE(current) == FREE ) {
                // free chunk of sufficient size is found
                return (void*) current;
            }

            current = (unint4*) NEXT(current);
        }

        // no free chunk with sufficient size found
        return 0;

    }
}
#endif
*/

#ifdef HAS_MemoryManager_NextFitCfd
void* SequentialFitMemManager::getFittingChunk(size_t size,bool aligned) {
    // Next Fit Policy
    unint4* current = lastAllocatedChunk;
    // the next aligned address of this chunk
    unint4* current_aligned = (unint4*)align((byte*)current,ALIGN_VAL);
    unint4* prev;
    unint4* next;
    unint4 old_next;
    unint4 old_prev;
    int1 old_state;
    unint4 old_size;

    if(aligned) {
        // we need aligned memory
        if(current == startChunk && NEXT(startChunk) == 0) {
            // only start chunk exists
            return (void*)current;
        }
        else {
            current = (unint4*) NEXT(lastAllocatedChunk);
            if(current == 0) {
                // we are at the end of the list
                current = startChunk;
            }
            current_aligned = (unint4*)align((byte*)current,ALIGN_VAL);
        }

        while(current != lastAllocatedChunk) {
            // iterate list
            if(SIZE(current) >= size && STATE(current) == FREE && current == current_aligned) {
                // the start address of this chunk is already an aligned address, chunk is free and sufficient large => chunk is found
                lastAllocatedChunk = current;
                return (void*)current;
            }
            else if(current != current_aligned && SIZE(current) >= size + (unint4)current_aligned - (unint4)current && STATE(current) == FREE) {
                // chunk is free and sufficient large (even if we start the chunk at the aligned address)
                prev = (unint4*)PREV(current);
                next = (unint4*)NEXT(current);
                // we reduce this chunk for the difference of the necessary alignment, the size of the previous chunk is augmented

                // change header of previous chunk
                NEXT(prev) = (unint4)current_aligned;
                SIZE(prev) = SIZE(prev) + (unint4)current_aligned - (unint4)current;

                // change (if exists) header of following chunk
                if(next != 0) {
                    PREV(next) = (unint4)current_aligned;
                }
                // shift and change header of old chunk
                old_next = NEXT(current);
                old_prev = PREV(current);
                old_state = STATE(current);
                old_size = SIZE(current);

                NEXT(current_aligned) = old_next;
                PREV(current_aligned) = old_prev;
                SIZE(current_aligned) = old_size - (unint4)current_aligned + (unint4)current;
                STATE(current_aligned) = old_state;

                lastAllocatedChunk = current_aligned;
                return (void*)current_aligned;
            }

            current = (unint4*) NEXT(current);
            if(current == 0) {
                // at the end of list
                current = startChunk;
            }
            current_aligned = (unint4*)align((byte*)current,ALIGN_VAL);
        }

        // back again at the lastAllocatedChunk (is it free????)
        if(SIZE(current) >= size && STATE(current) == FREE && current == current_aligned) {
            lastAllocatedChunk = current;
            return (void*)current;
        }
        else if(current != current_aligned && SIZE(current) >= size + (unint4)current_aligned - (unint4)current && STATE(current) == FREE) {
            // chunk is free and sufficient large (even if we start the chunk at the aligned address)
            prev = (unint4*)PREV(current);
            next = (unint4*)NEXT(current);

            // we reduce this chunk for the difference of the necessary alignment, the size of the previous chunk is augmented

            // change header of previous chunk
            NEXT(prev) = (unint4)current_aligned;
            SIZE(prev) = SIZE(prev) + (unint4)current_aligned - (unint4)current;

            // change (if exists) header of following chunk
            if(next != 0) {
                PREV(next) = (unint4)current_aligned;
            }
            // shift and change header of old chunk
            old_next = NEXT(current);
            old_prev = PREV(current);
            old_state = STATE(current);
            old_size = SIZE(current);

            NEXT(current_aligned) = old_next;
            PREV(current_aligned) = old_prev;
            SIZE(current_aligned) = old_size - (unint4)current_aligned + (unint4)current;
            STATE(current_aligned) = old_state;

            lastAllocatedChunk = current_aligned;
            return (void*)current_aligned;
        }
        else {
            // no free chunk of sufficient size found
            return 0;
        }
    }
    else {
        if(current == startChunk && NEXT(startChunk) == 0) {
            // only start chunk exists
            return (void*) current;
        }

        current = (unint4*) NEXT(current);
        if(current == 0) {
            // at the end of list
            current = startChunk;
        }

        while(current != lastAllocatedChunk) {
            // iterate list
            if(SIZE(current) >= size && STATE(current) == FREE) {
                // free chunk of sufficient size found
                lastAllocatedChunk = current;
                return (void*)current;
            }

            current = (unint4*) NEXT(current);
            if(current == 0) {
                // at the end of list
                current = startChunk;
            }
        }

        // back again at the last allocated chunk (is it free???)
        if(SIZE(current) >= size && STATE(current) == FREE) {
            lastAllocatedChunk = current;
            return (void*)current;
        }
        else {
            // no free chunk of sufficient size found
            return 0;
        }
    }
}
#endif

#ifdef HAS_MemoryManager_BestFitCfd
void* SequentialFitMemManager::getFittingChunk(size_t size,bool aligned, unint4 align_val) {
    // Best Fit Policy
    unint4* current = startChunk;
    // the best fitting chunk so far
    unint4* best_candidate = 0;
    // best fitting chunk start address with alignment
    unint4* best_candidate_aligned = 0;
    // size of unused memory of best fitting chunk
    size_t offset = this->getSize() + 1;
    // the next aligned address of this chunk
    unint4* current_aligned = (unint4*)align((byte*)current,align_val);
    unint4* prev;
    unint4* next;
    unint4 old_next;
    unint4 old_prev;
    int1 old_state;
    unint4 old_size;
    // difference between old address and aligned address
    unint4 diff_by_align;

    if(aligned) {
        // we need aligned memory
        while(current != 0) {

            if(current == current_aligned) {
                // the chunk start address is already aligned
                if(STATE(current) == FREE && SIZE(current) >= size) {
                    // chunk free and of sufficient size
                    if(size == SIZE(current)) {
                        // size of chunk equals requested size => cannot find better fit!
                        return (void*)current;
                    }
                    if(SIZE(current) - size < offset) {
                        // chunk is a better fitting one => choose it as best candidate so far
                        best_candidate = current;
                        best_candidate_aligned = best_candidate;
                        offset = SIZE(current) - size;
                    }
                }
            }
            else {
                // calculate difference between aligned address and old address
                diff_by_align = (unint4)current_aligned - (unint4)current;
                if(STATE(current) == FREE && SIZE(current) - diff_by_align >= size) {
                    // chunk is free and of sufficient size
                    if(SIZE(current) - size - diff_by_align < offset) {
                        // chunk is a better fitting one => choose it as best candidate so far
                        best_candidate = current;
                        best_candidate_aligned = current_aligned;
                        offset = SIZE(current) - size - diff_by_align;
                    }
                }
            }

            current = (unint4*) NEXT(current);
            current_aligned = (unint4*)align((byte*)current,align_val);
        }

        if(best_candidate == best_candidate_aligned) {
            // return best candidate (which is already aligned)
            return (void*)best_candidate;
        }
        else {
            // chunk is free and sufficient large (even if we start the chunk at the aligned address)
            prev = (unint4*)PREV(best_candidate);
            next = (unint4*)NEXT(best_candidate);

            // we reduce this chunk for the difference of the necessary alignment, the size of the previous chunk is augmented

            // change header of previous chunk
            NEXT(prev) = (unint4)best_candidate_aligned;
            SIZE(prev) = SIZE(prev) + (unint4)best_candidate_aligned - (unint4)best_candidate;

            // change (if exists) header of following chunk
            if(next != 0) {
                PREV(next) = (unint4)best_candidate_aligned;
            }
            // shift and change header of old chunk
            old_next = NEXT(best_candidate);
            old_prev = PREV(best_candidate);
            old_state = STATE(best_candidate);
            old_size = SIZE(best_candidate);

            NEXT(best_candidate_aligned) = old_next;
            PREV(best_candidate_aligned) = old_prev;
            SIZE(best_candidate_aligned) = old_size - (unint4)best_candidate_aligned + (unint4)best_candidate;
            STATE(best_candidate_aligned) = old_state;

            return (void*)best_candidate_aligned;
        }
    }
    else {
        while(current != 0) {
            if(STATE(current) == FREE && SIZE(current) >= size) {
                // free chunk of sufficient size found
                if(size == SIZE(current)) {
                    // size of chunk equals requested size => no better fit possible!
                    return (void*)current;
                }
                if(SIZE(current) - size < offset) {
                    // chunk is a better fitting one => choose as best candidate so far
                    best_candidate = current;
                    offset = SIZE(current) - size;
                }
            }

            current = (unint4*)NEXT(current);
        }

        return (void*)best_candidate;
    }
}
#endif

#ifdef HAS_MemoryManager_WorstFitCfd
void* SequentialFitMemManager::getFittingChunk(size_t size,bool aligned, unint4 align_val) {
    // Worst Fit Policy
    unint4* current = startChunk;
    // the worst fitting chunk so far
    unint4* worst_candidate = 0;
    // worst fitting chunk start address with alignment
    unint4* worst_candidate_aligned = 0;
    // size of unused memory of worst fitting chunk
    size_t offset = 0;
    // the next aligned address of this chunk
    unint4* current_aligned = (unint4*)align((byte*)current,align_val);
    unint4* prev;
    unint4* next;
    unint4 old_next;
    unint4 old_prev;
    int1 old_state;
    unint4 old_size;
    // difference between old address and aligned address
    unint4 diff_by_align;

    if(aligned) {
        // we need aligned memory
        while(current != 0) {
            // iterate list
            if(current == current_aligned) {
                // chunk start address is already aligned
                if(STATE(current) == FREE && SIZE(current) >= size) {
                    // free chunk of sufficient size
                    if(SIZE(current) - size >= offset) {
                        // chunk is worse fitting => choose as worst candidate so far
                        worst_candidate = current;
                        worst_candidate_aligned = worst_candidate;
                        offset = SIZE(current) - size;
                    }
                }
            }
            else {
                // calculate difference between aligned address and old address
                diff_by_align = (unint4)current_aligned - (unint4)current;
                if(STATE(current) == FREE && SIZE(current) - diff_by_align >= size) {
                    // free chunk of sufficient size found
                    if(SIZE(current) - size - diff_by_align >= offset) {
                        // chunk is worse fitting => choose as worst candidate so far
                        worst_candidate = current;
                        worst_candidate_aligned = current_aligned;
                        offset = SIZE(current) - size - diff_by_align;
                    }
                }
            }

            current = (unint4*) NEXT(current);
            current_aligned = (unint4*)align((byte*)current,align_val);
        }

        if(worst_candidate == worst_candidate_aligned) {
            // return worst candidate (already aligned)
            return (void*)worst_candidate;
        }
        else {
            // chunk is free and sufficient large (even if we start the chunk at the aligned address)
            prev = (unint4*)PREV(worst_candidate);
            next = (unint4*)NEXT(worst_candidate);

            // we reduce this chunk for the difference of the necessary alignment, the size of the previous chunk is augmented

            // change header of previous chunk
            NEXT(prev) = (unint4)worst_candidate_aligned;
            SIZE(prev) = SIZE(prev) + (unint4)worst_candidate_aligned - (unint4)worst_candidate;

            // change (if exists) header of following chunk
            if(next != 0) {
                PREV(next) = (unint4)worst_candidate_aligned;
            }
            // shift and change header of old chunk
            old_next = NEXT(worst_candidate);
            old_prev = PREV(worst_candidate);
            old_state = STATE(worst_candidate);
            old_size = SIZE(worst_candidate);

            NEXT(worst_candidate_aligned) = old_next;
            PREV(worst_candidate_aligned) = old_prev;
            SIZE(worst_candidate_aligned) = old_size - (unint4)worst_candidate_aligned + (unint4)worst_candidate;
            STATE(worst_candidate_aligned) = old_state;

            return (void*)worst_candidate_aligned;
        }
    }
    else {
        while(current != 0) {
            // iterate list
            if(STATE(current) == FREE && SIZE(current) >= size) {
                // free chunk of sufficient size
                if(SIZE(current) - size >= offset) {
                    // chunk is worse fitting => choose as worst candidate so far
                    worst_candidate = current;
                    offset = SIZE(current) - size;
                }
            }

            current = (unint4*)NEXT(current);
        }

        return (void*)worst_candidate;
    }
}
#endif

void* SequentialFitMemManager::alloc( size_t size, bool aligned, unint4 align_val ) {

	// initialization is done here as we are running in the right context
	// and can access the correct memory region with VM
	// this takes some performance per mem alloc, however this is much safer!
	// be sure memory sement is never starting at 0!
	// address 0 -> x shall be used to detect null pointers!
	if (startChunk == 0) {
	    startChunk = (unint4*) ( (int) Segment.getStartAddr() + sizeof(Chunk_Header) );
	    startChunk = (unint4*) align( (byte*) startChunk, ALIGN_VAL);

	#ifdef HAS_MemoryManager_NextFitCfd
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

#ifdef HAS_MemoryManager_NextFitCfd
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
