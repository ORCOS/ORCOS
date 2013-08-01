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

#include <Leon3InterruptHandler.hh>
#include <assemblerFunctions.cc>
#include "syscalls/handle_syscalls.hh"
#include "inc/error.hh"

//#define STACK_CONTENT(sp_int,offset) *( (long*) ( ( (long) sp_int) + offset)  )

Leon3InterruptHandler* theLeon3InterruptHandler;

extern Board_TimerCfdCl* theTimer;
extern Kernel* theOS;

extern "C" void handleTimerTrap(void* sp_int, void* myKernelStackBucketIndex) {

	//LOG(KERNEL,INFO,(KERNEL,INFO,"Timer Interrupt!"));

	// print the content of the registers
	LOG(PROCESS,TRACE,(PROCESS,TRACE,"l0 : 0x%x, l1: 0x%x, l2: 0x%x, i3:0x%x" , STACK_CONTENT(sp_int,  0),STACK_CONTENT(sp_int,4),STACK_CONTENT(sp_int,8),STACK_CONTENT(sp_int,12)));
	LOG(PROCESS,TRACE,(PROCESS,TRACE,"l4 : 0x%x, l5: 0x%x, l6: 0x%x, i7:0x%x" , STACK_CONTENT(sp_int, 16),STACK_CONTENT(sp_int,20),STACK_CONTENT(sp_int,24),STACK_CONTENT(sp_int,28)));
	LOG(PROCESS,TRACE,(PROCESS,TRACE,"i4 : 0x%x, i5: 0x%x, i6: 0x%x, l7:0x%x" , STACK_CONTENT(sp_int, 48),STACK_CONTENT(sp_int,52),STACK_CONTENT(sp_int,56),STACK_CONTENT(sp_int,60)));
	LOG(PROCESS,TRACE,(PROCESS,TRACE,"g0 : 0x%x, g1: 0x%x, g2: 0x%x, g3:0x%x" , STACK_CONTENT(sp_int, 64),STACK_CONTENT(sp_int,68),STACK_CONTENT(sp_int,72),STACK_CONTENT(sp_int,76)));

	// if we come from idle thread, pCurrentRunningThread = 0x0
	if (pCurrentRunningThread != 0) {
        ASSERT(isOk(pCurrentRunningThread->pushStackPointer(sp_int)));

        #if USE_SAFE_KERNEL_STACKS
            // store the used stack bucket index
            pCurrentRunningThread->setKernelStackBucketIndex((int)myKernelStackBucketIndex);
        #endif

    } else {
       #if USE_SAFE_KERNEL_STACKS
           FREE_KERNEL_STACK_SLOT(myKernelStackBucketIndex);
       #endif
	}



#ifdef HAS_Board_WatchdogCfd
        theOS->getBoard()->getWatchdog()->kick();
#endif


    // call Timer
    theTimer->tick();
}

