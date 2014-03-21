/*
 * archtypes.h
 *
 *  Created on: 30.12.2013
 *      Author: dbaldin
 */

#ifndef ARCHTYPES_H_
#define ARCHTYPES_H_

/*!
 * various integer types for ARMv4+
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

#endif /* ARCHTYPES_H_ */
