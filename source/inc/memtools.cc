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

#include <memtools.hh>
#include "inc/memio.h"
#include "sprintf.hh"
#include "inc/error.hh"
#include "archtypes.h"

/* weak definitions as they are supposed to be overriden by arch implementations
 * for better speed */
void* memcpy(void* dst0, const void* src0, size_t len0) __attribute__((weak));

/* Nonzero if either X or Y is not aligned on a "long" boundary.  */
#define UNALIGNED(X, Y) \
  (((long)X & (sizeof (long) - 1)) | ((long)Y & (sizeof (long) - 1)))

/* How many bytes are copied each iteration of the 4X unrolled loop.  */
#define BIGBLOCKSIZE    (sizeof (long) << 2)

/* How many bytes are copied each iteration of the word copy loop.  */
#define LITTLEBLOCKSIZE (sizeof (long))

/* Threshhold for punting to the byte copier.  */
#define TOO_SMALL(LEN)  ((LEN) < BIGBLOCKSIZE)

/*****************************************************************************
 * Method: memcpy(void* dst0, const void* src0, size_t len0)
 *
 * @description
 * Copies len0 bytes from src0 to dst0. If the addresses are word aligned
 * word copy will be used if len0 is sufficiently large
 *
 * @returns
 *  void*       dst0
 *******************************************************************************/
void* memcpy(void* __restrict dst0, const void* __restrict src0, size_t len0) {
    /*char *dst = reinterpret_cast<char *>(dst0);
    char *src = const_cast<char *>((const char*) src0);

    void* save = dst0;
    while (len0--) {
        *dst = *src;
        dst++;
        src++;
    }

    return (save);*/

    char *dst       = (char*) dst0;
    const char *src = (const char*) src0;
    long *aligned_dst;
    const long *aligned_src;

    /* If the size is small, or either SRC or DST is unaligned,
       then punt into the byte copy loop.  This should be rare.  */
    if (!TOO_SMALL(len0) && !UNALIGNED (src, dst))
    {
        aligned_dst = (long*)dst;
        aligned_src = (long*)src;

        /* Copy 4X long words at a time if possible.  */
        while (len0 >= BIGBLOCKSIZE)
        {
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            *aligned_dst++ = *aligned_src++;
            len0 -= BIGBLOCKSIZE;
        }

        /* Copy one long word at a time if possible.  */
        while (len0 >= LITTLEBLOCKSIZE)
        {
            *aligned_dst++ = *aligned_src++;
            len0 -= LITTLEBLOCKSIZE;
        }

         /* Pick up any residual with a byte copier.  */
        dst = (char*)aligned_dst;
        src = (char*)aligned_src;
    }

    /* copy residual */
    while (len0--)
      *dst++ = *src++;

    return (dst0);
}

/*****************************************************************************
 * Method: memcpyl(void* dst0, const void* src0, size_t len0)
 *
 * @description
 *  Performs memory copy from src0 to dst0 using word transfers. len0
 *  must be a multiple of the word length.
 * @returns
 *  void*       dst0
 *******************************************************************************/
void* memcpyl(void* dst0, const void* src0, size_t len0) {
   long *dst = reinterpret_cast<long *>(dst0);
   long *src = const_cast<long *>((const long*) src0);

   void* save = dst0;
   long len   = len0;

   while (len > 0) {
       *dst = *src;
       dst++;
       src++;
       len -= sizeof(long);
   }

   return (save);
}

/*****************************************************************************
 * Method: memcmp(const void* m1, const void* m2, size_t n)
 *
 * @description
 *
 *******************************************************************************/
int memcmp(const void* m1, const void* m2, size_t n) {
    unsigned char *s1 = const_cast<unsigned char *>((const unsigned char*) m1);
    unsigned char *s2 = const_cast<unsigned char *>((const unsigned char*) m2);

    while (n--) {
        if (*s1 != *s2) {
            return (*s1 - *s2);
        }
        s1++;
        s2++;
    }
    return (0);
}


/*****************************************************************************
 * Method: memset(void* ptr, int c, size_t n)
 *
 * @description
 *
 *******************************************************************************/
void* memset(void* ptr, int c, size_t n) {
    char* p = reinterpret_cast<char*>(ptr);

    void* save = ptr;

    while (n--) {
        *p++ = (unsigned char) c;
    }

    return (save);
}

/*****************************************************************************
 * Method: memsetlong(void* ptr, int c, size_t n)
 *
 * @description
 *
 *******************************************************************************/
void* memsetlong(void* ptr, int c, size_t n) {
    int* p = reinterpret_cast<int*>(ptr);

    void* save = ptr;

    while (n--) {
        *p++ = c;
    }

    return (save);
}

#if 1
/*****************************************************************************
 * Method: makeHexCharCompatible(char* msg, int len)
 *
 * @description
 *  Replaces all non representable characters by '.'
 *******************************************************************************/
void makeHexCharCompatible(char* msg, int len) {
    for (int i = 0; i < len; i++) {
        if (msg[i] < 32)
            msg[i] = '.';
        if (msg[i] > 126)
            msg[i] = '.';
    }
}

/*****************************************************************************
 * Method: memdump(int addr, int length)
 *
 * @description
 *
 *******************************************************************************/
void memdump(void* address, int length) {
    int addr = (int) address;
    for (int i = 0; i < length; i+=8) {
#if PROGRMEM
        printf_p(PSTR("0x%x: "),     static_cast<int>(addr + i));
        printf_p(PSTR("0x%2x "),     static_cast<int>(INB(addr + i)));
        printf_p(PSTR("0x%2x "),     static_cast<int>(INB(addr + i + 1)));
        printf_p(PSTR("0x%2x "),     static_cast<int>(INB(addr + i + 2)));
        printf_p(PSTR("0x%2x \r\n"), static_cast<int>(INB(addr + i + 3)));
        printf_p(PSTR("0x%2x "),     static_cast<int>(INB(addr + i + 4)));
        printf_p(PSTR("0x%2x "),     static_cast<int>(INB(addr + i + 5)));
        printf_p(PSTR("0x%2x "),     static_cast<int>(INB(addr + i + 6)));
        printf_p(PSTR("0x%2x \r\n"), static_cast<int>(INB(addr + i + 7)));
#else
        printf("0x%x: ",  static_cast<int>(addr + i));
        printf("0x%2x ",  static_cast<int>(INB(addr + i)));
        printf("0x%2x ",  static_cast<int>(INB(addr + i + 1)));
        printf("0x%2x ",  static_cast<int>(INB(addr + i + 2)));
        printf("0x%2x ",  static_cast<int>(INB(addr + i + 3)));
        printf("0x%2x ",  static_cast<int>(INB(addr + i + 4)));
        printf("0x%2x ",  static_cast<int>(INB(addr + i + 5)));
        printf("0x%2x ",  static_cast<int>(INB(addr + i + 6)));
        printf("0x%2x ",  static_cast<int>(INB(addr + i + 7)));
        char ascii[9];
        ascii[0] = INB(addr + i);
        ascii[1] = INB(addr + i + 1);
        ascii[2] = INB(addr + i + 2);
        ascii[3] = INB(addr + i + 3);
        ascii[4] = INB(addr + i + 4);
        ascii[5] = INB(addr + i + 5);
        ascii[6] = INB(addr + i + 6);
        ascii[7] = INB(addr + i + 7);
        ascii[8] = 0;
        makeHexCharCompatible(ascii, 8);
        printf(" %s\r\n", ascii);
#endif
    }
}

#endif
