/*
 ORCOS - an Organic Reconfigurable Operating Syste
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

#include "handle_syscalls.hh"
#include Kernel_Thread_hh
#include "assemblerFunctions.hh"


int getCycles(int4 sp_int)
{
	TimeT* time;
	SYSCALLGETPARAMS1(sp_int,time);
	VALIDATE_IN_PROCESS(time);
	*time = theOS->getBoard()->getClock()->getClockCycles();
	return (cOk);
}

int getDateTime(int4 sp_int) {
    return (theOS->getBoard()->getClock()->getDateTime());
}

int printToStdOut(int4 int_sp )
{
   const char *write_ptr;
   unint4 write_size;
   SYSCALLGETPARAMS2(int_sp,write_ptr,write_size);
   VALIDATE_IN_PROCESS(write_ptr);

   LOG(SYSCALLS,TRACE,"Syscall: printToStdOut(%s)",write_ptr);

    if (pCurrentRunningTask->getStdOutputDevice() != 0)
        return (pCurrentRunningTask->getStdOutputDevice()->writeBytes(write_ptr,write_size));
    else return (cError);
}

void thread_exitSyscall(int4 sp_int)
{
	int exitCode;
	SYSCALLGETPARAMS1(sp_int,exitCode);

	pCurrentRunningTask->exitValue = exitCode;
	theOS->getTaskManager()->terminateThread(pCurrentRunningThread);
}

int thread_terminateSyscall(int4 sp_int)
{
    ThreadIdT threadId;
    int params;
    SYSCALLGETPARAMS2(sp_int,threadId,params);

    /* get the thread */
    register Kernel_ThreadCfdCl* t = pCurrentRunningTask->getThreadbyId( threadId );

    if (t == 0) {
        return (cError);
    }

    if (params == TERM_SOFT) {
        LOG(SYSCALLS,DEBUG,"Syscall: Thread %d TERM_SOFT ",threadId);
        /* set the soft termination flag */
        t->setStatusFlag(cDoTermFlag);
        return (cOk);
    } else if( params == TERM_HARD) {
        LOG(SYSCALLS,DEBUG,"Syscall: Thread %d TERM_HARD ",threadId);
        /* call Thread::terminate to be sure no
         * further instances are created as it would be
         * done inside RealtimeThread::terminate() */
       theOS->getTaskManager()->terminateThread(t);
        return (cOk);
    }

    return (cInvalidArgument);

}
