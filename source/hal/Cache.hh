/*
 * Cache.hh
 *
 *  Created on: 09.12.2013
 *      Author: dbaldin
 */

#ifndef CACHE_HH_
#define CACHE_HH_

class Cache {
public:
    Cache() {
    }
    ;

    virtual ~Cache() {
    }
    ;

    virtual void invalidate_data(void* start, void* end) {
    }
    ;

    virtual void invalidate_instruction(void* start, void* end) {
    }
    ;
};

#endif /* CACHE_HH_ */
