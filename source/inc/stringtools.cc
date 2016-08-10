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

#include <stringtools.hh>

#define MODULO(a, b)  a % b


/*****************************************************************************
 * Method: strreverse(char* begin, char* end)
 *
 * @description
 *
 *******************************************************************************/
void strreverse(char* begin, char* end) {
    char aux;
    while (end > begin)
        aux = *end, *end-- = *begin, *begin++ = aux;
}

static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";

/*****************************************************************************
 * Method: uitoa(unsigned int value, char* str, int base)
 *
 * @description
 *  Unsigned Integer to ascii conversion with the given numerical base system
 *
 * @params
 *
 *******************************************************************************/
void uitoa(unsigned int value, char* str, int base) {
    char* wstr = str;

    // Validate base
    if (base < 2 || base > 35) {
        *wstr = '\0';
        return;
    }

    // Conversion. Number is reversed.
    do {
        *wstr++ = num[value % base];
    } while (value /= base);

    *wstr = '\0';

    // Reverse string
    strreverse(str, wstr - 1);
}

/*****************************************************************************
 * Method: itoa(int value, char* str, int base)
 *
 * @description
 *  Integer to ascii conversion with the given numerical base system
 *
 * @params
 *
 *******************************************************************************/
void itoa(int value, char* str, int base) {
    char* wstr = str;
    int8 sign;

    // Validate base
    if (base < 2 || base > 35) {
        *wstr = '\0';
        return;
    }

    // Take care of sign
    if ((sign = value) < 0)
        value = -value;

    // Conversion. Number is reversed.
    do {
        *wstr++ = num[MODULO(value, base)];
    } while (value /= base);

    if (sign < 0)
        *wstr++ = '-';

    *wstr = '\0';

    // Reverse string
    strreverse(str, wstr - 1);
}

/*****************************************************************************
 * Method: strcpy(char *dst0, const char *src0)
 *
 * @description
 *  newlib strcpy method
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
char* strcpy(char *dst0, const char *src0) {
    char *s = dst0;

    while (*src0) {
        *dst0 = *src0;
        dst0++;
        src0++;
    }

    /* copy tailing 0 */
    *dst0 = *src0;

    return (s);
}

/*****************************************************************************
 * Method: strncpy(char *dst0, const char *src0, size_t maxChars)
 *
 * @description
 *  Copies src0 to dst0, however limiting the string to maxChars if
 *  src0 is longer.
 * @params
 *
 * @returns
 *******************************************************************************/
char* strncpy(char *dst0, const char *src0, size_t maxChars) {
    char *s = dst0;

    while (*src0 && (maxChars > 0)) {
        *dst0 = *src0;
        dst0++;
        src0++;
        maxChars--;
    }

    // copy ending 0
    if (maxChars)
        *dst0 = 0;

    return (s);
}

/*****************************************************************************
 * Method: strtok(char *s, const char *delim)
 *
 * @description
 *
 * @params
 *
 * @returns
 *******************************************************************************/
char* strtok(char *s, const char *delim) {
    const char *spanp;
    int c, sc;
    char *tok;
    static char *last;

    if (s == 0 && (s = last) == 0)
        return (0);

    /*
     * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
     */
    cont: c = *s++;
    for (spanp = delim; (sc = *spanp++) != 0;) {
        if (c == sc)
            goto cont;
    }

    if (c == 0) {  // no non-delimiter characters
        last = 0;
        return (0);
    }
    tok = s - 1;

    /*
     * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     * Note that delim must have one NUL; we stop if we see that, too.
     */
    for (;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = 0;
                else
                    s[-1] = 0;

                last = s;
                return (tok);
            }
        } while (sc != 0);
    }
    /* NOTREACHED */
}

/*****************************************************************************
 * Method: strlen(const char * s)
 *
 * @description
 *   Determine the length of the given null terminated string
 *
 * @params
 *  s           pointer to the null terminated string
 *
 * @returns
 *  size_t      length
 *******************************************************************************/
#if 0
size_t strlen(const char * s) {
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc) {
    }

    /* nothing */
    return (sc - s);
}
#else
/* Return the length of the null-terminated string STR.  Scan for
   the null terminator quickly by testing four bytes at a time.  */
