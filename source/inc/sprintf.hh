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

#ifndef sprintf_hh
#define sprintf_hh

#include <stdarg.h>
//#include <kernel/Kernel.hh>

#ifdef __cplusplus
extern "C"
{
#endif

void print( char **out, const char *format, va_list args );

//void printf( const char *format, ... );
int printf (const char * format, ...);

//void sprintf( char *out, const char *format, ... );
int sprintf (char * str, const char * format, ...);

int puts(const char* s);
#ifdef __cplusplus
}
#endif
#endif
