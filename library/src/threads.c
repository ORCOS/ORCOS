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

#include "orcos_types.h"
#include "defines.h"
#include "orcos.h"


/*!
 * The sleep system call.
 */
 unsigned int  sleep(unsigned int sectime) {
    return (syscall(cSleepSysCallId, sectime * 1000000));
}

/*!
 * The usleep system call.
 */
 int   usleep(unsigned long ustime) {
    return (syscall(cSleepSysCallId, ustime));
}


int task_stop(int taskid) {
    return (syscall(cTask_StopSysCallId, taskid));
}

int task_run(char* path, char* arguments, char* stdout) {
    return (syscall(cRunTaskId, path, arguments, stdout));
}

int thread_name(int threadid,char* name) {
    return (syscall(cThreadNameSyscallId, threadid, name));
}

int task_kill(int taskid) {
    return (syscall(cTask_KillSysCallId, taskid));
}

int task_resume(int taskid) {
    return (syscall(cTask_ResumeSysCallId, taskid));
}

int thread_create(ThreadIdT* threadid,thread_attr_t* attr, void *(*start_routine)(void*), void* arg) {
    return (syscall(cThread_CreateSysCallId, threadid, attr, start_routine, arg));
}

int thread_run(int threadid)
{
    return (syscall(cThread_RunSysCallId, threadid));
}

int thread_self()
{
    return (syscall(cThread_SelfSysCallId));
}

void thread_yield()
{
    syscall(cThread_YieldSysCallId);
}

int thread_terminate(int threadId, int flag) {
    return (syscall(cThreadTerminateSyscallId, threadId, flag));
}

int waitpid(int pid)
{
    return (syscall(cThread_WaitPID, pid, 0));
}

int wait()
{
   /* wait for any thread child to finish*/
   return (waitpid(0));
}

int waittid(int tid)
{
    /* wait for a thread inside our task to finish */
    return (syscall(cThread_WaitPID, 0, tid));
}


int waitirq(unint4 irq) {
    return (syscall(cThreadWaitIRQSyscallId, irq));
}

int   getpid() {
    return (syscall(cGetPID));
}

int   taskioctl(int cmd, int taskid, char* dev) {
    return (syscall(cTaskioctlscallId, cmd, taskid, dev));
}
