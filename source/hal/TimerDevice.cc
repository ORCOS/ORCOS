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

#include "TimerDevice.hh"
#include "kernel/Kernel.hh"
#include <inc/sys/timer.h>
#include <syscalls/syscalls.hh>
#include <SCLConfig.hh>

extern Kernel* theOS;

TimerDevice::TimerDevice(const char* name) : GenericDeviceDriver((ResourceType) (cGenericDevice | cTimerDevice), true, name) {
    elapsedCycles   = 0;
    dispatcher      = theOS->getDispatcher();
    tickRate        = 0;
    llThread        = 0;
    isEnabled       = false;
    time            = 0;
    ASSERT(dispatcher);
    /* register myself at the filesystem manager */
    SimpleFileManager* fm = theOS->getFileManager();
    /* be sure we have a filesystem. if not we can not continue since every driver needs to register!! */
    ASSERT(fm);
    if (fm != 0) {
      fm->registerResource(this);
    }
}

TimerDevice::~TimerDevice() {
}

/*****************************************************************************
 * Method: TimerDevice::handleIRQ()
 *
 * @description
 *  Handles Timer IRQS from this device. Provides the functionality for
 *  ultra low latency thread activations.
 *******************************************************************************/
ErrorT TimerDevice::handleIRQ() {
#if 0
    Board_TimerCfdCl* timer = (Board_TimerCfdCl*) this;
    LOG(ARCH,INFO,"llThread Latency: %u",timer->getTimerValue());
#endif

    if (llThread != 0 && llThread->status.areSet(cSignalFlag)) {
        llThread->status.setBits(cReadyFlag);
        llThread->status.clearBits(cSignalFlag);
        /* TODO set new HW INTC priority threshold. Use a priority stack to restore the priority level
           for this as multiple timer llThreads may be preempting each other. */

        /* directly restore this thread ignoring all scheduling decisions.
         * This is absolutely wanted by the user to provide ultra low latencies
         * to threads. */
        pCurrentRunningThread   = static_cast<Kernel_ThreadCfdCl*>(llThread);
        pCurrentRunningTask     = pCurrentRunningThread->getOwner();
        assembler::restoreContext(llThread);
        __builtin_unreachable();
    }

    return (cOk);
}

/*****************************************************************************
* Method: TimerDevice::ioctl(int request, void* args)
*
* @description
*  Userspace configuration access to this timer devices.
*******************************************************************************/
ErrorT TimerDevice::ioctl(int request, void* args) {
    switch (request) {
        case TIMER_IOCTL_CONFIG: {
            VALIDATE_IN_PROCESS(args);
            timer_t* timer_conf = reinterpret_cast<timer_t*>(args);

            if (llThread != 0) {
                return (cError);
            }

            if (timer_conf->threadId == 0) {
                llThread = pCurrentRunningThread;
            } else {
                llThread = theOS->getTaskManager()->getThread(timer_conf->threadId);
                if (llThread == 0) {
                    return (cInvalidArgument);
                }
            }

            setPeriod(timer_conf->period_us);
            isEnabled = true;
            return (cOk);
        }
        case TIMER_IOCTL_RESET: {
            llThread = 0;
            disable();
            return (cOk);
        }
    }

    return (cError);
}
