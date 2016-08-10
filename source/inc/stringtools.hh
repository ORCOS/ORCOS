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

/*****************************************************************************
 * Method: strreverse(char* begin, char* end)
 *
 * @description
 *
 *******************************************************************************/
void strreverse(char* begin, char* end);


/*****************************************************************************
 * Method: itoa(int value, char* str, int base)
 *
 * @description
 *  Convert integer to string (non-standard function).
 *
 *  Parameters
 *
 *  value - Value to be converted to a string.
 *  str - Array in memory where to store the resulting null-terminated string.
 *  base - Numerical base used to represent the value as a string.
 *
 * @params
 *
 *******************************************************************************/
void itoa(int value, char* str, int base);

/*****************************************************************************
 * Method: uitoa(unsigned int value, char* str, int base)
 *
 * @description
 *  Unsigned Integer to ascii conversion with the given numerical base system
 *
 * @params
 *
 *******************************************************************************/
void uitoa(unsigned int value, char* str, int base);

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
char* strcpy(char *dst0, char const *src0);

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
char* strncpy(char *dst0, const char *src0, size_t maxChars);

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
int strncmp(const char *s1, char const *s2, size_t s1_len);

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
int strcmp(const char *s1, char const *s2);

/*****************************************************************************
 * Method: strtok(char *s, const char *delim)
 *
 * @description
 *
 * @params
 *
 * @returns
 *******************************************************************************/
char* strtok(char *s, char const *delim);

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
char* strcat(char *s1, const char *s2);

char* strncat(char *dst, const char *src, register size_t n);

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
size_t strlen(const char * s);

/*****************************************************************************
 * Method: strpos(const char*s, char c)
 *
 * @description
 *
 * @params
 *
 * @returns
 *******************************************************************************/
int strpos(const char*s, char c);

/*****************************************************************************
 * Method: strpos2(const char*s, char c)
 *
 * @description
 *
 * @params
 *
 * @returns
 *******************************************************************************/
int strpos2(const char*s, char c);

/*****************************************************************************
 * Method: ascii2unicode(const char * szAscii, char* szUnicode, unint2 len)
 *
 * @description
 *
 * @params
 *
 * @returns
 *******************************************************************************/
int ascii2unicode(const char * szAscii, char* szUnicode, unint2 len);


/*****************************************************************************
 * Method: unicode2ascii(const char* szUnicode, char* szAscii, unint2 len)
 *
 * @description
 *
 * @params
 *      len: length of the string (== length of the ascii string in bytes)
 * @returns
 *******************************************************************************/
int unicode2ascii(const char* szUnicode, char* szAscii, unint2 len);

/*****************************************************************************
 * Method: strnlower(char* pstr)
 *
 * @description
 *
 * @params
 *
 * @returns
 *******************************************************************************/
void strnlower(char* pstr, int count);

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
char* basename(const char *name);

#ifdef __cplusplus
}
#endif

#endif /*STRINGTOOLS_H_*/
