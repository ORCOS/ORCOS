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
#include "assemblerFunctions.hh"

extern Kernel* theOS;

#if MEM_TRACE
/* debug support for memory allocations
 * contains the backtrace for every memory allocation done
 * */
static ChunkTrace allocations[1024];
#endif

/*****************************************************************************
 * Method: SequentialFitMemManager::split(Chunk_Header* chunk, size_t &size, MemResource* segment)
 *
 * @description
 *  Splits a chunk into two chunks if possible.
 *
 *******************************************************************************/
void SequentialFitMemManager::split(Chunk_Header* chunk, size_t &size, MemResource* segment) {
    /*              x* 32                         y * 32          endPtrAddr
      | Chunk Head |  current payload | Chunk Head | next payload |
     */
    /* chunk is 4 bytes aligned.. payload field always 32 bytes aligned */
    intptr_t currentPayloadAddr = ((intptr_t) chunk) + sizeof(Chunk_Header);
    intptr_t nextPayloadAddr    = currentPayloadAddr + size + sizeof(Chunk_Header);
    nextPayloadAddr             = (intptr_t) alignCeil(reinterpret_cast<char*>(nextPayloadAddr), 32);
    intptr_t endPtrAddr         = currentPayloadAddr + chunk->size;

    /* check if we are allowed to split this chunk */
    if ((endPtrAddr > nextPayloadAddr) && (endPtrAddr - nextPayloadAddr) > MINIMUM_PAYLOAD + sizeof(Chunk_Header)) {
        /* split this chunk!
         * the new chunk containing the remaining unused memory of the payload is created */
        Chunk_Header* next_ch = reinterpret_cast<Chunk_Header*>(nextPayloadAddr - sizeof(Chunk_Header));

        /* update the following chunk to set the prev pointer to the new chunk */
        if (chunk->next_chunk != 0) {
            chunk->next_chunk->prev_chunk = next_ch;
        }

        /* Set its Header */
        next_ch->next_chunk = chunk->next_chunk;
        next_ch->prev_chunk = chunk;
        next_ch->state      = FREE;
        next_ch->size       = endPtrAddr - nextPayloadAddr;

        chunk->next_chunk   = next_ch;
        chunk->state        = OCCUPIED;
        chunk->size         = ((intptr_t) next_ch) - currentPayloadAddr;
        size                = chunk->size;

        segment->usedBytes += chunk->size;
        /* add the new chunk header as overhead .. */
        segment->overheadBytes += sizeof(Chunk_Header);
    } else {
        /* chunk is not splitted, only change state */
        chunk->state = OCCUPIED;
        segment->usedBytes += chunk->size;
        size = chunk->size;
    }
}

/*****************************************************************************
 * Method: SequentialFitMemManager::merge(Chunk_Header* chunk, MemResource* segment)
 *
 * @description
 *  Tries to merge a given chunk with its free neighbors to create
 *  bigger fee memory areas for future assignments.
 *
 * @params
 *  chunk       The free chunk to be merged
 *  segement    Segement the chunk belongs to
 *
 *******************************************************************************/
void SequentialFitMemManager::merge(Chunk_Header* chunk, MemResource* segment) {
    chunk->state = FREE;
    segment->usedBytes -= chunk->size;

    return;

    /* policy: try merging it with the previous chunk first
       if not existing merge with next if it is free
       if that fails just mark this chunk as free
       however, this will increase the used mem as the header stays reserved */

    if (chunk->prev_chunk != 0) {
        Chunk_Header *prev_ch = chunk->prev_chunk;
        /* change the size */
        if (prev_ch->state == FREE) {
            prev_ch->next_chunk = chunk->next_chunk;
            prev_ch->size += chunk->size + sizeof(Chunk_Header);

            if (chunk->next_chunk != 0) {
                chunk->next_chunk->prev_chunk = prev_ch;
            }

            chunk = prev_ch;
            segment->overheadBytes -= sizeof(Chunk_Header);
        }
    }

    if (chunk->next_chunk != 0) {
        Chunk_Header *next_ch = chunk->next_chunk;

        // change the size
        if (next_ch->state == FREE) {
            chunk->next_chunk = next_ch->next_chunk;
            chunk->size += next_ch->size + sizeof(Chunk_Header);

            if (chunk->next_chunk != 0) {
                chunk->next_chunk->prev_chunk = chunk;
            }
            segment->overheadBytes -= sizeof(Chunk_Header);
        }
    }
}

