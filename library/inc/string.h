/*
 * string.hh
 *
 *  Created on: 09.06.2013
 *      Author: dbaldin
 */

#include_next <string.h>

#ifndef STRING_HH_
#define STRING_HH_

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

int      isspace(int c);
char*    trim(const char * s);
int      strpos(const char* s1, const char* s2);
int      strmatch(const char *wildcard, const char *string);
void     strnlower(char* pstr, int len);
char*    dirname(const char *path);
char*    basename(const char *path);
void     time2date(struct tm* pTm, time_t seconds);
void     compactPath(char* path);

#ifdef __cplusplus
}
#endif

#endif /* STRING_HH_ */
