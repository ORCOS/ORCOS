/*
 * string.hh
 *
 *  Created on: 09.06.2013
 *      Author: dbaldin
 */

#ifndef STRING_HH_
#define STRING_HH_

extern "C" int 		sprintf( char *out, const char *format, ... );

extern "C" int	 	printf( const char *format, ... );

extern "C" int 		strcmp( const char *s1, char const *s2 );

extern "C" size_t 	strlen(const char * s);

extern "C" int 		strpos(const char* s1, const char* s2);

extern "C" char* 	strcat( char *s1, const char *s2 );

extern "C" char* 	strcpy( char *dst0, const char *src0 );

extern "C" char* 	strtok( char *s, const char *delim );

extern "C" void 	itoa( int value, char* str, int base );

extern "C" int 		atoi(char *p);

#endif /* STRING_HH_ */