/*****************************************************************************
 * Method: SequentialFitMemManager::isValidChunkAddress(Chunk_Header* chunk,
 *                                                      MemResource* &segment)
 *
 * @description
 *  Checks a given memory chunk on validity inside a given segment.
 *
 * @params
 *  chunk       The chunk to be checked.
 *  segment     The segment the chunked is supposed to be found inside.
 *
 * @returns
 *  bool        True if the chunk is a valid element of the segment
 *******************************************************************************/
bool SequentialFitMemManager::isValidChunkAddress(Chunk_Header* chunk, MemResource* &segment) {
    segment = 0;

    if (Segment.containsAddr(chunk))
        segment = &Segment;
#if MEM_CACHE_INHIBIT
    if (Segment_Cache_Inhibit.containsAddr(chunk))
        segment = &Segment_Cache_Inhibit;
#endif

    if (segment == 0) {
        /* happens often for strings contained inside ro data !*/
        LOG(MEM, DEBUG, "Invalid address %x. Not contained in any segment.", chunk);
        return (false);
    }

#if PRECISE_VALIDITY_CHECK
    Chunk_Header* current = reinterpret_cast<Chunk_Header*>(segment->firstAddr);

    /* scan whole list to see if this chunk is correctly linked */
    while (current != 0) {
        if (chunk == current) {
            return (true);
        } else if (chunk < current) {
            LOG(MEM, WARN, "Chunk not found at current location:");
            printChunk(current);
            printChunk(chunk);
            return (false);
        }

        current = current->next_chunk;
    }
#else
    /* simply test prev chunk */
    if (chunk->prev_chunk != 0) {
        return (chunk->prev_chunk->next_chunk == chunk);
    } else {
        return (startChunk == chunk);
    }
#endif

    LOG(MEM, WARN, "Address not found inside chunk list");
    LOG(MEM, WARN, "Segment: %x - %x", segment->getStartAddr(), segment->getEndAddr());

    return (false);
}

SequentialFitMemManager::SequentialFitMemManager(void* startAddr, void* endAddr) :
        MemManager(startAddr, endAddr) {
}

#if MEM_CACHE_INHIBIT
SequentialFitMemManager::SequentialFitMemManager(void* startAddr, void* endAddr, void* istartAddr, void* iendAddr) :
        MemManager(startAddr, endAddr, istartAddr, iendAddr) {
#if MEM_TRACE
    for (int i = 0; i < 1024; i++)
    allocations[i].used = 0;
#endif
}
#endif


/*****************************************************************************
 * Method: SequentialFitMemManager::getFittingChunk(size_t size,
 *                                                  bool aligned,
 *                                                  unint4 align_val,
 *                                                  Chunk_Header* current_chunk,
 *                                                  MemResource* segment)
 *
 * @description
 *  Tries to find a free memory chunk inside the given segment.
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
Chunk_Header* SequentialFitMemManager::getFittingChunk(size_t size,
                                                       bool aligned,
                                                       unint4 align_val,
                                                       Chunk_Header* current_chunk,
                                                       MemResource* segment) {
    /*
     * By enabling / disabling IRQS again all the time we allow low latencies
     */
    while (current_chunk != 0) {
        DISABLE_IRQS(status);
        /* check if we can reuse this chunk
          size must be able to handle the rearrangement of the chunk to fit its new alignment */
        if ((current_chunk->state == FREE) && (current_chunk->size >= size)) {
            current_chunk->state = OCCUPIED;
            /* found a free slot which is big enough */
            return (current_chunk);
        }  /* chunk fits */
        RESTORE_IRQS(status);
        current_chunk = current_chunk->next_chunk;
    }


    /* no free memory left */
    return (0);
}

