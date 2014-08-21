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

#ifdef MEM_TRACE
/* debug support for memory allocations
 * contains the backtrace for every memory allocation done
 * */
static ChunkTrace allocations[1024];
#endif

void SequentialFitMemManager::split(Chunk_Header* chunk, size_t &size, MemResource* segment) {

     segment->usedBytes += size;

    if (chunk->size >= (size + sizeof(Chunk_Header) + MINIMUM_PAYLOAD))
    {
        // split this chunk!
        // the new chunk containing the remaining unused memory of the payload is created
        Chunk_Header* next_ch = (Chunk_Header*) (((intptr_t) chunk) + size + sizeof(Chunk_Header));

        // update the following chunk to set the prev pointer to the new chunk
        if (chunk->next_chunk != 0)
        {
            chunk->next_chunk->prev_chunk = next_ch;
        }

        // Set its Header
        next_ch->next_chunk = chunk->next_chunk;
        next_ch->prev_chunk = chunk;
        next_ch->state      = FREE;
        next_ch->size       = chunk->size - (size + sizeof(Chunk_Header));

        chunk->next_chunk   = next_ch;
        chunk->state        = OCCUPIED;
        chunk->size         = size;

        ASSERT(!(((intptr_t ) chunk->next_chunk <= (intptr_t ) chunk) && ((intptr_t ) chunk->next_chunk != 0)));

        // add the new chunk header as overhead ..
        segment->overheadBytes += sizeof(Chunk_Header);
    }
    else
    {
        // chunk is not splitted, only change state
        chunk->state = OCCUPIED;
    }
}

void SequentialFitMemManager::merge(Chunk_Header* chunk, MemResource* segment) {

    chunk->state        = FREE;
    segment->usedBytes -= chunk->size;

    // policy: try merging it with the previous chunk first
    // if not existing merge with next if it is free
    // if that fails just mark this chunk as free
    // however, this will increase the used mem as the header stays reserved

    if (chunk->prev_chunk != 0)
    {
        Chunk_Header *prev_ch = chunk->prev_chunk;
        // change the size
        if (prev_ch->state == FREE)
        {

            prev_ch->next_chunk = chunk->next_chunk;
            prev_ch->size += chunk->size + sizeof(Chunk_Header);

            if (chunk->next_chunk != 0)
            {
                chunk->next_chunk->prev_chunk = prev_ch;
            }

            chunk = prev_ch;
            segment->overheadBytes -= sizeof(Chunk_Header);
        }
    }

    if (chunk->next_chunk != 0)
    {
        Chunk_Header *next_ch = chunk->next_chunk;

        // change the size
        if (next_ch->state == FREE)
        {

            chunk->next_chunk = next_ch->next_chunk;
            chunk->size += next_ch->size + sizeof(Chunk_Header);

            if (chunk->next_chunk != 0)
            {
                chunk->next_chunk->prev_chunk = chunk;
            }
            segment->overheadBytes -= sizeof(Chunk_Header);
        }

    }



}

bool __attribute((noinline)) SequentialFitMemManager::isValidChunkAddress(Chunk_Header* chunk, MemResource* &segment) {
    segment = 0;

    if (Segment.containsAddr(chunk))
        segment = &Segment;
#if MEM_CACHE_INHIBIT
    if (Segment_Cache_Inhibit.containsAddr(chunk))
        segment = &Segment_Cache_Inhibit;
#endif

    if (segment == 0) {
        LOG(MEM,WARN,"Invalid address %x. Not contained in any segment.",chunk);
        return (false);
    }

#if PRECISE_VALIDITY_CHECK
    Chunk_Header* current = (Chunk_Header*) segment->firstAddr;

    // scan whole list to see if this chunk is correctly linked
    while ( current != 0 )
    {
        if ( chunk == current)
        {
            return (true);
        }
        else if ( chunk < current)
        {
            LOG(MEM,WARN,"Chunk not found at current location:");
            printChunk(current);
            printChunk(chunk);
            return (false);
        }

        current = current->next_chunk;
    }
#else
    // simply test prev chunk
    if (chunk->prev_chunk != 0)
        return (chunk->prev_chunk->next_chunk == chunk);
    else
        return (startChunk == chunk);
#endif

    LOG(MEM,WARN,"Address not found inside chunk list");
    LOG(MEM,WARN,"Segment: %x - %x",segment->getStartAddr(),segment->getEndAddr());

    return (false);
}

