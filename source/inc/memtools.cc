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
#include "memio.h"
#include "sprintf.hh"

void* memcpy( void* dst0, const void* src0, size_t len0 ) {
    char *dst = (char *) dst0;
    char *src = const_cast<char *>( (const char*) src0);

    void* save = dst0;

    while ( len0-- ) {
        //*dst++ = *src++;
        *dst = *src;
        dst++;
        src++;
    }

    return (save);
}

int memcmp( const void* m1, const void* m2, size_t n ) {
    unsigned char *s1 = const_cast<unsigned char *>( (const unsigned char*) m1);
    unsigned char *s2 = const_cast<unsigned char *>( (const unsigned char*) m2);

    while ( n-- ) {
        if ( *s1 != *s2 ) {
            return (*s1 - *s2);
        }
        s1++;
        s2++;
    }
    return (0);
}

void* memset( void* ptr, int c, size_t n ) {
    char* p = (char*) ptr;

    void* save = ptr;

    while ( n-- ) {
        *p++ = (unsigned char) c;
    }

    return (save);
}

void* memsetlong( void* ptr, int c, size_t n ) {
    int* p = (int*) ptr;

    void* save = ptr;

    while ( n-- ) {
        *p++ = c;
    }

    return (save);
}

#if 1
void makeHexCharCompatible(char* msg, int len) {
	for (int i= 0; i < len; i++) {
		if (msg[i] < 32)  msg[i] = '.';
		if (msg[i] > 126) msg[i] = '.';
	}
}


void memdump(int addr,int length) {

	for (int i = 0 ; i < length ;i++) {
		printf("0x%x: ",addr + (4*i));
		printf("0x%2x ",INB(addr + (4*i)));
		printf("0x%2x ",INB(addr + (4*i) +1));
		printf("0x%2x ",INB(addr + (4*i) +2));
		printf("0x%2x ",INB(addr + (4*i) +3));

		char ascii[5];
		ascii[0] = INB(addr + (4*i));
		ascii[1] = INB(addr + (4*i) +1);
		ascii[2] = INB(addr + (4*i) +2);
		ascii[3] = INB(addr + (4*i) +3);
		ascii[4] = 0;
		makeHexCharCompatible(ascii,4);
		printf("\t%s\r",ascii);


	}



}
#endif

//char null_buffer[4] = {0,0,0,0};
