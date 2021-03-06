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

#include "orcos_types.h"
#include "defines.h"
#include <stdarg.h>  // SYSTEM include not provided by orcos!
#include "orcos.h"
#include <string.h>
#include <malloc.h>

#if 0
#define PAD_RIGHT       1
#define PAD_ZERO        2
#define PRINT_BUF_LEN   26
#define ZEROPAD         1       /* pad with zero */
#define SIGN            2       /* unsigned/signed long */
#define PLUS            4       /* show plus */
#define SPACE           8       /* space if plus */
#define LEFT            16      /* left justified */
#define SPECIAL         32      /* 0x */
#define LARGE           64      /* use 'ABCDEF' instead of 'abcdef' */

void* __stack_chk_guard = (void*) 0xdeadbeaf;


void strreverse( char* begin, char* end ) {
    char aux;
    while ( end > begin )
        aux = *end, *end-- = *begin, *begin++ = aux;
}

void uitoa( unsigned int value, char* str, int base ) {
    static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char* wstr = str;

    // Validate base
    if ( base < 2 || base > 35 ) {
        *wstr = '\0';
        return;
    }

    // Conversion. Number is reversed.
    do
        *wstr++ = num[ value % base ];
    while ( value /= base );

      *wstr = '\0';

    // Reverse string
    strreverse( str, wstr - 1 );
}


void ulltoa( unsigned long long value, char* str, int base ) {
    static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char* wstr = str;

    // Validate base
    if ( base < 2 || base > 35 ) {
        *wstr = '\0';
        return;
    }

    // Conversion. Number is reversed.
    do
        *wstr++ = num[ value % base ];
    while ( value /= base );

      *wstr = '\0';

    // Reverse string
    strreverse( str, wstr - 1 );
}

void lltoa( long long value, char* str, int base ) {
    static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char* wstr = str;
    int sign = 0;
    // for all other bases we interpret value as unsigned!
    if (base != 10) return (uitoa((unsigned int) value,str,base));

    // Validate base
    if ( base < 2 || base > 35 ) {
        *wstr = '\0';
        return;
    }


    if ( ( sign = value ) < 0 )
        value = -value;

    // Conversion. Number is reversed.
    do
        *wstr++ = num[ value % base ];
    while ( value /= base );

    if ( sign < 0 )
        *wstr++ = '-';

    *wstr = '\0';

    // Reverse string
    strreverse( str, wstr - 1 );
}



void itoa( int value, char* str, int base ) {
    static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char* wstr = str;
    int sign = 0;
    // for all other bases we interpret value as unsigned!
    if (base != 10) return (uitoa((unsigned int) value,str,base));

    // Validate base
    if ( base < 2 || base > 35 ) {
        *wstr = '\0';
        return;
    }


    if ( ( sign = value ) < 0 )
        value = -value;


    // Conversion. Number is reversed.
    do
        *wstr++ = num[ value % base ];
    while ( value /= base );

    if ( sign < 0 )
        *wstr++ = '-';

    *wstr = '\0';

    // Reverse string
    strreverse( str, wstr - 1 );
}




static void printchar( char **str, char c ) {
    if ( str && *str) {
        **str = c;
        ++( *str );
    }
}
static void prints( char **out, const char *string, int width, int pad ) {
    register char padchar = ' ';

    //if (width > 8) return;
    if (string == 0) return;
    if (out == 0) return;
    if (*out == 0) return;

    if ( width > 0 ) {
        register int len = 0;
        register const char *ptr;
        for ( ptr = string; *ptr; ++ptr )
            ++len;
        if ( len >= width )
            width = 0;
        else
            width -= len;
        if ( pad & PAD_ZERO )
            padchar = '0';
    }

    // pad to the left
    if ( !( pad & PAD_RIGHT ) ) {
        for ( ; width > 0; --width ) {
            printchar( out, padchar );
        }
    }

    for ( ; *string; ++string ) {
        printchar( out, *string );
    }

    for ( ; width > 0; --width ) {
        printchar( out, padchar );
    }

}

