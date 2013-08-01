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

#ifndef MEMMANAGER_HH_
#define MEMMANAGER_HH_

// Import section
#include <inc/types.hh>
#include "SCLConfig.hh"
#include MemoryManager_Seg_hh
#include MemoryManager_HatLayer_hh

//forward declaration of configured Threadtype
class ThreadCfdCl;


/*!
 * \ingroup memmanager
 * \brief Superclass for all memory managers, provides an interface for them
 *
 * MemManager is an interface for all memory managers which can be used in ORCOS
 * to allocate and free memory from a specified memory segment. The concrete allocation
 * strategy is not part of this class.
 *
 *
 */
class MemManager {
protected:
    //! the memory segment to be managed
    MemResource Segment;
//DEF_MemoryManager_SegCfd

public:

    /*!
     * \brief Constructor
     *
     * Places the HatLayer directly at physaddr followed by the Segment Descriptor. The managed memory segment starts at saddr and ends at eddr.
     */
    MemManager(void* saddr, void* eaddr) {
        Segment.startAddr = saddr;
        Segment.memSegSize = (byte*)eaddr - (byte*)saddr;
        //SegCfd = new(physaddr) MemoryManager_SegCfdCl((byte*)saddr , (byte*)eaddr - (byte*)saddr);
    }

    //! new-Operator for Memory Managers, to place it directly at an desired address
    void* operator new(size_t s,void* addr)
    {
        return addr;
    }


};

#endif /*MEMMANAGER_HH_*/
