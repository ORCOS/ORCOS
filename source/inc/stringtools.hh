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

#ifndef STRINGTOOLS_H_
#define STRINGTOOLS_H_

#include "inc/types.hh"

#ifdef __cplusplus
extern "C" {
#endif
/*!
 * \brief reverses the given string.
 */
void strreverse( char* begin, char* end );

/*!
 \brief Convert integer to string (non-standard function).

 Parameters

 value - Value to be converted to a string.
 str - Array in memory where to store the resulting null-terminated string.
 base - Numerical base used to represent the value as a string.
 */
void itoa( int value, char* str, int base );

void uitoa( unsigned int value, char* str, int base );

char* strcpy( char *dst0, char const *src0 );

int strcmp2( const char *s1, char const *s2, unint1 s1_len);

char* strtok( char *s, char const *delim );

char* strcat( char *s1, const char *s2 );

size_t strlen(const char * s);

int strpos(const char*s, char c);

int strpos2(const char*s, char c);

#ifdef __cplusplus
}
#endif

#endif /*STRINGTOOLS_H_*/