SequentialFitMemManager::SequentialFitMemManager(void* startAddr, void* endAddr) :
        MemManager(startAddr, endAddr) {

}

#if MEM_CACHE_INHIBIT
SequentialFitMemManager::SequentialFitMemManager(void* startAddr, void* endAddr, void* istartAddr, void* iendAddr) :
        MemManager(startAddr, endAddr, istartAddr, iendAddr) {

#ifdef MEM_TRACE
    for (int i = 0; i < 1024; i++)
        allocations[i].used = 0;
#endif

}
#endif

// new version
Chunk_Header* SequentialFitMemManager::getFittingChunk(size_t size, bool aligned, unint4 align_val, Chunk_Header* current_chunk, MemResource* segment) {

    if (align_val < 4)
        align_val = 4;

    // always aligned
    unint4 alignment_offset;

    while (current_chunk != 0)
    {
        // calculate fragmentation amount due to alignment
        alignment_offset =   ((intptr_t) alignCeil((char*) ( ((intptr_t) current_chunk) + sizeof(Chunk_Header)), align_val))
                         - (((intptr_t) current_chunk) + sizeof(Chunk_Header));

        // check if we can reuse this chunk
        // size must be able to handle the rearrangement of the chunk to fit its new alignment
        if ((current_chunk->state == FREE)  && (current_chunk->size >= (size + alignment_offset)))
        {
            // found a free slot which is big enough including alignment
            // return directly if we do not have to adapt the chunk to the new alignment
            if (alignment_offset == 0)
                return (current_chunk);

            /* move chunk to new alignment */
            intptr_t newchunk       = (intptr_t) alignCeil((char*) (((intptr_t) current_chunk) + sizeof(Chunk_Header)), align_val);
            Chunk_Header *new_ch    = (Chunk_Header*) (newchunk - sizeof(Chunk_Header));

            /* adopt next pointer of prev chunk in list */
            if (current_chunk->prev_chunk != 0)
            {
                Chunk_Header *prev_ch = current_chunk->prev_chunk;
                prev_ch->next_chunk   = new_ch;
                /* let the previous chunk regain the fragmentation */
                prev_ch->size += alignment_offset;
                if (prev_ch->state == OCCUPIED) {
                    segment->usedBytes += alignment_offset;
                }
            }

            /* get previous values */
            Chunk_Header* c_prev_chunk = current_chunk->prev_chunk;
            Chunk_Header* c_next_ch    = current_chunk->next_chunk;
            /* subtract alignment offset from the size of the current chunk */
            unint4 c_s                 = current_chunk->size - alignment_offset;

            /* copy to new location (may overlap, thus they are saved above) */
            new_ch->prev_chunk  = c_prev_chunk;
            new_ch->size        = c_s;
            new_ch->state       = OCCUPIED;
            new_ch->next_chunk  = c_next_ch;

            if (c_next_ch != 0)
            {
                c_next_ch->prev_chunk = new_ch;
            }

            /* if this was the fist chunk update the reference to it to not break the chain*/
            if (current_chunk == segment->firstAddr) {
                new_ch->size -= ((intptr_t) new_ch - (intptr_t) segment->firstAddr);
                segment->firstAddr = new_ch;
            }

#if CHUNK_ACCESS_CHECK
            Chunk_Header *prev_ch = current_chunk->prev_chunk;
            if (prev_ch != 0 && prev_ch->state == OCCUPIED) {
                intptr_t a = (intptr_t) (prev_ch + 1);
                a += prev_ch->size;
                a -= SAFETY_BUFFER;
                for (int i = 0; i < SAFETY_BUFFER/4; i++) {
                    ((int*)a)[i] = SAFETY_CHAR;
                }
            }
#endif
            return (new_ch);

        }  // chunk fits

        current_chunk = current_chunk->next_chunk;

    }

    // no free memory left
    return (0);
}

