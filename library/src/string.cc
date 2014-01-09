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

#include "./types.h"
#include "./defines.h"
#include <stdarg.h>  // SYSTEM include not provided by orcos!
#include "./orcos.hh"
#include "./string.hh"

#define PAD_RIGHT 1
#define PAD_ZERO 2
#define PRINT_BUF_LEN 12
#define ZEROPAD 1       /* pad with zero */
#define SIGN    2       /* unsigned/signed long */
#define PLUS    4       /* show plus */
#define SPACE   8       /* space if plus */
#define LEFT    16      /* left justified */
#define SPECIAL 32      /* 0x */
#define LARGE   64      /* use 'ABCDEF' instead of 'abcdef' */

void* __stack_chk_guard = (void*) 0xdeadbeaf;

extern"C" void __stack_chk_fail()
{
    while(1);
}



void strreverse( char* begin, char* end ) {
    char aux;
    while ( end > begin )
        aux = *end, *end-- = *begin, *begin++ = aux;
}

void itoa( int value, char* str, int base ) {
    static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char* wstr = str;
    int sign;

    // Validate base
    if ( base < 2 || base > 35 ) {
        *wstr = '\0';
        return;
    }

    // Take care of sign
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

int atoi(char *p) {
 int k = 0;
 while (*p) {
 k = (k<<3)+(k<<1)+(*p)-'0';
 p++;
 }
 return k;
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

extern "C" void print( char **out, const char *format, va_list args ) {
    register int width;
    register int pad;
    char scr[ 2 ];
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
            if( *format == 'x' ) {
                itoa( va_arg( args, int ), print_buf, 16 );
                prints( out, &print_buf[0], width, pad );
                continue;
            }
            if( *format == 'u' ) {
            	 itoa( va_arg( args, unsigned int ), print_buf, 10 );
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
    if ( out && *out )
        **out = '\0';

}


/**
 * \brief sprintf - Format a string and place it into a buffer
 *
 * \param out       The buffer to place the result into
 * \param format    The format string to use
 * \param ...       Arguments for the format string
 */
int sprintf( char *out, char *format, ... ) {
	char* orig_out = out;
    va_list args;
    va_start( args, format );
    print( &out, format, args );
    va_end(args);
    return (out - orig_out);
}



// create a temporary buffer on the stack
static char tmp[256];

int printf( char *format, ... )
{
	for (int i = 0; i < 256; i++)
		tmp[i] = 0;

    char* out = tmp;

    va_list args;
    va_start( args, format );
    print( &out, format, args );
    va_end(args);

    size_t size = strlen(tmp);

    printToStdOut(tmp,size);
    return (size);
}


/**
 * \brief memcpy - Copies a memory area into another
 *
 * \param dst0      The destination address
 * \param src0      The source address
 * \parem len0      The size to be copied
 */
void* memcpy( void* dst0, const void* src0, size_t len0 ) {
    char *dst = (char *) dst0;
    char *src = (char *) src0;

    void* save = dst0;

    while ( len0-- ) {
        *dst++ = *src++;
    }

    return save;
}

void* memset( void* ptr, char c, int n) {
    char* p = (char*) ptr;

    void* save = ptr;

    while (n--) {
        *p++ = c;
    }

    return save;
}

// newlib strcmp method
int strcmp( const char *s1, const char *s2) {
    while ( *s1 != '\0' && *s1 == *s2 ) {
        s1++;
        s2++;
    }

    return ( *(unsigned char *) s1 ) - ( *(unsigned char *) s2 );
}

/*!
 * \brief   Determine the length of a character array
 */
size_t strlen(const char * s){
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc)
        /* nothing */;
    return (sc - s);
}


int memcmp(const char* s1, const char* s2, int len) {
	for (int i = 0; i < len; i++) {
		if (s1[i] != s2[i]) return -1;
	}

	return 0;

}

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



extern "C" int puts(const char *s) {
	return (printToStdOut(s,strlen(s)));
}


