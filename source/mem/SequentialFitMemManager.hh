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

#ifndef SEQUENTIALFITMEMMANAGER_HH_
#define SEQUENTIALFITMEMMANAGER_HH_

#include "MemManager.hh"
#include "inc/types.hh"
#include "inc/error.hh"
#include "Alignment.hh"

class Resource;

typedef struct Chunk_Header {
    Chunk_Header *prev_chunk;
    Chunk_Header *next_chunk;
    unint1 state :1;
    unint4 size :31;  //(sizeof(unint4)-1);
}__attribute__((packed)) Chunk_Header;

#ifndef MEM_TRACE
#define MEM_TRACE 0
#endif

#if MEM_TRACE
typedef struct ChunkTrace {
   int used;
   void* chunk;
   void* trace[6];
   size_t malloc_size;
} ChunkTrace;
#endif

// possible States of a memory chunk
#define FREE 0
#define OCCUPIED 1

/*
 * Defines the number of bytes placed between two consecutive chunks
 * in order to cope with erroneous data bounds accesses.
 */
#define SAFETY_BUFFER 16

/*
 * Additional check whether an allocated chunk is accessed out of bounds in positive direction.
 * Allocates extra bytes and checks them on modification regularly.
 */
#define CHUNK_ACCESS_CHECK 0


#define SAFETY_CHAR 0x2ffe9ab1

// Minimum payload needed for a chunk when splitting
#define MINIMUM_PAYLOAD 0x20

// activating precisely validity checks checks a freed chunk on correct placement
// inside the chunk list. However, takes a lot more time.
#define PRECISE_VALIDITY_CHECK 1

// MEM_LAST_FIT will start searching at the last freed object for chunks to allocated
// this is faster than starting to search from the beginning
// however: this will lead to a higher number of chunks, thus, increasing the number
// of reserved (used) bytes not allocateable for the user.
#ifndef MEM_LAST_FIT
#define MEM_LAST_FIT 0
#endif


/*!
 * \ingroup memmanager
 *
 * \brief Class is a concrete implementation of a memory manager,
 *  providing allocation and release of memory. The memory is organized in a linked list of memory chunks
 *
 * SequentialFitMemManager only deals with a contiguous memory segment
 *
 * The memory is divided in chunks which are organized in a linked list.
 *
 * This memory manager features lists of allocatable segments with free alignment. Segments are automatically
 * arranged in memory to fullfill the alignment requirements of the malloc call.
 *
 * The organization of the linked list of memory chunks is as follows:
 * every chunk has a Header containing management information and a payload containing the data. So there exists
 * no additional structure in the memory to keep track of free and allocated chunks
 * The Header is 16 bytes and consists of: <br>
 * 0      4      8       12      16 <br>
 * ------------------------------ <br>
 * | PREV | NEXT | STATE | SIZE | with <br>
 * ------------------------------ <br>
 * - PREV: the preceding chunk in the list <br>
 * - NEXT: the following chunk in the list <br>
 * - STATE: the state of the chunk: free(possible to allocate) or occupied(contains already data) <br>
 * - SIZE: the payload size of the chunk <br>
 *
 * Directly after the Header starts the payload section. Every chunk is addressed by the start address of its payload.
 *
 * If memory should be allocated the linked list of chunks is traversed to find a free chunk of sufficient size.
 *
 * If a chunk is chosen for allocation and its payload leaves a rest of unused memory which is bigger than the Header size + the Minimum Payload,
 * then a split operation is performed on this chunk meaning the chunk is divided in two chunks: one containing the allocated memory and one containing the unused rest.
 *
 * If memory is released a merge operation is performed if the chunk containing the released memory has a neighbor chunk which is tagged as free. These free chunks are merged into
 * one chunk.
 *
 */
class SequentialFitMemManager: public MemManager {
    /* list of scheduled deletions. Resources will
     * be freed if the idle thread is executed once. */
    Resource*  scheduledDeletion[40];

    /* current start entry */
    int  schedDeletionStartPos;

    /* number of elements inside the scheduledDeletion array*/
    int  schedDeletionCount;

    /* number of elements safe to delete starting at schedDeletionNum */
    int  schedDeletionSafeNum;

