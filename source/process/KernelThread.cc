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

#include "KernelThread.hh"

#include "hal/CommDeviceDriver.hh"
#include "kernel/Kernel.hh"
#include "assemblerFunctions.hh"

extern Kernel* theOS;

#ifndef WORKERTHREAD_STACK_SIZE
#error 'WORKERTHREAD_STACK_SIZE' not defined!
#endif

// the stack address and size may be set explicitly
KernelThread::KernelThread(Task* p_owner) :
        Kernel_ThreadCfdCl(0, 0, p_owner, WORKERTHREAD_STACK_SIZE, 0) {
    jobid = None;
    pid = 0;
    irq = 0;
}

KernelThread::~KernelThread() {
}

/*****************************************************************************
 * Method: WorkerThread::callMain()
 *
 * @description
 *  Entry method for the workerthread
 *******************************************************************************/
void KernelThread::callMain() {
    /* clear the new flag since we are not new anymore */
    this->status.clearBits(cNewFlag);

    /* start new stack from top of stack addr space
     store all values inside register since we are going to switch the pid
     and variables on the stack will not be accessible any more */
    register void* stack_addr   = this->threadStack.endAddr;
    register unint1 u_pid       = this->pid;
    register void* thisptr      = this;
    this->threadStack.top       = 0;

    /* Branch to the work method while setting
     * the object pointer, the pid and the stack address*/
    BRANCHTO_WORKERTHREAD_WORK(thisptr, u_pid, stack_addr);

    /* we should never get here */
    ERROR("Workerthread callMain reached end.");
    __builtin_unreachable();
}

/*****************************************************************************
 * Method: WorkerThread::work()
 *
 * @description
 *  Workerthread body.
 *******************************************************************************/
void KernelThread::work() {
    /* now we are ready to finish the job
     * at this point we are also able to access the
     * jobs parameter since it lies either on the
     * stack of the thread which issued this job
     * or on the heap. Both is accessible since
     * the pid is set correctly. */

    /* enable interrupts here since we are now executing like a user thread except that we are in kernel space */
    _enableInterrupts();

    if (jobid == IRQJob) {
        GenericDeviceDriver* dev = param.driver;

        /* call receive method which is the job here
         * as long as interrupts are pending for it */
        while (dev->interruptPending)
            dev->handleIRQ(irq);

        _disableInterrupts();

        /* indicate that the work has been finished on this comm_dev */
        dev->hasAssignedWorkerThread = false;
        /* communication device may now throw interrupts again */
        dev->enableIRQ(irq);
    } else if ((jobid == TimedFunctionCallJob)) {
        TimedFunctionCall* funcCall = &param.timedCall;
        /* call the callbackFunction of that object */
        (funcCall->objectptr)->callbackFunc(funcCall->parameterptr);
    } else if (jobid == PeriodicFunctionCallJob) {
        PeriodicFunctionCall* pcall = &param.periodicCall;
        (pcall->functioncall.objectptr)->callbackFunc(pcall->functioncall.parameterptr);
    }

    _disableInterrupts();

    /* reset the new flag so next time we will execute callMain again next time
     * and do not try to restore some context that does not exist */

    this->status.setBits(cNewFlag);

    if (jobid == PeriodicFunctionCallJob) {
        /* we are a periodic thread. dont block myself bet get to sleep instead */
        PeriodicFunctionCall* pcall = &param.periodicCall;
        /* set the absolute time of execution */
        pcall->functioncall.time += pcall->period;
        /* increase instance counter */
        this->instance++;
        // calculate the time until execution and get to sleep
        Thread::sleep(pcall->functioncall.time);
    } else if (jobid == IRQJob) {
        /* just go back to blocked mode until next IRQ */
        this->block();
    } else {
        /* stop again so we can be reassigned */
        this->stop();
    }

    __builtin_unreachable();
}

/*****************************************************************************
 * Method: WorkerThread::stop()
 *
 * @description
 *
 *******************************************************************************/
void KernelThread::stop() {
   jobid = None;
   this->setName("");
   KernelTask* pWTask = static_cast<KernelTask*>(this->getOwner());
   pWTask->workFinished(this);
   this->block();
}
