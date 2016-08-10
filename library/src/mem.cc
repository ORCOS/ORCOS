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


#include "orcos_types.h"
#include "defines.h"
#include "orcos.h"
#include "sys/types.h"
#include "stdlib.h"


extern "C" int shm_map(const char* file, unint4* mapped_address, unint4* mapped_size, unint4 flags, unint4 offset) {
    return (syscall(cShmMapId, file, mapped_address, mapped_size, flags, offset));
}

void* operator new( size_t s ) {
    return ((void*) malloc(s));
}

/*!
 * The delete operator for user level threads
 */
void operator delete(void* ptr) {
    free(ptr);
}



