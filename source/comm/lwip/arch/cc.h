/*
 * cc.h
 *
 *  Created on: 27.01.2010
 *      Author: dbaldin
 */

#ifndef CC_H_
#define CC_H_

#include "stringtools.hh"
#include "types.hh"
#include "sprintf.hh"
#include "lwipopts.h"

/* Define platform endianness */
#ifndef BYTE_ORDER
#error "No BYTE_ORDER (LITTLE_ENDIAN / BIG_ENDIAN) specified!"
#endif
//#ifndef BYTE_ORDER
//#define BYTE_ORDER LITTLE_ENDIAN
//#define BYTE_ORDER BIG_ENDIAN
//#endif /* BYTE_ORDER */

/* Define generic types used in lwIP */
typedef unsigned   char    u8_t;
typedef signed     char    s8_t;
typedef unsigned   short   u16_t;
typedef signed     short   s16_t;
typedef unsigned   int     u32_t;
typedef signed     int     s32_t;

typedef unsigned long mem_ptr_t;

/* Define (sn)printf formatters for these lwIP types */
#define U16_F "u"
#define S16_F "d"
#define X16_F "x"
#define U32_F "u"
#define S32_F "d"
#define X32_F "x"

/* If only we could use C99 and get %zu */
#if defined(__x86_64__)
#define SZT_F "lu"
#else
#define SZT_F "u"
#endif

/* Compiler hints for packing structures */
#define PACK_STRUCT_FIELD(x) x
#define PACK_STRUCT_STRUCT __attribute__((packed))
#define PACK_STRUCT_BEGIN
#define PACK_STRUCT_END

//#if LOG_ENABLE
//#if 1

/* Plaform specific diagnostic output */
#define LWIP_PLATFORM_DIAG(x)	printf x
//#define LWIP_PLATFORM_DIAG(x)


#define LWIP_PLATFORM_ASSERT(x) do { printf("Assertion \"%s\" failed at line %d in %s\n", x, __LINE__, __FILE__);} while(0)

#include "logger_config.hh"
#define LOGC(prefix,level,msg) if ((int) level <= (int) prefix) printf msg

/*#else

#define LWIP_PLATFORM_DIAG(x)

#define LWIP_PLATFORM_ASSERT(x) do {} while(0)
#endif*/

//#define LWIP_PLATFORM_ASSERT(x) libprintf x

#define LWIP_RAND() ((u32_t)rand())



#endif /* CC_H_ */
