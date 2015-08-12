/*
 * stdlib.c
 *
 *  Created on: 08.02.2015
 *      Author: Daniel
 */
#include "orcos_types.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <defines.h>
#include <error.h>

#if 0
char* __findenv(const char *name, int *offset);

static char *initial_env[] = { 0 };

/* Posix says `environ' is a pointer to a null terminated list of pointers.
   Hence `environ' itself is never NULL.  */
char **environ = &initial_env[0];

int errno;

#define  PSIZE  sizeof(char *)

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

char* getenv(const char *name) {
   int offset;
   return __findenv(name, &offset);
}

/*
 * __findenv --
 *  Returns pointer to value associated with name, if any, else NULL.
 *  Sets offset to be the offset of the name/value combination in the
 *  environmental array, for use by setenv(3) and unsetenv(3).
 *  Explicitly removes '=' in argument name.
 *
 *  This routine *should* be a static; don't use it.
 */
char* __findenv(const char *name, int *offset)
{
    size_t len;
    const char *np;
    char **p, *c;

    if (name == NULL || environ == NULL)
        return NULL;
    for (np = name; *np && *np != '='; ++np)
        continue;
    len = np - name;
    for (p = environ; (c = *p) != NULL; ++p)
        if (strncmp(c, name, len) == 0 && c[len] == '=') {
            *offset = p - environ;
            return c + len + 1;
        }
    *offset = p - environ;
    return NULL;
}


int
putenv( entry )
  char *entry;
{
  unsigned length;
  unsigned size;
  char     *temp;
  char     **p;
  char     **new_environ;

  /*  Find the length of the "NAME="  */

  temp = strchr(entry,'=');
  if ( temp == 0 )
    return( -1 );

  length = (unsigned) (temp - entry + 1);


  /*  Scan through the environment looking for "NAME="  */

  for ( p=environ; *p != 0 ; p++ )
    if ( strncmp( entry, *p, length ) == 0 )
      {
      *p = entry;
      return( 0 );
      }


  /*  The name was not found, build a bigger environment  */

  size = p - environ;

  new_environ = (char **) malloc( (size+2)*PSIZE );

  if ( new_environ == (char **) NULL )
    return( -1 );

  memcpy ((char *) new_environ, (const char *) environ, size*PSIZE );

  new_environ[size]   = entry;
  new_environ[size+1] = NULL;

  environ = new_environ;

  return(0);
}

int unsetenv(const char *name)
{
    extern char **environ;
    char **ep, **sp;
    size_t len;

    if (name == NULL || name[0] == '\0' || strchr(name, '=') != NULL) {
        errno = 1;
        return -1;
    }

    len = strlen(name);

    for (ep = environ; *ep != NULL; )
        if (strncmp(*ep, name, len) == 0 && (*ep)[len] == '=') {

            /* Remove found entry by shifting all successive entries
               back one element */

            for (sp = ep; *sp != NULL; sp++)
                *sp = *(sp + 1);

            /* Continue around the loop to further instances of 'name' */

        } else {
            ep++;
        }

    return 0;
}


int setenv(const char *name, const char *value, int overwrite)
{
    char *es;

    if (name == NULL || name[0] == '\0' || strchr(name, '=') != NULL ||
            value == NULL) {
        errno = 1;
        return -1;
    }

    if (getenv(name) != NULL && overwrite == 0)
        return 0;

    unsetenv(name);             /* Remove all occurrences */

    es = malloc(strlen(name) + strlen(value) + 2);
                                /* +2 for '=' and null terminator */
    if (es == NULL)
        return -1;

    strcpy(es, name);
    strcat(es, "=");
    strcat(es, value);

    return (putenv(es) != 0) ? -1 : 0;
}
#endif

time_t time(time_t* t) {
    time_t value = getDateTime();
    if (t != 0) {
       *t = value;
    }
    return value;
}

int system(const char *cmd) {
    /* try to execute the command..
     * we must interpret the command first..
     * the first argument must be the program to run ..
     * the rest is the program argument vector */

    return -1;
}

void  abort(void) {
    thread_exit(-1);
}


void*  mutex_create() {
  return (malloc(sizeof(int)));
}

ErrorT  mutex_destroy(void* mutex) {
    free(mutex);
    return (cOk);
}

ErrorT mutex_acquire(void* mutex, int blocking) {
    // this actually implements a simple Mutex operation
    // blocked threads will be activated on Mutex::release according to
    // their priorities
    int* counter = (int*) mutex;
    while ( testandset(counter, 1, 0) == 0 ) {
         if (!blocking)
             return (cError);

         signal_wait( (void*) counter, 0, 1 );
    }

    counter = 0;
    return (cOk);
}

ErrorT mutex_release(void* mutex) {
    int* counter = (int*) mutex;
    counter = 1;
    signal_signal( (void*) counter, 0, 1 );
    return (cOk);
}
