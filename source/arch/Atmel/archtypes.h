/*
 * archtypes.h
 *
 *  Created on: 30.12.2013
 *    Copyright & Author: dbaldin
 */

#ifndef SOURCE_ARCH_ATMEL_ARCHTYPES_H_
#define SOURCE_ARCH_ATMEL_ARCHTYPES_H_

#include "avr/io.h"
#include "avr/common.h"
#include "avr/pgmspace.h"

/*!
 * various integer types for ATMEL 8 bit
 *
 */
typedef signed char      int1;
typedef signed int       int2;
typedef signed long      int4;
typedef signed long long int8;
typedef unsigned char    unint1;
typedef unsigned int     unint2;
typedef unsigned long    unint4;
typedef unsigned long long unint8;

typedef unsigned int size_t;

typedef unint2 TimeT;

#define MAX_UINT8  18446744073709551615ULL
#define MAX_UINT4  4294967295UL
#define MAX_INT4   2147483647

#define PROGRMEM 1

extern int printf_p(const char *format_str, ...);

#define LOG(prefix, level, msg,...) if ((int) level <= (int) prefix) { printf_p(PSTR(msg "\n") , ## __VA_ARGS__ ); }

#endif // SOURCE_ARCH_ATMEL_ARCHTYPES_H_
