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
extern "C" {
#endif

typedef void (*charout)(char** str, char c);

void print(charout outfunc, char **out, const char *format, va_list args);

/*
 *  * Prints formatted string the given by format to standard out
 */
int printf(const char * format, ...);

/*
 * Prints formatted string the given by format using to the function provided using the parameter param
 * => each character c of the formatted result string will be put into out as out(param,c);
 */
int fprintf(charout out, char** param, const char *format, ...);

/*
 * Prints formatted string the given by format to the location pointed by str
 */
int sprintf(char * str, const char * format, ...);

/*
 * Prints the given string unformatted to standard out
 */
int puts(const char* s);

#ifdef __cplusplus
}
#endif
#endif
