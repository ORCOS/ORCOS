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

/*****************************************************************************
 * Method: memcpy(void* dst0, const void* src0, size_t len0)
 *
 * @description
 *
 * @returns
 *  void*       dst0
 *******************************************************************************/
void*       memcpy(void* dst0, const void* src0, size_t len0);

/*****************************************************************************
 * Method: memcpyl(void* dst0, const void* src0, size_t len0)
 *
 * @description
 *  Performs memory copy from src0 to dst0 using word transfers. len0
 *  must be a multiple of the word length.
 * @returns
 *  void*       dst0
 *******************************************************************************/
void*       memcpyl(void* dst0, const void* src0, size_t len0);

/*****************************************************************************
 * Method: memcmp(const void* m1, const void* m2, size_t n)
 *
 * @description
 *
 *******************************************************************************/
int         memcmp(const void* m1, const void* m2, size_t n);

/*****************************************************************************
 * Method: memset(void* ptr, int c, size_t n)
 *
 * @description
 *  Sets the area pointer to by ptr to c using byte memory stores
 *******************************************************************************/
void*       memset(void* ptr, int c, size_t n);

/*****************************************************************************
 * Method: memsetlong(void* ptr, int c, size_t n)
 *
 * @description
 *   Sets the area pointer to by ptr to c using word memory stores
 *******************************************************************************/
void*       memsetlong(void* ptr, int c, size_t n);


/*****************************************************************************
 * Method: memdump(int addr, int length)
 *
 * @description
 *  Prints out length 32 bit words at the memory at location addr
 *  to std out. The output is as e.g.:
 *
 *  0xab 0x34 0x12 0x14    char1 char2 char3 char 4
 *******************************************************************************/
void        memdump(void* addr, int length);

#ifdef __cplusplus
}
#endif

#endif /*MEMTOOLS_H_*/
