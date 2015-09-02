/*
 * Cache.hh
 *
 *  Created on: 09.12.2013
 *      Author: dbaldin
 */

#ifndef CACHE_HH_
#define CACHE_HH_

#include "inc/types.hh"

class Cache {
public:
    Cache() {
    }


    virtual ~Cache() {
    }


    virtual void invalidate_data(void* start, size_t len) {
    }

    virtual void clean_data(void* start, size_t len) {
    }


    virtual void invalidate_instruction(void* start, size_t len) {
    }

};

#endif /* CACHE_HH_ */
