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
private:
    //! size of the represented memory segment
    size_t memSegSize;
    //! starting address of the represented memory segment
    void* startAddr;

public:

    /*!
     *  \brief Constructor to create a new MemResource
     *
     *  starting address is specified and
     *  the represented memory segment is given a defined size
     */
    MemResource() {
        memSegSize = 0;
        startAddr = 0;
    }

    /*!
     *  \brief returns the size of the represented memory segment
     *
     *  method should not be used if the memory segment is unlimited
     *  as the size is then undefined
     */
    inline size_t getSize() {
        return memSegSize;
    }

    //! returns the starting address of the represented memory segment
    inline void* getStartAddr() {
        return startAddr;
    }

    //! set starting address of the represented memory segment
    inline void setStartAddr( void* s ) {
        startAddr = s;
    }

    inline void* getEndAddr() {
        return (byte*) startAddr + memSegSize;
    }

    //! new operator used to initialize MemResource
    void* operator new( size_t s, void* addr ) {
        return addr;
    }

};

#endif /*MEMRESOURCE_HH_*/
