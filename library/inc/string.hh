/*
 * string.hh
 *
 *  Created on: 09.06.2013
 *      Author: dbaldin
 */

#ifndef STRING_HH_
#define STRING_HH_

#include <time.h>

extern "C" int      strpos(const char* s1, const char* s2);
extern "C" int      strmatch(const char *wildcard, const char *string);
extern "C" void     strnlower(char* pstr, int len);
extern "C" char*    dirname(const char *path);
extern "C" char*    basename(const char *path);
extern "C" void     time2date(struct tm* pTm, time_t seconds);

#if 0
extern "C" int      sprintf( char *out, const char *format, ... );

extern "C" int      printf( const char *format, ... );

extern "C" int      strcmp( const char *s1, char const *s2 );

extern "C" size_t   strlen(const char * s);

extern "C" int      strpos(const char* s1, const char* s2);

extern "C" char*    strcat( char *s1, const char *s2 );

extern "C" char*    strchr(const char* s1, int i);

extern "C" char*    strcpy( char *dst0, const char *src0 );

extern "C" char*    strncpy(char *dst, const char *src, size_t n);

extern "C" char*    strtok( char *s, const char *delim );

extern "C" char*    strdup (const char *s);

extern "C" void     strnlower(char* pstr, int len);

extern "C" char*    strpbrk(const char *s1, const char *s2) ;

extern "C" int      strmatch(const char *wildcard, const char *string);

extern "C" double   strtod(const char *str, char **endptr);

extern "C" float    strtof(const char *str, char **endptr);

extern "C" long double strtold(const char *str, char **endptr);

extern "C" double   atof(const char *str);

extern "C" void     itoa( int value, char* str, int base );

extern "C" void     uitoa( unsigned int value, char* str, int base );

extern "C" int      puts(const char* s);

extern "C" void*    memcpy( void* dst0, const void* src0, size_t len0 );

extern "C" void*    memset( void* ptr, int c, size_t n);

extern "C" char*    inet_ntop(int af, const char *src, char *dst, size_t size);

extern "C" void     bzero(void *b,size_t length);

extern "C" char*    dirname(const char *path);

extern "C" char*    basename(const char *path);

char* fgets (char *s, int count, int fd);
#endif

#endif /* STRING_HH_ */
