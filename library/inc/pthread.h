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

#ifndef PTHREAD_H_
#define PTHREAD_H_

#include "orcos_types.h"
#include "sys/types.h"

typedef int pthread_t;
typedef thread_attr_t pthread_attr_t;

typedef void* pthread_mutex_t;
typedef void* pthread_mutexattr_t;

typedef void* pthread_cond_t;
typedef void* pthread_condattr_t;


#ifdef __cplusplus
extern "C" {
#endif

int pthread_create( pthread_t*, const pthread_attr_t*, void *(*start_routine)( void* ), void* );
void pthread_exit( void *value_ptr );
int pthread_cancel( pthread_t thread );
pthread_t pthread_self( void );
int sched_yield( void );

int pthread_attr_destroy( pthread_attr_t* attr );
int pthread_attr_init( pthread_attr_t* attr );
int pthread_attr_getstacksize( const pthread_attr_t* attr, size_t* stacksize );
int pthread_attr_setstacksize( pthread_attr_t *attr, size_t stacksize );

int pthread_mutex_init( pthread_mutex_t *__mutex, const pthread_mutexattr_t *__mutexattr );
int pthread_mutex_destroy( pthread_mutex_t *__mutex );
int pthread_mutex_lock( pthread_mutex_t *__mutex );
int pthread_mutex_unlock( pthread_mutex_t *__mutex );
int pthread_mutexattr_init( pthread_mutexattr_t *__attr );
int pthread_mutexattr_destroy( pthread_mutexattr_t *__attr );

int pthread_cond_init( pthread_cond_t *, const pthread_condattr_t * );
int pthread_cond_destroy( pthread_cond_t * );
int pthread_cond_signal( pthread_cond_t * );
int pthread_cond_broadcast( pthread_cond_t * );
//int pthread_cond_timedwait( pthread_cond_t *restrict, pthread_mutex_t *restrict, const struct timespec *restrict );
int pthread_cond_wait( pthread_cond_t *, pthread_mutex_t * );
int pthread_condattr_init( pthread_condattr_t * );
int pthread_condattr_destroy( pthread_condattr_t * );

#ifdef __cplusplus
}
#endif

#endif /* PTHREAD_H_ */