/*****************************************************************************
 * Method: SequentialFitMemManager::alloc(size_t size, bool aligned, unint4 align_val, MemResource* segment)
 *
 * @description
 *  Tries to allocate a new memory chunk of given size inside the given segment.
 *
 * @params
 *
 * @returns
 *  void*         The assigned memory address. 0 on error
 *******************************************************************************/
void* SequentialFitMemManager::alloc(size_t size, bool aligned, unint4 align_val, MemResource* segment) {
    /* initialization is done here as we are running in the right context
       and can access the correct memory region with VM
       this takes some performance per mem alloc, however this is much safer!
       be sure memory segment is never starting at 0!
       address 0 -> x shall be used to detect null pointers! */

    if (align_val > 32 || (align_val & 1))
        LOG(MEM, ERROR, " SequentialFitMemManager::alloc() align: %d not supported", align_val);

    if (segment == 0)
        segment = &Segment;

    Chunk_Header* startChunk = reinterpret_cast<Chunk_Header*>(segment->firstAddr);

    if (segment->lastAllocated == 0) {
        startChunk              = reinterpret_cast<Chunk_Header*>((size_t) segment->getStartAddr() + 32 - sizeof(Chunk_Header));
        segment->lastAllocated  = startChunk;
        segment->firstFreed     = reinterpret_cast<void*>(-1);
        segment->firstAddr      = startChunk;
        startChunk->state       = FREE;
        startChunk->next_chunk  = 0;
        startChunk->prev_chunk  = 0;
        startChunk->size        = segment->getSize() - 32;
    }

    /* put some buffer between to avoid erroneous program access */
    size = size + SAFETY_BUFFER;

    int irqstatus;
    GET_INTERRUPT_ENABLE_BIT(irqstatus);

#if MEM_LAST_FIT == 1

    Chunk_Header* lookupStart = reinterpret_cast<Chunk_Header*>(segment->lastAllocated);
    if ( segment->firstFreed < segment->lastAllocated )
        lookupStart = reinterpret_cast<Chunk_Header*>(segment->firstFreed);

    Chunk_Header* chunk = getFittingChunk(size,
            aligned,
            align_val,
            lookupStart,
            segment);
#else
    Chunk_Header* chunk = getFittingChunk(size, aligned, align_val, startChunk, segment);
#endif

    intptr_t addr = (((intptr_t) chunk) + sizeof(Chunk_Header));

    /* check alignment! */
    ASSERT(!(((addr) & (align_val - 1)) != 0));

    if (chunk != 0) {
        split(chunk, size, segment);
        segment->lastAllocated = chunk;
        RESTORE_IRQS(irqstatus);
        /* fill allocated memory with zeros */
        memset(reinterpret_cast<void*>(addr), 0, size);

#if CHUNK_ACCESS_CHECK
        intptr_t a = (intptr_t) (chunk + 1);
        a += size;
        a -= SAFETY_BUFFER;
        for (int i = 0; i < SAFETY_BUFFER/4; i++) {
            (reinterpret_cast<int*>(a))[i] = SAFETY_CHAR;
        }
#endif

        LOG(MEM, TRACE, " SequentialFitMemManager::alloc() addr: 0x%x (%d)", (unint4) addr, size);

#if MEM_TRACE
        for (int i = 0; i < 1024; i++) {
            if (allocations[i].used == 0) {
                allocations[i].used = 1;
                allocations[i].chunk = chunk;
                allocations[i].malloc_size = size;
                backtrace(reinterpret_cast<void**>(&allocations[i].trace), 6);
                break;
            }
        }
#endif
        return (reinterpret_cast<void*>(addr));
    } else {
        RESTORE_IRQS(irqstatus);
        LOG(MEM, ERROR, "SequentialFitMemManager::alloc(%d) No more free memory...", size);
        LOG(MEM, ERROR, "UsedMem: %d", segment->usedBytes);
        LOG(MEM, ERROR, "Overhead: %d", segment->overheadBytes);
        return (0);
    }

    return (0);
}

