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

#include "pthread.hh"

#include "Mutex.hh"

#include "orcos.hh"


int pthread_create( pthread_t* thread, const pthread_attr_t* attr, void* (*start_routine)( void* ), void* arg ) {
    int ret;

    ret = thread_create( thread, (thread_attr_t*) attr, start_routine, arg );

    if ( ret == 0 ) {
        ret = thread_run( *thread );
    }

    return ret;
}

void pthread_exit( void *value_ptr ) {
    thread_exit();
}

int pthread_cancel( pthread_t thread ) {
    if ( pthread_self() == thread ) {
        thread_exit();
    }

    //TODO missing syscall to kill other threads
    return -1;
}

pthread_t pthread_self( void ) {
    return thread_self();
}

int sched_yield( void ) {
    thread_yield();

    return 0;
}


int pthread_attr_destroy( pthread_attr_t* attr ) {
    return 0;
}

int pthread_attr_init( pthread_attr_t* attr ) {
    attr->phase = 0;
    attr->priority = 0;
    attr->period = 0;
    attr->stack_size = 1200;
    attr->deadline = 0;
    attr->executionTime = 0;
    return 0;
}

int pthread_attr_getstacksize( const pthread_attr_t* attr, size_t* stacksize ) {
    *stacksize = (size_t) attr->stack_size;

    return 0;
}

int pthread_attr_setstacksize( pthread_attr_t *attr, size_t stacksize ) {
    attr->stack_size = (int) stacksize;

    return 0;
}



int pthread_mutex_init( pthread_mutex_t *__mutex, const pthread_mutexattr_t *__mutexattr ) {
    Mutex** m = (Mutex**) __mutex;
    *m = new Mutex();

    return 0;
}

int pthread_mutex_destroy( pthread_mutex_t *__mutex ) {
    Mutex** m = (Mutex**) __mutex;
    delete *m;

    return 0;
}

int pthread_mutex_lock( pthread_mutex_t *__mutex ) {
    Mutex** m = (Mutex**) __mutex;

    return ( *m )->acquire();
}

int pthread_mutex_unlock( pthread_mutex_t *__mutex ) {
    Mutex** m = (Mutex**) __mutex;

    return ( *m )->release();
}

int pthread_mutexattr_init( pthread_mutexattr_t *__attr ) {
    return 0;
}

int pthread_mutexattr_destroy( pthread_mutexattr_t *__attr ) {
    return 0;
}

int pthread_cond_init( pthread_cond_t * __cond, const pthread_condattr_t * __attr ) {
    bool** b = (bool**) __cond;
    *b = new bool;

    return 0;
}

int pthread_cond_destroy( pthread_cond_t * __cond ) {
    bool** b = (bool**) __cond;
    delete *b;

    return 0;
}

int pthread_cond_signal( pthread_cond_t * __cond ) {
    signal_signal( (void*) __cond, true );

    return 0;
}

int pthread_cond_broadcast( pthread_cond_t * __cond ) {
    signal_signal( (void*) __cond, true );

    return 0;
}

//int pthread_cond_timedwait( pthread_cond_t *restrict, pthread_mutex_t *restrict, const struct timespec *restrict );

int pthread_cond_wait( pthread_cond_t * __cond, pthread_mutex_t * __mutex ) {

    pthread_mutex_unlock( __mutex );

    signal_wait( (void*) __cond, true );

    pthread_mutex_lock( __mutex );

    return 0;
}

int pthread_condattr_init( pthread_condattr_t * __attr ) {
    return 0;
}

int pthread_condattr_destroy( pthread_condattr_t * __attr ) {
    return 0;
}
