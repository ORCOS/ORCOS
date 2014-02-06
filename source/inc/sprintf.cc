/*
 File: sprintf.cc

 Copyright (C) 2004  Kustaa Nyholm

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#include "sprintf.hh"
#include "stringtools.hh"

#define PAD_RIGHT 1
#define PAD_ZERO 2
/* the following should be enough for 32 bit int */
#define PRINT_BUF_LEN 12

// externals
extern "C" void putchar( char c ); //< puts the character to the standard output device
extern void itoa( int8 value, char* str, int base );

static void printchar( char **str, char c ) {
    if ( str ) {
        **str = c;
        ++( *str );
    }
    else
        (void) putchar( c );
}



static void prints( char **out, const char *string, int width, int pad ) {
    register char padchar = ' ';

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

void print( char **out, const char *format, va_list args ) {
    register int width;
    register int pad;
    char scr[ 2 ];

    for ( ; *format != 0; ++format ) {
        if ( *format == '%' ) {
            ++format;
            width = pad = 0;
            if ( *format == '\0' )
                break;
            if ( *format == '%' )
                goto out;
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
                register char *s = (char*) va_arg( args, int );
                prints( out, s ? s : "(null)", width, pad );
                continue;
            }
            if ( *format == 'd' ) {
                char print_buf[ PRINT_BUF_LEN ];
                itoa( va_arg( args, int ), print_buf, 10 );
                prints( out, print_buf, width, pad );
                continue;
            }
            if ( *format == 'u' ) {
                char print_buf[ PRINT_BUF_LEN ];
                itoa( va_arg( args, unsigned int ), print_buf, 10 );
                prints( out, print_buf, width, pad );
                continue;
            }
            if( *format == 'x' ) {
                char print_buf[ PRINT_BUF_LEN ];
                itoa( va_arg( args, unsigned int ), print_buf, 16 );
                prints( out, print_buf, width, pad );
             continue;
             }

            /* if( *format == 'X' ) {
             char print_buf[ PRINT_BUF_LEN ];
		     itoa( va_arg( args, int ), print_buf, 10 );
			 prints( out, print_buf, width, pad );
             prints (out,  va_arg( args, int ), 16, width, pad);
             continue;
             }
             if( *format == 'u' ) {
             prints (out,  va_arg( args, int ), 10, width, pad);
             continue;
             }*/
            if ( *format == 'c' ) {
                /* char are converted to int then pushed on the stack */
                scr[ 0 ] = (char) va_arg( args, int );
                scr[ 1 ] = '\0';
                prints( out, scr, width, pad );
                continue;
            }
        }
        else {
            out: printchar( out, *format );
        }
    }
    if ( out )
        **out = '\0';
    va_end( args );
}

#if 1
int printf( const char *format, ... ) {
    va_list args;
    va_start( args, format );
    print( 0, format, args );
    return (0);
}

extern "C" int puts(const char* s) {
	// TODO: directly print instead of going through printf
	printf(s);
	return (1);
}

int sprintf( char *out, const char *format, ... ) {
    va_list args;
    va_start( args, format );
    print( &out, format, args );
    return (0);
}
#endif
