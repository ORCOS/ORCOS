/*
 * NoRamManager.hh
 *
 *  Created on: 30.12.2013
 *      Author: dbaldin
 */

#ifndef NORAMMANAGER_HH_
#define NORAMMANAGER_HH_

#include "inc/types.hh"
#include "inc/const.hh"

class NoRamManager {
public:
	NoRamManager() {};

	virtual ~NoRamManager() {};

	ErrorT markAsUsed(unint4 start, unint4 end, unint1 pid) {return cError;};

	void* alloc(size_t size, unint1 pid) {return 0;};

	ErrorT free(unint4 start, unint4 end) {return cError;};

	ErrorT freeAll(unint1 pid) {return cError;};
};

#endif /* NORAMMANAGER_HH_ */