size_t strlen (const char *str) {
  const char *char_ptr;
  const unsigned long int *longword_ptr;
  //unsigned long int  magic_bits;
  unsigned long int longword, himagic, lomagic;

  /* Handle the first few characters by reading one character at a time.
     Do this until CHAR_PTR is aligned on a longword boundary.  */
  for (char_ptr = str; ((unsigned long int) char_ptr
                        & (sizeof (longword) - 1)) != 0;
       ++char_ptr)
    if (*char_ptr == '\0')
      return char_ptr - str;

  /* All these elucidatory comments refer to 4-byte longwords,
     but the theory applies equally well to 8-byte longwords.  */

  longword_ptr = (unsigned long int *) char_ptr;

  /* Bits 31, 24, 16, and 8 of this number are zero.  Call these bits
     the "holes."  Note that there is a hole just to the left of
     each byte, with an extra at the end:

     bits:  01111110 11111110 11111110 11111111
     bytes: AAAAAAAA BBBBBBBB CCCCCCCC DDDDDDDD

     The 1-bits make sure that carries propagate to the next 0-bit.
     The 0-bits provide holes for carries to fall into.  */
  //magic_bits = 0x7efefeffL;
  himagic = 0x80808080L;
  lomagic = 0x01010101L;
  if (sizeof (longword) > 4)
  {
      /* 64-bit version of the magic.  */
      /* Do the shift in two steps to avoid a warning if long has 32 bits.  */
      //magic_bits = ((0x7efefefeL << 16) << 16) | 0xfefefeffL;
      himagic = ((himagic << 16) << 16) | himagic;
      lomagic = ((lomagic << 16) << 16) | lomagic;
  }

  /* Instead of the traditional loop which tests each character,
     we will test a longword at a time.  The tricky part is testing
     if *any of the four* bytes in the longword in question are zero.  */
  for (;;)
    {
      /* We tentatively exit the loop if adding MAGIC_BITS to
         LONGWORD fails to change any of the hole bits of LONGWORD.

         1) Is this safe?  Will it catch all the zero bytes?
         Suppose there is a byte with all zeros.  Any carry bits
         propagating from its left will fall into the hole at its
         least significant bit and stop.  Since there will be no
         carry from its most significant bit, the LSB of the
         byte to the left will be unchanged, and the zero will be
         detected.

         2) Is this worthwhile?  Will it ignore everything except
         zero bytes?  Suppose every byte of LONGWORD has a bit set
         somewhere.  There will be a carry into bit 8.  If bit 8
         is set, this will carry into bit 16.  If bit 8 is clear,
         one of bits 9-15 must be set, so there will be a carry
         into bit 16.  Similarly, there will be a carry into bit
         24.  If one of bits 24-30 is set, there will be a carry
         into bit 31, so all of the hole bits will be changed.

         The one misfire occurs when bits 24-30 are clear and bit
         31 is set; in this case, the hole at bit 31 is not
         changed.  If we had access to the processor carry flag,
         we could close this loophole by putting the fourth hole
         at bit 32!

         So it ignores everything except 128's, when they're aligned
         properly.  */

      longword = *longword_ptr++;

      if (
#if 0
          /* Add MAGIC_BITS to LONGWORD.  */
          (((longword + magic_bits)

            /* Set those bits that were unchanged by the addition.  */
            ^ ~longword)

           /* Look at only the hole bits.  If any of the hole bits
              are unchanged, most likely one of the bytes was a
              zero.  */
           & ~magic_bits)
#else
          ((longword - lomagic) & himagic)
#endif
          != 0)
        {
          /* Which of the bytes was the zero?  If none of them were, it was
             a misfire; continue the search.  */

          const char *cp = (const char *) (longword_ptr - 1);

          if (cp[0] == 0)
            return cp - str;
          if (cp[1] == 0)
            return cp - str + 1;
          if (cp[2] == 0)
            return cp - str + 2;
          if (cp[3] == 0)
            return cp - str + 3;
          if (sizeof (longword) > 4)
            {
              if (cp[4] == 0)
                return cp - str + 4;
              if (cp[5] == 0)
                return cp - str + 5;
              if (cp[6] == 0)
                return cp - str + 6;
              if (cp[7] == 0)
                return cp - str + 7;
            }
        }
    }
}

#endif

/*****************************************************************************
 * Method: strcat(char *s1, const char *s2)
 *
 * @description
 *   Concatenates s2 to s1.
 *
 * @params
 *  s1          first string which will be concatenated with s2
 *  s2          seconds string which is to be concatenated
 * @returns

 *******************************************************************************/
char* strcat(char *s1, const char *s2) {
    char *s = s1;

    /* find end of the first string */
    while (*s1)
        s1++;

    /* append */
    do {
        *s1 = *s2;
        s1++;
        s2++;
    } while (*s2);

    *s1 = 0;

    return (s);
}

