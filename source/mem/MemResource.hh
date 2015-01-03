/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2008 University of Paderborn

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MEMRESOURCE_HH_
#define MEMRESOURCE_HH_

#include "inc/types.hh"

/*!
 *  \ingroup memmanager
 *  \brief Class which describes a memory segment which can be managed by a memory manager
 *
 *  Used of representation for a chunk of memory.
 *  Serves as superclass and can be extended due to the needs of a specific memory manager and his
 *  allocation strategy.
 */
class MemResource {
    friend class MemManager;
    friend class Kernel; /* for exporting RO variables */
    friend class Task; /* for exporting RO variables */
private:
    //! size of the represented memory segment
    size_t memSegSize;
    //! starting address of the represented memory segment
    void* startAddr;

public:
    size_t usedBytes;

    size_t overheadBytes;

    void* lastAllocated;

    void* firstFreed;

    void* firstAddr;

    /*!
     *  \brief Constructor to create a new MemResource
     *
     *  starting address is specified and
     *  the represented memory segment is given a defined size
     */
    MemResource() {
        memSegSize    = 0;
        startAddr     = 0;
        usedBytes     = 0;
        overheadBytes = 0;
        lastAllocated = 0;
        firstFreed    = 0;
        firstAddr     = 0;
    }


    /*****************************************************************************
     * Method: getSize()
     *
     * @description
     *   Returns the size of the represented memory segment
     *
     *---------------------------------------------------------------------------*/
    inline size_t getSize() {
        return (memSegSize);
    }

    /*****************************************************************************
     * Method: getStartAddr()
     *
     * @description
     *   returns the starting address of the represented memory segment
     *---------------------------------------------------------------------------*/
    inline void* getStartAddr() {
        return (startAddr);
    }

    /*****************************************************************************
     * Method: setStartAddr(void* s)
     *
     * @description
     * set starting address of the represented memory segment
     *---------------------------------------------------------------------------*/
    inline void setStartAddr(void* s) {
        startAddr = s;
    }

    /*****************************************************************************
     * Method: getEndAddr()
     *
     * @description
     *
     *---------------------------------------------------------------------------*/
    inline void* getEndAddr() {
        return ((byte*) startAddr + memSegSize);
    }

    /*****************************************************************************
     * Method: containsAddr(void* addr)
     *
     * @description
     *
     *---------------------------------------------------------------------------*/
    inline bool containsAddr(void* addr) {
        if (((intptr_t) startAddr <= (intptr_t) addr) && ((intptr_t) startAddr + (intptr_t) memSegSize) > ((intptr_t) addr))
            return (true);
        return (false);
    }

    /*****************************************************************************
     * Method: new(size_t s, void* addr)
     *
     * @description
     *   new operator used to initialize MemResource
     *---------------------------------------------------------------------------*/
    void* operator new(size_t s, void* addr) {
        return (addr);
    }
};

#endif /*MEMRESOURCE_HH_*/
