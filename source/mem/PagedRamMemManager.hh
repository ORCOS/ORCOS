/*
 * PagedRamMemManager.hh
 *
 *  Created on: 27.07.2013
 *      Copyright & Author: dbaldin
 */

#ifndef PAGEDRAMMEMMANAGER_HH_
#define PAGEDRAMMEMMANAGER_HH_

#include "inc/types.hh"



class PagedRamMemManager {
public:
    PagedRamMemManager();

    ~PagedRamMemManager();

    /*****************************************************************************
     * Method: markAsUsed(unint4 start, unint4 end, unint1 pid)
     *
     * @description
     *  Marks the memory area between start and end as used by the process space
     *  with given pid
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
     * Method: mapContiguous(size_t size, unint1 pid, intptr_t logical_address)
     *
     * @description
     *  Tries to allocate a physically contiguous memory area of size "size" for
     *  process with id pid and maps it to logical address. Areas > 1 MB will be mapped
     *  using 1 MB pages only.
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
    intptr_t mapContiguous(size_t size, unint1 pid, intptr_t logical_address);

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
     *  Tries to allocate a physical contiguous area of size for the given pid. Does not map it into the process.
     *
     *  Returns the physical memory address of the allocated region on success.
     *  Returns 0 if no area could be allocated.
     *
     * @params
     *   size             size of the area to be allocated
     *
     * @returns
     *  void*             physical address the area is mapped to (if no mmu the physical address is returned)
     *******************************************************************************/
    intptr_t alloc_physical(size_t size, unint1 pid);

    /*****************************************************************************
     * Method: free(unint4 start, unint4 end)
     *
     * @description
     *  Marks the memory pages from start to end as free.
     *
     * @params
     *  start       logical starting page address
     *  end         logical end page address
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT free(intptr_t start, intptr_t end);

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
     * Method: mapKernelPages(int pid)
     *
     * @description
     *  Maps all pages used by the kernel (pid 0) into the address space
     *  of the given pid.
     *
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT mapKernelPages(int pid);

private:
    intptr_t allocSubPages(int page, size_t size);
};

#endif /* PAGEDRAMMEMMANAGER_HH_ */
