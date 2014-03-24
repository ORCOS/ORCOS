/*
 * PagedRamMemManager.hh
 *
 *  Created on: 27.07.2013
 *      Author: dbaldin
 */

#ifndef PAGEDRAMMEMMANAGER_HH_
#define PAGEDRAMMEMMANAGER_HH_

#include "inc/types.hh"

class PagedRamMemManager {
public:
	PagedRamMemManager();

	virtual ~PagedRamMemManager();

	ErrorT markAsUsed(unint4 start, unint4 end, unint1 pid);

	/*
	 * Tries to allocate a consecutive memory area of size "size" for
	 * process with id pid.
	 *
	 * Returns the physical memory address of the allocated region on success.
	 * Returns 0 if no area could be allocated.
	 **/
	void* alloc(size_t size, unint1 pid);

	/*
	 * Makrs the memory pages from start to end as free.
	 */
	ErrorT free(unint4 start, unint4 end);

	/*
	 * Removes all ram allocations used by process with pid.
	 */
	ErrorT freeAll(unint1 pid);

	/*
	 * Maps the pages used by the kernel into the process given by pid.
	 */
	ErrorT mapKernelPages(int pid);
};

#endif /* PAGEDRAMMEMMANAGER_HH_ */
