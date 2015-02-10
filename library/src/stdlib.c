/*
 * stdlib.c
 *
 *  Created on: 08.02.2015
 *      Author: Daniel
 */

static char *initial_env[] = { 0 };

/* Posix says `environ' is a pointer to a null terminated list of pointers.
   Hence `environ' itself is never NULL.  */
char **environ = &initial_env[0];


int atoi(const char *p) {
 int k = 0;
 while (*p) {
 k = (k<<3)+(k<<1)+(*p)-'0';
 p++;
 }
 return k;
}

int abs(int i) {
  return (i < 0) ? -i : i;
}

