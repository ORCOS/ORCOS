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

#include "LinearMemManager.hh"
#include <Alignment.hh>
#include <assemblerFunctions.hh>

#include "inc/memtools.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;



void* LinearMemManager::alloc(size_t size, bool aligned, unint align_value) {
    register char* ptr;
    register void* retval;

    ATOMAR(

    if ( aligned )
        ptr = align( FreeHeadPtr, align_value);
    else ptr = FreeHeadPtr;

    // see if we fit into our memory segment
    if ( (size_t) ptr + size <=  end ) {
        FreeHeadPtr =  ptr + size;
        retval =(void*) ptr;
    }
    else
    {
        // big fat oops
        LOG(MEM,ERROR,"Out Of memory!");
        retval = 0;
    }

    );

    ASSERT(retval);
    return (retval);
}

size_t LinearMemManager::getUsedMemSize() {
    return ((size_t) FreeHeadPtr - start);
}

#ifdef SERIALIZE
bool LinearMemManager::serialize(void* &serialized_object, unint2 &length)
{
    serialized_object = this;
    length = sizeof(LinearMemManager);
    return 1;
}

LinearMemManager* LinearMemManager::deserialize(void* serialized_object, unint2 length)
{
    if (length != sizeof(LinearMemManager)) return 0;

    LinearMemManager* newobject = (LinearMemManager*) theOS->getMemoryManager()->alloc(length);
    memcpy(newobject,serialized_object,length);
    return newobject;
}
#endif
