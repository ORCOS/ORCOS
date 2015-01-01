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
     *  end         Phyiscal end page address
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT markAsUsed(unint4 start, unint4 end, unint1 pid);

    /*****************************************************************************
     * Method: alloc(size_t size, unint1 pid)
     *
     * @description
     * Tries to allocate a consecutive memory area of size "size" for
     * process with id pid.
     *
     *  Returns the physical memory address of the allocated region on success.
     *  Returns 0 if no area could be allocated.
     *
     * @params
     *
     * @returns
     *  void*         physical page
     *******************************************************************************/
    void* alloc(size_t size, unint1 pid);

    /*****************************************************************************
     * Method: free(unint4 start, unint4 end)
     *
     * @description
     *  Marks the memory pages from start to end as free.
     * @params
     *  start       Physical starting page address
     *  end         Phyiscal end page address
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT free(unint4 start, unint4 end);

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
     *  Maps all pages used by the kernel, has pid 0, into the address space
     *  of the given pid.
     *
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT mapKernelPages(int pid);
};

#endif /* PAGEDRAMMEMMANAGER_HH_ */