#if MEM_CACHE_INHIBIT
/*****************************************************************************
 * Method: SequentialFitMemManager::alloci(size_t size, bool aligned, unint4 align_val)
 *
 * @description
 *  Tries to allocate a new memory chunk of given size inside the cache inhibit
 *  memory segment.
 *
 * @params
 *
 * @returns
 *  void*         The assigned memory address. 0 on error
 *******************************************************************************/
void* SequentialFitMemManager::alloci(size_t size, bool aligned, unint4 align_val) {
    if (this->Segment_Cache_Inhibit.getStartAddr() == 0)
        return (alloc(size, aligned, align_val));

    return (alloc(size, aligned, align_val, &Segment_Cache_Inhibit));
}
#endif

/*****************************************************************************
 * Method: SequentialFitMemManager::free(void* chunk)
 *
 * @description
 *  Frees a given chunk and makes it available for future allocations again.
 *
 * @params
 *  chunk       The chunk to be freed.
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT SequentialFitMemManager::free(void* chunk) {
    LOG(MEM, DEBUG, "SequentialFitMemManager::free() chunk: %x", (unint4) chunk);

    Chunk_Header* chunk_head = reinterpret_cast<Chunk_Header*>(((intptr_t) chunk) - sizeof(Chunk_Header));
    MemResource* segment;

    if (!isValidChunkAddress(chunk_head, segment)) {
        LOG(MEM, DEBUG, "SequentialFitMemManager::free() Trying to free invalid address %x", (unint4) chunk);
        /* show call stack to identify the component that is causing this bug*/
        if (static_cast<int>(DEBUG) <= static_cast<int>(MEM))
            backtrace_current();
        return (cNoValidChunkAddress );
    }

#if MEM_TRACE
    for (int i = 0; i < 1024; i++) {
        if (allocations[i].chunk == chunk) {
            allocations[i].used = 0;
            break;
        }
    }
#endif

#if MEM_LAST_FIT == 1
    if (chunk_head == segment->lastAllocated) {
        /* freed chunk was the last one allocated.. reset lookup to start
         * this is slower, however ensures we are not endlessly appending and creating
         * holes that can not be filled afterwards */
        Chunk_Header* startChunk = reinterpret_cast<Chunk_Header*>(segment->firstAddr);
        segment->lastAllocated = startChunk;
    }

    if (chunk_head < segment->firstFreed) {
        segment->firstFreed = chunk_head;
    }

#endif

    /* merge this chunk with its neighbor chunks */
    merge(chunk_head, segment);
    return (cOk );
}

/*****************************************************************************
 * Method: SequentialFitMemManager::getUsedMemSize(size_t &u_overhead, size_t &u_free_mem)
 *
 * @description
 *  Returns the used memory size, overhead and free memory size.
 *
 * @params
 *
 * @returns
 *  int        The used memory size
 *******************************************************************************/
size_t SequentialFitMemManager::getUsedMemSize(size_t &u_overhead, size_t &u_free_mem) {
    size_t used = 0;

    Chunk_Header* current = reinterpret_cast<Chunk_Header*>(Segment.firstAddr);

    /* scan whole list to see if this chunk is correctly linked */
    while (current != 0) {
        if (current->state == OCCUPIED) {
            used += current->size;
            u_overhead += sizeof(Chunk_Header);
        }
        current = current->next_chunk;
    }

    u_free_mem = Segment.getSize() - used;

    if (Segment.usedBytes != used) {
        LOG(MEM, ERROR, "SequentialFitMemManager::getUsedMemSize() used mem %d != calculated %d", Segment.usedBytes, used);
    }

    return (used);
}

