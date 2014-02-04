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

#define MODULO(a,b)  a % b

#if 1
void strreverse( char* begin, char* end ) {
    char aux;
    while ( end > begin )
        aux = *end, *end-- = *begin, *begin++ = aux;
}

void itoa( int8 value, char* str, int base ) {
    static char num[] = "0123456789abcdefghijklmnopqrstuvwxyz";
    char* wstr = str;
    int8 sign;

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
        *wstr++ = num[ MODULO(value , base) ];
    while ( value /= base );

    if ( sign < 0 )
        *wstr++ = '-';

    *wstr = '\0';

    // Reverse string
    strreverse( str, wstr - 1 );
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

/*!
 * \brief   Determine the length of a character array
 */
size_t strlen(const char * s){
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc)

    /* nothing */;
    return (sc - s);
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

    *s1 = 0;

    return s;
}

#endif


// newlib strcmp method
int strcmp( const char *s1, const char *s2, unint1 name_len ) {

	// max string length check if no length is given
	if (name_len == 0) name_len = 255;

    while ( *s1 != '\0' && *s1 == *s2 && name_len > 1) {
        s1++;
        s2++;
        name_len--;
    }

    return ( *(unsigned const char *) s1 ) - ( *(unsigned const char *) s2 );
}

int strpos(const char*s, char c) {
	const char *sc;

	for (sc = s; *sc != c; ++sc) {
		if (*sc == '\0') return -1;
	}

	/* nothing */;
	return (sc - s);
}

int strpos2(const char*s, char c) {
	const char *sc;

	for (sc = s; *sc != c; ++sc) {
		if (*sc == '\0') return (sc - s);
	}

	/* nothing */;
	return (sc - s);
}



