/*
 * string.hh
 *
 *  Created on: 09.06.2013
 *      Author: dbaldin
 */

#ifndef STRING_HH_
#define STRING_HH_

extern "C" int		sprintf( char *out, char *format, ... );

extern "C" int	 	printf( char *format, ... );

extern "C" int 		strcmp( const char *s1, char const *s2 );

extern "C" size_t 	strlen(const char * s);

extern "C" int 		strpos(const char* s1, const char* s2);

extern "C" char* 	strcat( char *s1, const char *s2 );

extern "C" char* 	strcpy( char *dst0, const char *src0 );

extern "C" char* 	strtok( char *s, const char *delim );

extern "C" void 	itoa( int value, char* str, int base );

extern "C" void 	uitoa( unsigned int value, char* str, int base );

extern "C" int 		atoi(char *p);

extern "C" int 		puts(const char* s);

extern "C" void* 	memcpy( void* dst0, const void* src0, size_t len0 );

extern "C" void* 	memset( void* ptr, char c, int n);

#endif /* STRING_HH_ */
