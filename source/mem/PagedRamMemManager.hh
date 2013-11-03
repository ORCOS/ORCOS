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

	void* alloc(size_t size, int pid);

	ErrorT free(unint4 start, unint4 end);

	ErrorT freeAll(int pid);
};

#endif /* PAGEDRAMMEMMANAGER_HH_ */
