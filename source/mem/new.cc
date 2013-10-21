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

#include <kernel/Kernel.hh>
#include "SCLConfig.hh"
#include Kernel_Thread_hh
#include Kernel_MemoryManager_hh

extern Kernel* theOS;

void* operator new( size_t s )
/*---------------------------------------------------------------------------*/
{
    Kernel_MemoryManagerCfdCl* mm;

    void* addr = 0;

    mm = theOS->getMemoryManager();
    addr = mm->alloc( s, DO_ALIGN);

    return addr;
}

void* operator new( size_t s, bool aligned )
/*---------------------------------------------------------------------------*/
{
	Kernel_MemoryManagerCfdCl* mm;

    void* addr = 0;

    mm = theOS->getMemoryManager();
    addr = mm->alloc( s, aligned );

    return addr;
}

/*---------------------------------------------------------------------------*/
void operator delete( void* ptr )
/*---------------------------------------------------------------------------*/
{
	Kernel_MemoryManagerCfdCl* mm;
    int status;

    mm = theOS->getMemoryManager();
    status = mm->free( ptr );
    if ( isOk(status) ) {
        ptr = 0;
    }

}

void* operator new[]( size_t s )
/*---------------------------------------------------------------------------*/
{
	Kernel_MemoryManagerCfdCl* mm;

    void* addr = 0;

    mm = theOS->getMemoryManager();
    addr = mm->alloc( s, NO_ALIGN);

    return addr;
}

void operator delete[]( void* ptr )
/*---------------------------------------------------------------------------*/
{
	Kernel_MemoryManagerCfdCl* mm;
    int status;

    mm = theOS->getMemoryManager();
    status = mm->free( ptr );
    if ( isOk(status) ) {
        ptr = 0;
    }

}