    int  m_lock;

private:
    /*****************************************************************************
     * Method: split(Chunk_Header* chunk, size_t &size, MemResource* segment)
     *
     * @description
     * The splitting of the chunk is only possible if the following condition holds:
     * size of the payload of this chunk is bigger or equal to requested size + size of header + minimum payload size
     *
     * If true -> the chunk is splitted in two chunks A and B where A is of the requested size and will contain the data for which the allocation has been called and
     *            B will contain the remaining unused memory of the payload
     *
     * If false -> the whole chunk is used and its state is set to occupied.
     *
     *******************************************************************************/
    void split(Chunk_Header*, size_t&, MemResource* segment);

    /*****************************************************************************
     * Method: merge(Chunk_Header* chunk, MemResource* segment)
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
    void merge(Chunk_Header* chunk, MemResource* segment);


    /*****************************************************************************
     * Method: isValidChunkAddress(Chunk_Header* chunk, MemResource* &segment)
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
    bool isValidChunkAddress(Chunk_Header* chunk,  MemResource* &segment);


    /*****************************************************************************
     * Method: getFittingChunk(size_t size,
     *                         bool aligned,
     *                         unint4 align_val,
     *                         Chunk_Header* current_chunk,
     *                         MemResource* segment)
     *
     * @description
     *  Tries to find a free memory chunk inside the given segment starting searching at
     *  current_chunk.
     *
     * @params
     *
     * @returns
     *  Chunk_Header*         Free fitting chunk or null if none
     *******************************************************************************/
    Chunk_Header* getFittingChunk(size_t size, bool aligned, unint4 align_val, Chunk_Header* current_chunk, MemResource* segment);

public:
    /*!
     * \brief creates a new SequentialFitMemManager
     *
     * Constructor. The managed memory segment starts at parameter startAddr and end at parameter endAddr
     */
    SequentialFitMemManager(void* startAddr, void* endAddr);

#if MEM_CACHE_INHIBIT
    SequentialFitMemManager(void* startAddr, void* endAddr, void* istartAddr, void* iendAddr);
#endif

    /*****************************************************************************
     * Method: alloc(size_t size, bool aligned, unint4 align_val, MemResource* segment)
     *
     * @description
     *  Tries to allocate a new memory chunk of given size inside the given segment.
     *
     * @params
     *
     * @returns
     *  void*         The assigned memory address. 0 on error
     *******************************************************************************/
    void* alloc(size_t size, bool aligned = true, unint4 align_val = ALIGN_VAL, MemResource* segment = 0);

#if MEM_CACHE_INHIBIT
    /*****************************************************************************
     * Method: alloci(size_t size, bool aligned, unint4 align_val)
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
    void* alloci(size_t size, bool aligned = true, unint4 align_val = ALIGN_VAL);
#endif

    /*****************************************************************************
     * Method: free(void* addr)
     *
     * @description
     *  If the given address is a valid address of a memory chunk,
     *  it is attempted by calling the merge method to merge the memory chunk which is freed with his neighboring chunks (if these are free)
     *
     *******************************************************************************/
    ErrorT free(void* addr);


    /*****************************************************************************
     * Method: getUsedMemSize(size_t &u_overhead, size_t &u_free_mem)
     *
     * @description
     *  method to get the used heap size.
     *  The linked list is traversed and returned is the sum of all
     *  Headers of free chunks and the Headers and Payload of occupied chunks
     *******************************************************************************/
    size_t getUsedMemSize(size_t &u_overhead, size_t &u_free_mem);

    /*****************************************************************************
     * Method: printChunk(Chunk_Header* chunk)
     *
     * @description
     *
     *******************************************************************************/
    void printChunk(Chunk_Header* chunk);

    /*****************************************************************************
     * Method: service()
     *
     * @description
     *  Service method to be called regularly
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    void service();

    /*****************************************************************************
     * Method: debug()
     *
     * @description
     *
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    void debug(MemResource* segment);

    /*****************************************************************************
     * Method: scheduleDelete(Resource* addr)
     *
     * @description
     *  Schedule the deletion of thos resource.
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    void scheduleDeletion(Resource* addr);

    /*****************************************************************************
     * Method: idleEnter
     *
     * @description
     *  Method should be called upon idle thread entry.
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    void idleEnter();

};

#endif /*SEQUENTIALFITMEMMANAGER_HH_*/