void* SequentialFitMemManager::alloc(size_t size, bool aligned, unint4 align_val, MemResource* segment) {

    // initialization is done here as we are running in the right context
    // and can access the correct memory region with VM
    // this takes some performance per mem alloc, however this is much safer!
    // be sure memory segment is never starting at 0!
    // address 0 -> x shall be used to detect null pointers!

    if (segment == 0)
        segment = &Segment;

    Chunk_Header* startChunk = (Chunk_Header*) segment->firstAddr;

    if (segment->lastAllocated == 0)
    {
        startChunk =  (Chunk_Header*) segment->getStartAddr();
        segment->lastAllocated  = startChunk;
        segment->firstFreed     = (void*) -1;
        segment->firstAddr      = startChunk;
        startChunk->state       = FREE;
        startChunk->next_chunk  = 0;
        startChunk->prev_chunk  = 0;
        startChunk->size        = segment->getSize() - sizeof(Chunk_Header);
    }

    // chunk is aligned and SZ_HEADER is a multiple of 4 so this works
    size = (size_t) alignCeil((char*) size, 4);

    /* put some buffer between to avoid erroneous program access */
    size = size + SAFETY_BUFFER;

#if MEM_LAST_FIT == 1

    Chunk_Header* lookupStart = (Chunk_Header*) segment->lastAllocated;
    if ( segment->firstFreed < segment->lastAllocated )
        lookupStart = (Chunk_Header*) segment->firstFreed;

    Chunk_Header* chunk = getFittingChunk(size,
                                          aligned,
                                          align_val,
                                          lookupStart,
                                          segment);
#else
    Chunk_Header* chunk = getFittingChunk( size,
                                           aligned,
                                           align_val,
                                           startChunk,
                                           segment);
#endif


    void* addr = (void*) (((intptr_t) chunk) + sizeof(Chunk_Header));

    ASSERT(!((((intptr_t )addr) & (align_val - 1)) != 0));

    if (chunk != 0)
    {
        split(chunk, size,segment);
        segment->lastAllocated = chunk;
        memset(addr, 0, size);

#if CHUNK_ACCESS_CHECK
        intptr_t a = (intptr_t) (chunk + 1);
        a += size;
        a -= SAFETY_BUFFER;
        for (int i = 0; i < SAFETY_BUFFER/4; i++) {
            ((int*)a)[i] = SAFETY_CHAR;
        }
#endif

        LOG(MEM, TRACE, " SequentialFitMemManager::alloc() addr: 0x%x (%d)",(unint4) addr, size);

#ifdef MEM_TRACE
        for (int i = 0; i < 1024; i++) {
            if (allocations[i].used == 0) {
                allocations[i].used = 1;
                allocations[i].chunk = chunk;
                allocations[i].malloc_size = size;
                backtrace((void**) &allocations[i].trace,6);
                break;
            }
        }
#endif

        return (addr);
    }
    else
    {
        LOG(MEM, ERROR, "SequentialFitMemManager::alloc(%d) No more free memory...",size);
        LOG(MEM, ERROR, "UsedMem: %d", segment->usedBytes);
        LOG(MEM, ERROR, "Overhead: %d", segment->overheadBytes);
        return (0);
    }

    return (0);
}

#if MEM_CACHE_INHIBIT
void* SequentialFitMemManager::alloci(size_t size, bool aligned, unint4 align_val) {
    if (this->Segment_Cache_Inhibit.getStartAddr() == 0)
        return (alloc(size, aligned, align_val));

    return (alloc(size,aligned,align_val,&Segment_Cache_Inhibit));
}
#endif


ErrorT SequentialFitMemManager::free(void* chunk) {
    LOG(MEM, DEBUG, "SequentialFitMemManager::free() chunk: %x",(unint4) chunk);

    Chunk_Header* chunk_head = (Chunk_Header*) (((intptr_t) chunk) - sizeof(Chunk_Header));
    MemResource* segment;

    if (!isValidChunkAddress(chunk_head,segment))
    {
        LOG(MEM, WARN, "SequentialFitMemManager::free() Trying to free invalid address %x",(unint4) chunk);
        /* show call stack to identify the component that is causing this bug*/
        backtrace_current();
        return (cNoValidChunkAddress );
    }

#ifdef MEM_TRACE
    for (int i = 0; i < 1024; i++) {
         if (allocations[i].chunk == chunk) {
             allocations[i].used = 0;
             break;
         }
     }
#endif

#if MEM_LAST_FIT == 1
    if (chunk_head == segment->lastAllocated)
    {
        /* freed chunk was the last one allocated.. reset lookup to start
         * this is slower, however ensures we are not endlessly appending and creating
         * holes that can not be filled afterwards */
        Chunk_Header* startChunk = (Chunk_Header*) segment->firstAddr;
        segment->lastAllocated = startChunk;
    }
    if (chunk_head < segment->firstFreed) {
        segment->firstFreed = chunk_head;
    }

#endif

    /* merge this chunk with its neighbor chunks */
    merge(chunk_head,segment);
    return (cOk );
}

