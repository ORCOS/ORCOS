/*
 * endian.c
 *
 *  Created on: 11.07.2013
 *      Author: dbaldin
 */

#include "lwip/arch.h"
#include "SCLConfig.hh"

#if BYTE_ORDER != BIG_ENDIAN
#if BYTE_ORDER != LITTLE_ENDIAN
#error "Target Architecture Endianess settings missing. Add BYTE_ORDER define with value BIG_ENDIAN or LITTLE_ENDIAN define in the SCLConfig file."
#endif
#endif

#if BYTE_ORDER == BIG_ENDIAN

unint4
cputole32(unint4 n)
{
  return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000UL) >> 8) |
    ((n & 0xff000000UL) >> 24);
}

unint2
cputole16(unint2 n)
{
  return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

#else

unint4 cputole32(unint4 n) { return n;}

unint2 cputole16(unint2 n) { return n;}

#endif


#if BYTE_ORDER == BIG_ENDIAN

unint4 cputobe32(unint4 n) { return n;}

unint2 cputobe16(unint2 n) { return n;}

#else

unint4
cputobe32(unint4 n)
{
  return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000UL) >> 8) |
    ((n & 0xff000000UL) >> 24);
}

unint2
cputobe16(unint2 n)
{
  return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

#endif


