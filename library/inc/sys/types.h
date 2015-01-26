/*
 * types.h
 *
 *  Created on: 21.01.2015
 *      Author: Daniel
 */

#ifndef LIBRARY_INC_SYS_TYPES_H_
#define LIBRARY_INC_SYS_TYPES_H_


typedef int pid_t;
typedef int off_t;
typedef int mode_t;
typedef int key_t;
typedef int id_t;
typedef int uid_t;
typedef int gid_t;
typedef int dev_t;
typedef unsigned int clock_t;
typedef unsigned int useconds_t;
typedef int clockid_t;
typedef unsigned int blksize_t;
typedef unsigned int blkcnt_t;

typedef int pthread_t;
typedef thread_attr_t pthread_attr_t;

typedef void* pthread_mutex_t;
typedef void* pthread_mutexattr_t;

typedef void* pthread_cond_t;
typedef void* pthread_condattr_t;



#endif /* LIBRARY_INC_SYS_TYPES_H_ */