/*****************************************************************************
 * Method: SequentialFitMemManager::printChunk(Chunk_Header* chunk)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
void SequentialFitMemManager::printChunk(Chunk_Header* chunk) {
    LOG(MEM, ERROR, "%8x: | %8x | %8x | %u | %d |", (unint4) chunk, chunk->prev_chunk, chunk->next_chunk, chunk->size, chunk->state);
}

/*****************************************************************************
 * Method: SequentialFitMemManager::debug(MemResource* segment)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
void SequentialFitMemManager::debug(MemResource* segment) {
    Chunk_Header* current    = reinterpret_cast<Chunk_Header*>(segment->firstAddr);
    Chunk_Header* prev       = reinterpret_cast<Chunk_Header*>(segment->firstAddr);
    Chunk_Header* startChunk = current;

    if (current->prev_chunk != 0)
        LOG(MEM, ERROR, "SequentialFitMemManager::debug() start chunk incorrect");

    // scan whole list to see if this chunk is correctly linked
    while (current != 0) {
        if (current->next_chunk && current->next_chunk->prev_chunk != current) {
            LOG(MEM, ERROR, "SequentialFitMemManager::debug() chunk link error %x", (unint4) current);
            printChunk(prev);
            printChunk(current);
            printChunk(current->next_chunk);

#if MEM_TRACE
            for (int i = 0; i < 1024; i++) {
                if (allocations[i].chunk == current) {
                    puts("current backtrace:"LINEFEED);
                    for (int j = 0; j < 6; j++) {
                        printf("%x %s"LINEFEED, allocations[i].trace[j], getMethodSignature((unint4) allocations[i].trace[j]));
                    }
                }
                if (allocations[i].chunk == current->next_chunk) {
                    puts("next backtrace:"LINEFEED);
                    for (int j = 0; j < 6; j++) {
                        printf("%x %s"LINEFEED, allocations[i].trace[j], getMethodSignature((unint4) allocations[i].trace[j]));
                    }
                }
            }
#endif
        }

        if (current->next_chunk) {
            // check sizes
            if ((((intptr_t) current) + current->size + sizeof(Chunk_Header)) != ((intptr_t) current->next_chunk)) {
                LOG(MEM, ERROR, "SequentialFitMemManager::debug() chunk size wrong  %u", (unint4) current->size);
                printChunk(prev);
                printChunk(current);
                printChunk(current->next_chunk);
            }

#if CHUNK_ACCESS_CHECK
            intptr_t addr = (intptr_t) (current + 1);
            addr += current->size;
            addr -= SAFETY_BUFFER;
            for (int i = 0; i < SAFETY_BUFFER/4; i++) {
                if ((reinterpret_cast<int*>(addr))[i] != SAFETY_CHAR) {
                    LOG(MEM, ERROR, "SequentialFitMemManager::debug() chunk border overwritten");
                    printChunk(current);
                    memdump(&((reinterpret_cast<int*>(addr))[i]), 1);

#if MEM_TRACE
                    for (int i = 0; i < 1024; i++) {
                        if (allocations[i].chunk == current) {
                            for (int j = 0; j < 6; j++) {
                                printf("%x %s\r", allocations[i].trace[j], getMethodSignature((unint4) allocations[i].trace[j]));
                            }
                            break;
                        }
                    }
#endif
                }
            }
#endif
        }

        if (current->next_chunk == 0) {
            // check last size
            unint4 computed_size = (unint4) current - (unint4) startChunk;
            computed_size += current->size + sizeof(Chunk_Header);

            unint4 total_size = (size_t) segment->getSize() - ((intptr_t) segment->firstAddr - (intptr_t) (segment->getStartAddr()));

            if (total_size != computed_size) {
                LOG(MEM, ERROR, "SequentialFitMemManager::debug() last chunk has wrong size: %u", (unint4) current->size);
                LOG(MEM, ERROR, "SequentialFitMemManager::debug() total size != computed size : %u != %u", total_size, computed_size);
                printChunk(current);

                current->size += total_size - computed_size;
            }
        }

        prev = current;
        current = current->next_chunk;
    }
}

/*****************************************************************************
 * Method: SequentialFitMemManager::service()
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
void SequentialFitMemManager::service() {
    debug(&Segment);
#if MEM_CACHE_INHIBIT
    debug(&Segment_Cache_Inhibit);
#endif
}
