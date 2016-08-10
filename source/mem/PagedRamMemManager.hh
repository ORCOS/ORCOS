/*
 * PagedRamMemManager.hh
 *
 *  Created on: 27.07.2013
 *      Copyright & Author: dbaldin
 */

#ifndef PAGEDRAMMEMMANAGER_HH_
#define PAGEDRAMMEMMANAGER_HH_

#include "inc/types.hh"
#include "hal/CharacterDevice.hh"


class PagedRamMemManager : CharacterDevice {
private:
    int m_lock;  // lock to protect against concurrent modification

     /*****************************************************************************
     * Method: allocSubPages(int page, size_t size)
     *
     * @description
     *  Allocates a series of sub pages (4KB pages) inside the
     *  given page. Allowed sizes = 64 KB or 4 KB. All other
     *  sizes are invalid!
     *
     * @params
     *   page       Page to allocate the subpages in
     *   size       Size to allocated. 64KB or 4 KB
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    intptr_t allocSubPages(int page, size_t size);
public:
    PagedRamMemManager();

    ~PagedRamMemManager();

    /*****************************************************************************
     * Method: markAsUsed(unint4 start, unint4 end, unint1 pid)
     *
     * @description
     *  Marks the memory area between start and end as used by the process space
     *  with given pid, basically reserving that memory area. You may
     *  use this method to reserve areas of ram from allocation at early boot time
     *  to do what ever you want with that area.
     *
     * @params
     *  start       Physical starting page address
     *  end         Physical end page address
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT markAsUsed(intptr_t start, intptr_t end, unint1 pid);

    /*****************************************************************************
     * Method: alloc_logical(size_t size, unint1 pid, intptr_t logical_address);
     *
     * @description
     *  Tries to allocate a logically contiguous memory area of size "size" for
     *  process with id pid and maps it to logical address.
     *
     *  Returns the logical memory address of the mapped and allocated region on success.
     *  Returns 0 if no area could be allocated.
     *
     * @params
     *   logical_address  address the area shall be mapped to if != 0
     *
     * @returns
     *  void*             logical address the area is mapped to (if no mmu the physical address is returned)
     *******************************************************************************/
    intptr_t alloc_logical(size_t size, unint1 pid, intptr_t logical_address);

    /*****************************************************************************
     * Method: alloc_phyiscal(size_t size, unint1 pid)
     *
     * @description
     *  Tries to allocate a physical contiguous area of size 'size' for the address space of PID.
     *  The algorithm is implemented to minimize the number of mappings required to map the area.
     *  Thus, sizes > 1 MB will be mapped using ONLY 1 MB pages. Example: 1.2 MB will be allocated by using
     *  two contiguous 1 MB pages. High fragmentation can be introduces this way, thus it is the
     *  responsibility of the caller to ensure proper allocations.
     *
     *  The size is always rounded up to the next higher page size.
     *  Examples:
     *  40 KB allocations will result in one 64 KB page allocation.
     *  20 Bytes allocation in 4 KB allocations.
     *  70 KB in a 1 MB allocations.
     *
     * @params
     *   size             size of the area to be allocated
     *
     * @returns
     *  void*             physical address the area is mapped to (if no mmu the physical address is returned)
     *******************************************************************************/
    intptr_t alloc_physical(size_t size, unint1 pid);

    /*****************************************************************************
     * Method: free_physical(unint4 start, unint4 end)
     *
     * @description
     *  Marks the memory pages from start to end as free. This function
     *  ignores the original allocation size. Thus, if an area of size
     *  64 Kb has been allocated and this method is called with end-start
     *  == 59 KB only 60 kb will be freed. It is the responsibility
     *  of the called to correctly free areas that have been allocated before!
     *
     *
     * @params
     *  start       physical starting page address
     *  end         physical end page address
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT free_physical(intptr_t start, intptr_t end);

    /*****************************************************************************
     * Method: free_logical(unint4 start, unint4 end)
     *
     * @description
     *  Marks the memory pages from start to end as free. Only works for the logical
     *  addresses of the current executing task!
     *
     * @params
     *  start       logical starting page address
     *  end         logical end page address
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT free_logical(intptr_t start, intptr_t end);

    /*****************************************************************************
     * Method: freeAll(unint1 pid)
     *
     * @description
     *   Marks all pages used by address page pid as free again
     * @params
     *  pid         PID
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT freeAll(unint1 pid);

    /*****************************************************************************
     * Method: mapRAM(int pid)
     *
     * @description
     *  Maps all RAM pages into the address space 1:1 for kernel access
     *  into the Process with given PID. (only accessable from kernel mode)
     *
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT mapRAM(int pid);

    /*****************************************************************************
     * Method: readBytes(char *bytes, unint4 &length)
     *
     * @description
     *  Allows reading the current page allocations in
     *  human readable ascii form.
     *
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT readBytes(char *bytes, unint4 &length);
};

#endif /* PAGEDRAMMEMMANAGER_HH_ */