char* strncat(char *dst, const char *src, register size_t n)
{
    if (n != 0) {
        register char *d = dst;
        register const char *s = src;

        while (*d != 0)
            d++;
        do {
            if ((*d = *s++) == 0)
                break;
            d++;
        } while (--n != 0);
        *d = 0;
    }
    return (dst);
}


/*****************************************************************************
 * Method: strcmp(const char *s1, const char *s2)
 *
 * @description
 *  newlib strcmp method
 *
 * @params
 *  s1          First null terminated string
 *  s2          Second null terminated string
 * @returns
 *  int         0 if equal.
 *******************************************************************************/
int strcmp(const char *s1, const char *s2) {
    while (*s1 != '\0' && *s1 == *s2) {
        s1++;
        s2++;
    }

    return ((*(unsigned char *) s1) - (*(unsigned char *) s2));
}

/*****************************************************************************
 * Method: strncmp(const char *s1, const char *s2, unint1 name_len)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int
 *******************************************************************************/
int strncmp(const char *s1, const char *s2, size_t name_len) {
    /* max string length check if no length is given */
    if (name_len == 0)
        name_len = 255;

    while (*s1 != 0 && *s1 == *s2 && name_len > 1) {
        s1++;
        s2++;
        name_len--;
    }

    return ((*(unsigned const char *) s1) - (*(unsigned const char *) s2));
}

/*****************************************************************************
 * Method: strpos(const char*s, char c)
 *
 * @description
 *
 * @params
 *
 * @returns
 *******************************************************************************/
int strpos(const char*s, char c) {
    const char *sc;

    for (sc = s; *sc != c; ++sc) {
        if (*sc == '\0')
            return -1;
    }

    return (sc - s);
}

/*****************************************************************************
 * Method: strpos2(const char*s, char c)
 *
 * @description
 *
 * @params
 *
 * @returns
 *******************************************************************************/
int strpos2(const char*s, char c) {
    const char *sc;

    for (sc = s; *sc != c; ++sc) {
        if (*sc == '\0')
            return (sc - s);
    }

    return (sc - s);
}

/*****************************************************************************
 * Method: ascii2unicode(const char * szAscii, char* szUnicode, unint2 len)
 *
 * @description
 *
 * @params
 *
 * @returns
 *******************************************************************************/
int ascii2unicode(const char * szAscii, char* szUnicode, unint2 len) {
    int i;
    if ((szUnicode == 0) || (szAscii == 0))
        return (false);

    for (i = 0; i < len; i++) {
        char code = *szAscii++;
        *szUnicode++ = code;
        if (code != 0xff)
            *szUnicode++ = 0;
        else
            *szUnicode++ = 0xff;
    }

    return (true);
}

/*****************************************************************************
 * Method: unicode2ascii(const char* szUnicode, char* szAscii, unint2 len)
 *
 * @description
 *
 * @params
 *      len: length of the string (== length of the ascii string in bytes)
 * @returns
 *******************************************************************************/
int unicode2ascii(const char* szUnicode, char* szAscii, unint2 len) {
    int i;
    if ((szUnicode == 0) || (szAscii == 0))
        return (false);

    for (i = 0; i < len; i++) {
        *szAscii++ = *szUnicode++;
        szUnicode++;
    }

    return (true);
}

/*****************************************************************************
 * Method: strnlower(char* pstr)
 *
 * @description
 *
 * @params
 *
 * @returns
 *******************************************************************************/
void strnlower(char* pstr, int count) {
    for ( char *p = pstr; *p && count; ++p ) {
        *p= *p >= 'A' && *p <= 'Z' ? *p |0x60 : *p;
        count--;
    }
}


/*****************************************************************************
 * Method: basename(const char *name)
 *
 * @description
 *  Given a pointer to a string containing a typical pathname
 *   (/usr/src/cmd/ls/ls.c for example), returns a pointer to the
 *   last component of the pathname ("ls.c" in this case).
 *
 * @params
 *
 * @returns
 *******************************************************************************/
char* basename(const char *name) {
  const char *base = name;

  while (*name) {
      if (*name++ == '/')  {
          base = name;
      }
  }

  return (const_cast<char*>(base));
}

/*****************************************************************************
 * Method: strchrs(const char* pstr, char* characters)
 *
 * @description
 * Checks if any of the characters specifiec in 'characters' is
 * contained inside pstr.
 *
 * @params
 *   @param pstr        String to check
 *   @param characters  Set of characters to check for
 *
 * @returns
 *   0 : no character was found
 *   1 : a character was found
 *******************************************************************************/
int strchrs(const char* pstr, char* characters) {
    while (*characters) {
        if (strpos(pstr, *characters)) return (1);
        characters++;
    }

    return (0);
}