int print( char **out, const char *format, va_list args ) {
    register int width;
    register int pad;
    char scr[ 2 ];
    char* outIn = 0;
    if (out)
        outIn = *out;
    char print_buf[ PRINT_BUF_LEN ];

    for ( ; *format != 0; ++format ) {
        if ( *format == '%' ) {
            ++format;
            width = pad = 0;
            if ( *format == '\0' )
                break;
            if ( *format == '%' ) {
                printchar( out, *format );
                continue;
            }
            if ( *format == '-' ) {
                ++format;
                pad = PAD_RIGHT;
            }
            while ( *format == '0' ) {
                ++format;
                pad |= PAD_ZERO;
            }
            for ( ; *format >= '0' && *format <= '9'; ++format ) {
                width *= 10;
                width += *format - '0';
            }
            if ( *format == 's' ) {
                register char *s = va_arg( args, char* );
                prints( out, s ? s : "(null)", width, pad );
                continue;
            }
            if ( *format == 'd' ) {
                itoa( va_arg( args, int ), print_buf, 10 );
                prints( out,  &print_buf[0], width, pad );
                continue;
            }
            if ( *format == 'l' ) {
                format++;
                if ( *format == 'u' ) {
                    uitoa( va_arg( args, unsigned int ), print_buf, 10 );
                    prints( out,  &print_buf[0], width, pad );
                    continue;
                }
                if ( *format == 'd' ) {
                    itoa( va_arg( args, int ), print_buf, 10 );
                    prints( out,  &print_buf[0], width, pad );
                    continue;
                }
                if (*format == 'l') {
                    /* long long printing */
                    format++;
                    if (*format == 'u') {
                        ulltoa( va_arg( args, long long unsigned), print_buf, 10 );
                        prints( out,  &print_buf[0], width, pad );
                        continue;
                    }
                    if (*format == 'd') {
                        lltoa( va_arg( args, long long ), print_buf, 10 );
                        prints( out,  &print_buf[0], width, pad );
                        continue;
                    }
                }

            }

            if( *format == 'x' ) {
                uitoa( va_arg( args, unsigned int ), print_buf, 16 );
                prints( out, &print_buf[0], width, pad );
                continue;
            }
            if( *format == 'u' ) {
                 uitoa( va_arg( args, unsigned int ), print_buf, 10 );
                 prints( out,  &print_buf[0], width, pad );
                 continue;
            }
            if ( *format == 'c' ) {
                /* char are converted to int then pushed on the stack */
                scr[ 0 ] = va_arg( args, int );
                scr[ 1 ] = '\0';
                prints( out, scr, width, pad );
                continue;
            }
        }
        else {
            printchar( out, *format );
        }
    }
    if (out) {
          **out = '\0';
          return (*out - outIn);
      }

      return (0);

}


/**
 * \brief sprintf - Format a string and place it into a buffer
 *
 * \param out       The buffer to place the result into
 * \param format    The format string to use
 * \param ...       Arguments for the format string
 */
int sprintf( char *out, const char *format, ... ) {
    va_list args;
    va_start( args, format );
    int ret = print( &out, format, args );
    va_end(args);
    return (ret);
}



// create a temporary buffer on the stack
static char tmp[256];

int printf( const char *format, ... )
{
    char* out = tmp;
    va_list args;
    va_start( args, format );
    int ret = print( &out, format, args );
    va_end(args);
    printToStdOut(tmp, ret);
    return (ret);
}

#endif

#if 0
/**
 * \brief memcpy - Copies a memory area into another
 *
 * \param dst0      The destination address
 * \param src0      The source address
 * \parem len0      The size to be copied
 */
extern"C" void* memcpy( void* dst0, const void* src0, size_t len0 ) {
    char *dst = (char *) dst0;
    char *src = (char *) src0;

    void* save = dst0;

    while ( len0-- ) {
        *dst++ = *src++;
    }

    return (save);
}

extern"C" void* memset( void* ptr, int c, size_t n) {
    char* p = (char*) ptr;

    void* save = ptr;

    while (n--) {
        *p++ = c;
    }

    return (save);
}

// newlib strcmp method
extern"C" int strcmp( const char *s1, const char *s2) {
    while ( *s1 != '\0' && *s1 == *s2 ) {
        s1++;
        s2++;
    }

    return (( *(unsigned char *) s1 ) - ( *(unsigned char *) s2 ));
}


/*!
 * \brief   Determine the length of a character array
 */
extern"C" size_t strlen(const char * s){
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc)
        /* nothing */;
    return (sc - s);
}


extern"C" int memcmp(const char* s1, const char* s2, int len) {
    for (int i = 0; i < len; i++) {
        if (s1[i] != s2[i]) return (-1);
    }

    return (0);

}
#endif
/*
 * Returns the position of s1 inside s2
 */
