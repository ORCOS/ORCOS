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

#ifndef _SHARED_POWERPC_SYSTYPES_HH
#define _SHARED_POWERPC_SYSTYPES_HH

/*!
 * \brief PowerPC 405 type declaration
 *
 * this file specifies the correct typification of datatypes lite int4,int2,...
 */

#ifndef _ASSEMBLER
//#include <stddef.h>
#endif

#ifndef _ASSEMBLER
/*!
 * word is as long as a machines word
 */
typedef int word;
typedef unsigned int unword;

/*!
 * processor register type
 */
typedef int RegisterT;

/*!
 * FPU coprocessor register type
 */
typedef int FPURegT[ 2 ];


/*!
 * various integer types for ppc405
 *
 */
typedef char int1;
typedef short int2;
typedef int int4;
typedef long long int8;
typedef unsigned char unint1;
typedef unsigned short unint2;
typedef unsigned int unint4;
typedef unsigned long long unint8;

typedef unint8 TimeT;


#define MAX_UINT8  18446744073709551615ULL
#define MAX_UINT4  4294967295UL
#define MAX_INT4  2147483647


/* No harvard architecture. Program == data memory*/
#define printf_p printf


// </group>


/*!
 * the struct TimerTickT and ClockTickT
 * <group>
 */
//typedef unint4 TimerTickT;
//typedef unint8 ClockTickT;
// </group>
#endif

#ifndef _ASSEMBLER
//typedef unint2 PriorityT;
#define cLowPri                 (PriorityT) (cMaxPri-1)
#define cHighPri                (PriorityT) 0
#endif

#endif /* _SHARED_POWERPC_SYSTYPES_HH */
