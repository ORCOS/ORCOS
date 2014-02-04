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

/* No harvard architecture. Program == data memory*/
#define printf_p printf

#endif /* ARCHTYPES_H_ */