int strpos(const char* s1, const char* s2) {
    int s1_len = strlen(s1);
    int s2_len = strlen(s2);

    for (int i = 0; i <= s2_len - s1_len; i++) {
        if ((memcmp(&s2[i],s1,s1_len)) == 0) return i;
    }

    return -1;
}

#if 0
char* strcat( char *s1, const char *s2 ) {
    char *s = s1;

    // find end of the first string
    while ( *s1 )
        s1++;

    // append
    do {
        *s1 = *s2;
        s1++;
        s2++;
    }
    while ( *s2 );

    *s1 = '\0';

    return s;
}

// newlib strcpy method
char* strcpy( char *dst0, const char *src0 ) {
    char *s = dst0;

    while ( *src0 ) {
        *dst0 = *src0;
        dst0++;
        src0++;
    };

    // copy ending 0
    *dst0 = *src0;

    return s;
}


char* strtok( char *s, const char *delim ) {
    const char *spanp;
    int c, sc;
    char *tok;
    static char *last;

    if ( s == 0 && ( s = last ) == 0 )
        return ( 0 );

    /*
     * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
     */
    cont: c = *s++;
    for ( spanp = delim; ( sc = *spanp++ ) != 0; ) {
        if ( c == sc )
            goto cont;
    }

    if ( c == 0 ) { // no non-delimiter characters
        last = 0;
        return ( 0 );
    }
    tok = s - 1;

    /*
     * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
     * Note that delim must have one NUL; we stop if we see that, too.
     */
    for ( ;; ) {
        c = *s++;
        spanp = delim;
        do {
            if ( ( sc = *spanp++ ) == c ) {

                if ( c == 0 )
                    s = 0;
                else
                    s[ -1 ] = 0;

                last = s;
                return ( tok );
            }
        }
        while ( sc != 0 );
    }
    /* NOTREACHED */
}


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


char* strdup (const char *s)
{
    size_t len = strlen (s) + 1;
    void *newstr = malloc (len);

    if (newstr == NULL)
        return NULL;

    return (char *) memcpy (newstr, s, len);
}
#endif

void strnlower(char* pstr, int count) {
    for ( char *p = pstr; *p && count; ++p ) {
        *p= *p >= 'A' && *p <= 'Z' ? *p |0x60 : *p;
        count--;
    }
}


int strmatch(const char *wild, const char *string) {
  const char *cp = NULL, *mp = NULL;

  while ((*string) && (*wild != '*')) {
    if ((*wild != *string) && (*wild != '?')) {
      return (0);
    }
    wild++;
    string++;
  }

  while (*string) {
    if (*wild == '*') {
      if (!*++wild) {
        return (1);
      }
      mp = wild;
      cp = string+1;
    } else if ((*wild == *string) || (*wild == '?')) {
      wild++;
      string++;
    } else {
      wild = mp;
      string = cp++;
    }
  }

  while (*wild == '*') {
    wild++;
  }
  return (!*wild);
}

int writeStdOut(char* msg, int len);

int puts(const char *s) {
    return (writeStdOut(s, strlen(s)));
}


int isspace(int c)
{
        return ((c>=0x09 && c<=0x0D) || (c==0x20));
}

char* trim(const char * s) {
    int l = strlen(s);
    if (l == 0) {
        return (strdup(s));
    }

    while(isspace(s[l - 1])) --l;
    while(* s && isspace(* s)) ++s, --l;

    return (strndup(s, l));
}


static char ret_dirname[200];

char* dirname(const char *path) {
    strncpy(ret_dirname, path, 200);
    char *ret = ret_dirname;

    char *name = ret;
    char* last_slash = 0;
    while (*name) {
        if (*name == '/')  {
            last_slash = name;
        }
        name++;
    }
    if (last_slash && last_slash != ret) {
        last_slash[0] = 0;
    }
    if (last_slash == 0) {
        ret[0] = '.';
        ret[1] = 0;
    }
    return (ret);
}

char* basename(const char *name) {
  const char *base = name;

  while (*name) {
      if (*name++ == '/')  {
          base = name;
      }
  }
  return ((char*)(base));
}


typedef struct {
    int   num;
    char* errorstring;
} errorstrtable_t;

