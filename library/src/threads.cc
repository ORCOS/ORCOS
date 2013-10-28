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



#include "./types.h"
#include "./defines.h"
#include "./orcos.hh"

/*!
 * The sleep system call.
 */
extern "C"void sleep(int ms)
{
    // rise the sc software interrupt
    // arguments will be passed onto the stack by the gcc compiler
    syscall(cSleepSysCallId, ms * 1000);
}

/*!
 * The sleep system call.
 */
extern "C"void usleep(int us)
{
    syscall(cSleepSysCallId, us);
}


extern "C"int task_stop(int taskid)
{
    return syscall(cTask_StopSysCallId,taskid);
}

extern "C" int task_run(char* path, char* arguments) {
    return syscall(cRunTaskId,path,arguments);
}

extern "C" int 	task_kill(int taskid) {
	return syscall(cTask_KillSysCallId,taskid);
}

extern "C"int task_resume(int taskid)
{
    return syscall(cTask_ResumeSysCallId,taskid);
}

extern "C"int thread_create(int* threadid,thread_attr_t* attr, void *(*start_routine)(void*), void* arg)
{
    return syscall(cThread_CreateSysCallId,threadid,attr,start_routine,arg);
}

extern "C"int thread_run(int threadid)
{
    return syscall(cThread_RunSysCallId,threadid);
}

extern "C"int thread_self()
{
    return syscall(cThread_SelfSysCallId);
}

extern "C"void thread_yield()
{
    syscall(cThread_YieldSysCallId);
}


extern "C" int	waitpid(unint1 pid)
{
    return (syscall(cThread_WaitPID,pid));
}

extern "C" int 	wait()
{
   return (waitpid(0));
}


