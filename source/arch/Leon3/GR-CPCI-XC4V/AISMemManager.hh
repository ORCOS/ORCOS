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

#ifndef AISMEMMANAGER_HH_
#define AISMEMMANAGER_HH_

#include <inc/types.hh>
#include <inc/error.hh>

typedef struct {
	unint4 start;
	unint4 end;
} protected_mem_area;

extern protected_mem_area pmem_areas[16];


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
class AISMemManager {
private:
    //! Pointer to the beginning of the free memory space of the managed memory segment
	unint4 protmemptr[16];

public:

    /*!
     * \brief creates a new LinearMemManager
     *
     * Constructor. The managed memory segment starts at parameter startAddr and end at parameter endAddr
     * The Parameter tid specifies which task this memory manager allocates memory for.
     */
    AISMemManager() {
		int i ;
		for (i = 0; i < 16; i++) protmemptr[i] = pmem_areas[i].start;
		setHardwareProtection();
    }


    void* allocp( size_t size, unint1 protection);

    void setHardwareProtection();

};

#endif /*LINEARMEMMANAGER_HH_*/
