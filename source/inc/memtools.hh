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

#ifndef MEMTOOLS_H_
#define MEMTOOLS_H_

#include "inc/types.hh"

#ifdef __cplusplus
extern "C" {
#endif

void* memcpy( void* dst0, const void* src0, size_t len0 );

int memcmp( const void* m1, const void* m2, size_t n );

void* memset( void* ptr, int c, size_t n );

void* memsetlong( void* ptr, int c, size_t n);

#ifdef __cplusplus
unint4 getNextWord( struct packet_layer* &packet, unint2 &position );

unint2 getNextHalfWord( struct packet_layer* &packet, unint2 &position );
#endif

void memdump(int addr,int length);

#ifdef __cplusplus
}
#endif

#endif /*MEMTOOLS_H_*/
