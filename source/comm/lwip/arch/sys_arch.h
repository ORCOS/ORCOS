/*
 * sys_arch.h
 *
 *  Created on: 27.01.2010
 *      Author: dbaldin
 */

#ifndef SYS_ARCH_H_
#define SYS_ARCH_H_

#define SYS_MBOX_NULL NULL
#define SYS_SEM_NULL  NULL

typedef u32_t sys_prot_t;

struct sys_sem;
typedef struct sys_sem * sys_sem_t;

struct sys_mbox;
typedef struct sys_mbox *sys_mbox_t;

struct sys_thread;
typedef struct sys_thread * sys_thread_t;


#endif /* SYS_ARCH_H_ */
