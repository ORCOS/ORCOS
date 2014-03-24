/*
 * archtypes.h
 *
 *  Created on: 30.12.2013
 *      Author: dbaldin
 */

#ifndef ARCHTYPES_H_
#define ARCHTYPES_H_

/*!
 * various integer types for PPC405s
 *
 */
typedef signed char int1;
typedef signed short int2;
typedef signed int int4;
typedef signed long long int8;
typedef unsigned char unint1;
typedef unsigned short unint2;
typedef unsigned int unint4;
typedef unsigned long long unint8;

typedef unint8 TimeT;

#define printf_p printf

#endif /* ARCHTYPES_H_ */
