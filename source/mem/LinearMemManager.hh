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

#ifndef LINEARMEMMANAGER_HH_
#define LINEARMEMMANAGER_HH_

#include "MemManager.hh"
#include <inc/types.hh>
#include <inc/error.hh>
#include "SCLConfig.hh"


#define MEM_NO_FREE


/*!
 *  \ingroup memmanager
 *  \brief Class is a concrete implementation of a memory manager,
 *  only allocation of memory, no free method
 *
 *  LinearMemManager only deals with a contiguous memory segment (which can be both limited in size or not)
 *  It also provides only a method for the allocation of memory.
 *  Memory which is once allocated will not be released anymore.
 *
 */
class LinearMemManager: public MemManager {
private:
    //! Pointer to the beginning of the free memory space of the managed memory segment
	char* FreeHeadPtr;

public:

    /*!
     * \brief creates a new LinearMemManager
     *
     * Constructor. The managed memory segment starts at parameter startAddr and end at parameter endAddr
     * The Parameter tid specifies which task this memory manager allocates memory for.
     */
    LinearMemManager( void* startAddr, void* endAddr) :
        MemManager( startAddr, endAddr ) {
        FreeHeadPtr = (char*) Segment.getStartAddr();
    }

    /*!
     * \brief allocates memory of given size from the managed memory segment
     *
     * Implements the corresponding method in MemManager.hh
     *
     * A pointer to the beginning of the allocated memory is returned.
     * The pointer FreeHeadPtr is moved by the allocated memory size.
     */
    void* alloc( size_t, bool = false, unint align_value = ALIGN_VAL);

    ErrorT free( void* ) {
        return cNotImplemented;
    }
    ;


    /*!
     * \brief method to get the used heap size
     *
     * Implements corresponding method in MemManager.hh
     *
     * Returns difference between FreeHeadPtr and startAddr
     */
    size_t getUsedMemSize(int* fragmentation = 0);


#ifdef SERIALIZE
    /*!
    * \brief serialization method of this mem manager
    */
    bool serialize(void* &serialized_object, unint2 &length);

    static LinearMemManager* deserialize(void* serialized_object, unint2 length);
#endif
};

#endif /*LINEARMEMMANAGER_HH_*/
