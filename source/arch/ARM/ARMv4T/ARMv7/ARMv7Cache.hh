/*
 * ARMv7Cache.hh
 *
 *  Created on: 09.12.2013
 *      Author: dbaldin
 */

#ifndef ARMV7CACHE_HH_
#define ARMV7CACHE_HH_

#include "hal/Cache.hh"
#include "inc/types.hh"

class ARMv7Cache: public Cache {

private:
	unint4 line_len;

public:
	ARMv7Cache();
	virtual ~ARMv7Cache();

	void invalidate_data(void* start, void* end);

	void invalidate_instruction(void* start, void* end);

	void invalidate(unint4 asid);

};

#endif /* ARMV7CACHE_HH_ */