errorstrtable_t errorstrings[] = {
  {cOk,                     "No Error occurred."},
  {cError,                  "Unspecified error occurred (-1000)"},
  {cNotImplemented,         "Function not implemented (-1001)"},
  {cNullPointerProvided,    "Null pointer provided (-1002)"},
  {cStackOverflow,          "Stack overflow (-1003)"},
  {cStackUnderflow,         "Stack underflow (-1004)"},
  {cWrongAlignment,         "Wrong alignment (-1005)"},
  {cResourceNotRemovable,   "Resource not removable (-1006)"},
  {cWrongResourceType,      "Wrong Resource Type (-1007)"},
  {cNoData,                 "No data (-1008)"},
  {cResourceAlreadyExists,  "Resource already exists (-1009)"},
  {cResourceRemoved,        "Resource has been removed (-1010)"},
  {cInvalidResourceType,    "Invalid Resource Type (-1011)"},
  {cTransactionFailed,      "Transaction failed (-1012)"},
  {cResourceNotOwned,       "Resource not owned (-1013)"},
  {cResourceNotWriteable,   "Resource not writeable (-1014)"},
  {cResourceNotReadable,    "Resource not readable (-1015)"},
  {cInvalidResource,        "Invalid Resource (-1016)"},
  {cCanNotAquireResource,   "Resource can not be acquired (-1017)"},
  {cFileNotFound,           "File can not be found (-1018)"},
  {cArrayLengthOutOfBounds, "Array out of bounds (-800)"},
  {cWrongArrayLengthByte,   "Wrong array length (-801)"},
  {cEOF,                    "End of File (-5)"},
  {cInvalidPath,            "Invalid Path (-6)"},
  {cUnknownCmdOrPath,       "Command or File not found (-7)"},
  {cHeapMemoryExhausted,    "Heap Memory exhausted (-100)"},
  {cDeviceMemoryExhausted,  "Device Memory exhausted (-103)"},
  {cInvalidCBHeader,        "Invalid Task Control Block Header (-300)"},
  {cTaskCRCFailed,          "Task CRC check failed (-301)"},
  {cThreadNotFound,         "Thread not found (-302)"},
  {cNotConnected,           "Operation failed: Not connected (-400)"},
  {cTransportProtocolNotAvailable,   "Transport Protocol not available (-401)"},
  {cAddressProtocolNotAvailable,     "Address Protocol not available (-402)"},
  {cSocketAlreadyBoundError,"Socket already bound (-403)"},
  {cTCPEnqueueFailed,       "TCP enqueue failed. Buffer full. (-404)"},
  {cPBufNoMoreMemory,       "No more pbufs available (-405)"},
  {cDatabaseOverflow,       "Database overflow (-500)"},
  {cElementNotInDatabase,   "Element not in database (-501)"},
  {cIndexOutOfBounds,       "Index out of bounds (-502)"},
  {cInvalidArgument,        "Invalid argument (-600)"},
  {cBlockDeviceReadError,   "Block device read error (-700)"},
  {cBlockDeviceWriteError,  "Block device write error (-701)"},
  {cBlockDeviceTooManyBlocks,  "Block device: Too many blocks given (-702)"},
  {cTimeout,                "Operation timed out (-1019)"},
  {cInvalidSocketType,      "Invalid socket type for operation (-407)"},
  {cErrorConnecting,        "Error connecting (-408)"},
  {cInvalidConcurrentAccess,"Invalid concurrent access. Another thread already performs this operation (-1020)"},
  {cErrorBindingPort,       "Could not bind to port (-409)"},
  {cErrorAllocatingMemory,  "Error allocating memory (-105)"},
};


char* strerror(int errornum) {

    int i = 0;
    int errors = sizeof(errorstrings) / sizeof(errorstrtable_t);
    for (i = 0; i < errors; i++) {
        if (errorstrings[i].num == errornum) {
            return (errorstrings[i].errorstring);
        }
    }

    return ("Unknown error number");
}



void bzero(void *b, size_t length) {
  char *ptr = (char *)b;
  while (length--)
    *ptr++ = 0;
}

#if 0

char*    strchr(const char* s, int c) {
  while (*s && *s != c)
    s++;

  if (*s == c)
    return (char *)s;

  return NULL;
}



char *strpbrk(const char *s1, const char *s2) {
        const  char *c = s2;
        if (!*s1) {
                return (char *) NULL;
        }

        while (*s1) {
                for (c = s2; *c; c++) {
                        if (*s1 == *c)
                                break;
                }
                if (*c)
                        break;
                s1++;
        }

        if (*c == '\0')
                s1 = NULL;

        return (char *) s1;
}

#endif

void __stack_chk_fail()
{
    puts("Stack check failed!" LINEFEED);
    while(1);
}

