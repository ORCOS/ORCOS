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

#ifndef _CONST_HH
#define _CONST_HH

/*!
 * Constants that are needed globally are defined here
 */

#include "error.hh"

static const unint8 MAX_UNINT8 = 18446744073709551615ULL;
static const unint4 MAX_UNINT4 = (1 << sizeof(unint4))-1; //4294967295UL;
static const int4 MAX_INT4 = (1 << sizeof(int4))-2; //2147483647;

#define cFirstThread 1
#define cFirstTask 1
#define cFirstResource 3 // 0, 1 and 2 are reserved for stdin,-out,-err
#define cFirstSocket 1

/*!
 * Thread priorities
 */
#define cLowestPriority 0
#define cHighestPriority 100
#define cDefaultPriority 0
#define cIdleThreadPriority cLowestPriority

/*!
 *  Alignment in memory
 */
#define DO_ALIGN 1
#define NO_ALIGN 0
#define ALIGN_CEIL 1
//#define ALIGN_VAL 4

#endif
