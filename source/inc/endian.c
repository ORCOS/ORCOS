/*
 * endian.c
 *
 *  Created on: 11.07.2013
 *    Copyright & Author: dbaldin
 */

#include "lwip/arch.h"
#include "SCLConfig.hh"
#include "endian.h"

#if BYTE_ORDER != BIG_ENDIAN
#if BYTE_ORDER != LITTLE_ENDIAN
#error "Target Architecture Endianess settings missing. Add BYTE_ORDER define with value BIG_ENDIAN or LITTLE_ENDIAN define in the SCLConfig file."
#endif
#endif

#if BYTE_ORDER == BIG_ENDIAN

/*****************************************************************************
 * Method: cputole32(unint4 n)
 *
 * @description
 *  Converts an 32 bit integer given in cpu endiannes
 *   to little endian
 * @params
 *  n         value in host format
 * @returns
 *  int        value in little endian format
 *******************************************************************************/
unint4 cputole32(unint4 n) {
    return (__builtin_bswap32(n));

    /*return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000UL) >> 8) |
    ((n & 0xff000000UL) >> 24);*/
}

/*****************************************************************************
 * Method: cputole16(unint4 n)
 *
 * @description
 *  Converts an 16 bit integer given in cpu endiannes
 *   to little endian
 * @params
 *  n         value in host format
 * @returns
 *  int        value in little endian format
 *******************************************************************************/
unint2 cputole16(unint2 n) {
    return (__builtin_bswap16(n));
    //return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

#else

/*****************************************************************************
 * Method: cputole32(unint4 n)
 *
 * @description
 *  Converts an 32 bit integer given in cpu endiannes
 *   to little endian
 * @params
 *  n         value in host format
 * @returns
 *  int        value in little endian format
 *******************************************************************************/
unint4 cputole32(unint4 n) {
    return (n);
}

/*****************************************************************************
 * Method: cputole16(unint4 n)
 *
 * @description
 *  Converts an 16 bit integer given in cpu endiannes
 *   to little endian
 * @params
 *  n         value in host format
 * @returns
 *  int        value in little endian format
 *******************************************************************************/
unint2 cputole16(unint2 n) {
    return (n);
}

#endif

#if BYTE_ORDER == BIG_ENDIAN

/*****************************************************************************
 * Method: cputobe32(unint4 n)
 *
 * @description
 *  Converts an 32 bit integer given in cpu endiannes
 *   to big endian
 * @params
 *  n         value in host format
 * @returns
 *  int        value in big endian format
 *******************************************************************************/
unint4 cputobe32(unint4 n){
    return (n);
}

/*****************************************************************************
 * Method: cputobe16(unint4 n)
 *
 * @description
 *  Converts an 16 bit integer given in cpu endiannes
 *   to big endian
 * @params
 *  n         value in host format
 * @returns
 *  int        value in big endian format
 *******************************************************************************/
unint2 cputobe16(unint2 n) {
    return (n);
}

#else

/*****************************************************************************
 * Method: cputobe32(unint4 n)
 *
 * @description
 *  Converts an 32 bit integer given in cpu endiannes
 *   to big endian
 * @params
 *  n         value in host format
 * @returns
 *  int        value in big endian format
 *******************************************************************************/
unint4 cputobe32(unint4 n) {
    return (__builtin_bswap32(n));
    /*return (((n & 0xff) << 24) | ((n & 0xff00) << 8) | ((n & 0xff0000UL) >> 8)
            | ((n & 0xff000000UL) >> 24));*/
}

/*****************************************************************************
 * Method: cputobe16(unint4 n)
 *
 * @description
 *  Converts an 16 bit integer given in cpu endiannes
 *   to big endian
 * @params
 *  n         value in host format
 * @returns
 *  int        value in big endian format
 *******************************************************************************/
unint2 cputobe16(unint2 n) {
    return (__builtin_bswap16(n));
    //return ((unint2)(((n & 0xff) << 8) | ((n & 0xff00) >> 8)));
}

#endif