size_t SequentialFitMemManager::getUsedMemSize(size_t &u_overhead, size_t &u_free_mem) {
    size_t used = 0;

    Chunk_Header* current    = (Chunk_Header*) Segment.firstAddr;

    // scan whole list to see if this chunk is correctly linked
     while ( current != 0 ) {
         if (current->state == OCCUPIED) {
             used += current->size;
             u_overhead += sizeof(Chunk_Header);
         }
         current = current->next_chunk;
     }

     u_free_mem = Segment.getSize() - used;

     if (Segment.usedBytes != used) {
         LOG(MEM, ERROR, "SequentialFitMemManager::getUsedMemSize() used mem %d != calculated %d",Segment.usedBytes,used);
     }

     return (used);

}


void SequentialFitMemManager::printChunk(Chunk_Header* chunk) {
    LOG(MEM, ERROR, "%8x: | %8x | %8x | %u | %d |",(unint4) chunk,chunk->prev_chunk,chunk->next_chunk,chunk->size,chunk->state);
}


void SequentialFitMemManager::debug(MemResource* segment) {
    Chunk_Header* current    = (Chunk_Header*) segment->firstAddr;
    Chunk_Header* startChunk = current;

      if (current->prev_chunk != 0)
          LOG(MEM, ERROR, "SequentialFitMemManager::debug() start chunk incorrect");

     // scan whole list to see if this chunk is correctly linked
      while ( current != 0 )
      {
         if (current->next_chunk && current->next_chunk->prev_chunk != current) {
             LOG(MEM, ERROR, "SequentialFitMemManager::debug() chunk link error %x",(unint4) current);
             printChunk(current);
             printChunk(current->next_chunk);

#ifdef MEM_TRACE
             for (int i = 0; i < 1024; i++) {
                 if (allocations[i].chunk == current) {
                     for (int j = 0; j < 6; j++) {
                         printf("%x %s\r",allocations[i].trace[j],getMethodSignature((unint4) allocations[i].trace[j]));
                     }
                     break;
                 }
             }
#endif
         }

         if (current->next_chunk) {
             // check sizes
             if (( ((intptr_t) current) + current->size + sizeof(Chunk_Header)) != ((intptr_t) current->next_chunk)){
                 LOG(MEM, ERROR, "SequentialFitMemManager::debug() chunk size wrong  %u",(unint4) current->size);
                 printChunk(current);
                 printChunk(current->next_chunk);
             }

#if CHUNK_ACCESS_CHECK
             intptr_t addr = (intptr_t) (current + 1);
             addr += current->size;
             addr -= SAFETY_BUFFER;
             for (int i = 0; i < SAFETY_BUFFER/4; i++) {
                 if ( ((int*)addr)[i] != SAFETY_CHAR) {
                     LOG(MEM, ERROR, "SequentialFitMemManager::debug() chunk border overwritten");
                     printChunk(current);
                     memdump((int)  &(((int*)addr)[i]),1);

        #ifdef MEM_TRACE
                     for (int i = 0; i < 1024; i++) {
                         if (allocations[i].chunk == current) {
                             for (int j = 0; j < 6; j++) {
                                 printf("%x %s\r",allocations[i].trace[j],getMethodSignature((unint4) allocations[i].trace[j]));
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

             unint4 total_size = (size_t) segment->getSize() - ( (intptr_t) segment->firstAddr - (intptr_t) (segment->getStartAddr()));

             if (total_size != computed_size) {
                 LOG(MEM, ERROR, "SequentialFitMemManager::debug() last chunk has wrong size: %u",(unint4)  current->size);
                 LOG(MEM, ERROR, "SequentialFitMemManager::debug() total size != computed size : %u != %u", total_size, computed_size);
                 printChunk(current);

                 current->size += total_size - computed_size;

             }

         }

         current = current->next_chunk;
      }

}

void SequentialFitMemManager::service() {
    debug(&Segment);
    debug(&Segment_Cache_Inhibit);

}
