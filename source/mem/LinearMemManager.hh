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

#define MEM_NO_FREE 1

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
class LinearMemManager {
private:
    //! Pointer to the beginning of the free memory space of the managed memory segment
    char* FreeHeadPtr;

    size_t end;

    size_t start;
public:

    /*!
     * \brief creates a new LinearMemManager
     *
     * Constructor. The managed memory segment starts at parameter startAddr and end at parameter endAddr
     * The Parameter tid specifies which task this memory manager allocates memory for.
     */
    LinearMemManager(void* startAddr, void* endAddr) {
        start = (size_t) startAddr;
        FreeHeadPtr = (char*) start;
        end = (size_t) endAddr;
    }

    /*****************************************************************************
     * Method: alloc(size_t size, bool aligned, unint align_value)
     *
     * @description
     *   Allocates memory of given size from the managed memory segment
     *
     * Implements the corresponding method in MemManager.hh
     *
     * A pointer to the beginning of the allocated memory is returned.
     * The pointer FreeHeadPtr is moved by the allocated memory size.
     *---------------------------------------------------------------------------*/
    void* alloc(size_t, bool = false, unint align_value = ALIGN_VAL);

    /*****************************************************************************
     * Method: free(void*)
     *
     * @description
     *   Free is not supported.
     *---------------------------------------------------------------------------*/
    ErrorT free(void*) {
        return (cNotImplemented );
    }

    /*****************************************************************************
     * Method: new(size_t s, void* addr)
     *
     * @description
     *   new-Operator for Memory Managers, to place it directly at an desired address
     *---------------------------------------------------------------------------*/
    void* operator new(size_t s, void* addr) {
        return (addr);
    }

    /*****************************************************************************
     * Method: containsAddr(void* addr)
     *
     * @description
     *
     *---------------------------------------------------------------------------*/
    bool containsAddr(void* addr) {
        return ((intptr_t) addr >= (intptr_t) start && (intptr_t) addr < (intptr_t) end);
    }

    /*****************************************************************************
     * Method: getUsedMemSize()
     *
     * @description
     *  Method to get the used heap size
     *  Implements corresponding method in MemManager.hh
     *  Returns difference between FreeHeadPtr and startAddr
     *---------------------------------------------------------------------------*/
    size_t getUsedMemSize();
};

#endif /*LINEARMEMMANAGER_HH_*/
