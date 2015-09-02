/*
 * ARMv7Cache.hh
 *
 *  Created on: 09.12.2013
 *    Copyright &  Author: dbaldin
 */

#ifndef ARMV7CACHE_HH_
#define ARMV7CACHE_HH_

#include "hal/Cache.hh"
#include "inc/types.hh"

class ARMv7Cache: public Cache {
private:
    unint4 line_len;
    unint1 LoU; // Level of Unification
    unint1 LoC; // level of Coherency
    unint1 LlC; // last level Cache

public:
    ARMv7Cache();
    ~ARMv7Cache();

    /*****************************************************************************
     * Method: invalidate_data(void* start, void* end)
     *
     * @description
     *  Invalidates all data cache lines containing physical addresses between
     *  start and end. Data is not written back. Ensures the data
     *  is fetched from POC (main memory) on next access.
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    void invalidate_data(void* start, size_t len);


    /*****************************************************************************
     * Method: clean_data(void* start, void* end)
     *
     * @description
     *  Cleans all data cache lines containing physical addresses between
     *  start and end causing the data to be written back to POC (main memory) if
     *  has changed.
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    void clean_data(void* start, size_t len);

    /*****************************************************************************
     * Method: invalidate_instruction(void* start, void* end)
     *
     * @description
     *  Invalidates all instruction cache lines containing physical addresses between
     *  start and end.
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    void invalidate_instruction(void* start, size_t len);

    /*****************************************************************************
     * Method: invalidate(unint4 asid)
     *
     * @description
     *  Ensures that no valid cache line exists containing addresses from
     *  the given address space. For ARMv7 this unfortunatly means
     *  we must invalidate the whole cache ..
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    void invalidate(unint4 asid);
};

#endif /* ARMV7CACHE_HH_ */
