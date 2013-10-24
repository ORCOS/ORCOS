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


#include "./types.h"
#include "./defines.h"
#include "./orcos.hh"

/*!
 * The new operator for user level threads
 */
void* operator new( size_t s ) {
    return (void*) syscall( cNewSysCallId, s );
}

extern "C"void* malloc(size_t s)
{
    return (void*) syscall(cNewSysCallId,s);
}

extern "C"void* mallocp(size_t s,int mode)
{
    return (void*) syscall(cNewProtSysCallId,s,mode);
}

extern "C"void free(void *s)
{
    syscall(cDeleteSysCallId,s);
}

extern "C" int map_logmemory( const char* log_start, const char* phy_start, size_t size, int protection)
{
	return syscall(cMapMemorySyscallId,log_start,phy_start,size,protection);
}

extern "C" int 	shm_map(const char* file,unint4* mapped_address, unint4* mapped_size) {
	return syscall(cShmMapId,file,mapped_address,mapped_size);
}


/*!
 * The delete operator for user level threads
 */
void operator delete( void* ptr ) {
    syscall( cDeleteSysCallId, ptr );
}