extern "C" void handleIRQTrap(void* sp_int,int irqSrc, void* myKernelStackBucketIndex) {

	//LOG(KERNEL,INFO,(KERNEL,INFO,"--- IRQ --- sp @ 0x%x",sp_int));

	LOG(PROCESS,TRACE,(PROCESS,TRACE,"l0 : 0x%x, l1: 0x%x, l2: 0x%x, i3:0x%x" , STACK_CONTENT(sp_int,  0),STACK_CONTENT(sp_int,4),STACK_CONTENT(sp_int,8),STACK_CONTENT(sp_int,12)));
	LOG(PROCESS,TRACE,(PROCESS,TRACE,"l4 : 0x%x, l5: 0x%x, l6: 0x%x, i7:0x%x" , STACK_CONTENT(sp_int, 16),STACK_CONTENT(sp_int,20),STACK_CONTENT(sp_int,24),STACK_CONTENT(sp_int,28)));
	LOG(PROCESS,TRACE,(PROCESS,TRACE,"i4 : 0x%x, i5: 0x%x, i6: 0x%x, l7:0x%x" , STACK_CONTENT(sp_int, 48),STACK_CONTENT(sp_int,52),STACK_CONTENT(sp_int,56),STACK_CONTENT(sp_int,60)));
	LOG(PROCESS,TRACE,(PROCESS,TRACE,"g0 : 0x%x, g1: 0x%x, g2: 0x%x, g3:0x%x" , STACK_CONTENT(sp_int, 64),STACK_CONTENT(sp_int,68),STACK_CONTENT(sp_int,72),STACK_CONTENT(sp_int,76)));


	// if we come from idle thread, pCurrentRunningThread = 0x0
	if (pCurrentRunningThread != 0){
	    ASSERT(isOk(pCurrentRunningThread->pushStackPointer(sp_int)));

    #if USE_SAFE_KERNEL_STACKS
        // store the used stack bucket index
        pCurrentRunningThread->setKernelStackBucketIndex((int)myKernelStackBucketIndex);
    #endif

    } else {
       #if USE_SAFE_KERNEL_STACKS
           FREE_KERNEL_STACK_SLOT(myKernelStackBucketIndex);
       #endif
    }

#if ENABLE_NESTED_INTERRUPTS
    // we want nested interrupts so enable interrupts again
    _enableInterrupts();
#endif

    switch (irqSrc)
    {
		// UART
		case 0x12:
				{

				#if HAS_Board_UARTCfd
					LOG(PROCESS,INFO,(PROCESS,INFO,"UART IRQ"));
					register CommDeviceDriver* uart = theOS->getBoard()->getUART();
					uart->clearIRQ();
					uart->disableIRQ();
					uart->interruptPending = true;
					uart->recv();
					break;
				#endif
				}
		// Shared Memory communication
		case 0x14: {
			#if HAS_Board_SHMCfd
				LOG(PROCESS,DEBUG,(PROCESS,DEBUG,"SHM IRQ"));
				register CommDeviceDriver* shm = theOS->getBoard()->getSHM();
				shm->interruptPending = true;
				/*#if USE_WORKERTASK
					if (!shm->hasAssignedWokerThread){
						if (theOS->getWorkerTask()->addJob(ExternalDeviceJob,0,(void*) shm, WORKERTHREAD_UART_PRIORITY_PARAM)) {
							shm->hasAssignedWokerThread = true;
						}
					}
				#else*/
				shm->recv();
				//#endif

				//theOS->getCPUDispatcher()->dispatch( theOS->getClock()->getTimeSinceStartup() - lastCycleStamp );

			#endif

			break;
		}
		// GRETH
		case 0x1e:
				{
					int LocalNodeNr = 0;
					GET_CPU_INDEX(LocalNodeNr);
				#if HAS_Board_ETHCfd
					LOG(PROCESS,INFO,(PROCESS,INFO,"ETH IRQ"));
					if (LocalNodeNr == 1) {
	                register CommDeviceDriver* eth = theOS->getBoard()->getETH();
	                eth->clearIRQ();
	                eth->disableIRQ();
	                eth->interruptPending = true;
	                eth->recv();
					}
	                break;
				#endif
				}
		default:
		{
			#ifdef HAS_Kernel_LoggerCfd
				theOS->getLogger()->log(ARCH,FATAL,"Unhandled Interrupt Nr: %i \n\r", irqSrc);
			#endif

			break;
		}

	}

    theOS->getCPUDispatcher()->dispatch( theOS->getClock()->getTimeSinceStartup() - lastCycleStamp );
	    // if we get here we need to restore the context
    /*if (pCurrentRunningThread)
    assembler::restoreContext( pCurrentRunningThread );
    else {

    	theOS->getCPUDispatcher()->dispatch( theOS->getClock()->getTimeSinceStartup() - lastCycleStamp );
    	//interrupt during idle thread
    	asm volatile (
    			//set stack pointer and branch to leaveHandler
    			"mov %0, %%g5;"
    			"ba leavehandler;"
    			"nop"
    			:
    			:"r" (sp_int)
    			:
    		);
    }*/


}

extern "C" void handleSyscallTrap(void* sp_int, void* myKernelStackBucketIndex) {

	LOG(PROCESS,TRACE,(PROCESS,TRACE,"Syscall Interrupt! sp@%x",sp_int));
	// print the content of the registers
	LOG(PROCESS,TRACE,(PROCESS,TRACE,"l0 : 0x%x, l1: 0x%x, l2: 0x%x, i3:0x%x" , STACK_CONTENT(sp_int,  0),STACK_CONTENT(sp_int,4),STACK_CONTENT(sp_int,8),STACK_CONTENT(sp_int,12)));
	LOG(PROCESS,TRACE,(PROCESS,TRACE,"l4 : 0x%x, l5: 0x%x, l6: 0x%x, i7:0x%x" , STACK_CONTENT(sp_int, 16),STACK_CONTENT(sp_int,20),STACK_CONTENT(sp_int,24),STACK_CONTENT(sp_int,28)));
	LOG(PROCESS,TRACE,(PROCESS,TRACE,"i4 : 0x%x, i5: 0x%x, i6: 0x%x, l7:0x%x" , STACK_CONTENT(sp_int, 48),STACK_CONTENT(sp_int,52),STACK_CONTENT(sp_int,56),STACK_CONTENT(sp_int,60)));
	LOG(PROCESS,TRACE,(PROCESS,TRACE,"g0 : 0x%x, g1: 0x%x, g2: 0x%x, g3:0x%x" , STACK_CONTENT(sp_int, 64),STACK_CONTENT(sp_int,68),STACK_CONTENT(sp_int,72),STACK_CONTENT(sp_int,76)));

    ASSERT(isOk(pCurrentRunningThread->pushStackPointer(sp_int)));

#if USE_SAFE_KERNEL_STACKS
    // store the used stack bucket index
    pCurrentRunningThread->setKernelStackBucketIndex((int)myKernelStackBucketIndex);
#endif

#if ENABLE_NESTED_INTERRUPTS
    // we want nested interrupts so enable interrupts again
    _enableInterrupts();
#endif

    // get the syscallmanager now (since it could have been replaced..) and call syscall handling method
    handleSyscall( (int4) sp_int);
}
