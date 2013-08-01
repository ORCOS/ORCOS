/*
 * Module.cc
 *
 *  Created on: 27.01.2013
 *      Author: Daniel
 */

#include "Module.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;
extern ThreadCfdCl* pCurrentRunningThread;

Module::Module(unint4 phy_start, unint4 phy_end, unint4 heap_start) {

	#ifndef HAS_MemoryManager_HatLayerCfd
	#warning "User Space Device Driver/Modules can only be supported with HatLayer (virtual memory) enabled!!"
	return;
	#endif

	register MemoryManagerCfdCl* OSMemManager = theOS->getMemManager();
    void* memaddr = OSMemManager->alloc(sizeof(MemoryManagerCfdCl),true);

    unint4 size = phy_end - phy_start;
    // create the vm map for the task! protection = 7 = RWX, ZoneSelect = 3
    theOS->getHatLayer()->map((void*) LOG_MODULE_SPACE_START, (void*) phy_start, size ,7,3,this->getId(), !ICACHE_ENABLE);

    // now since the task is mapped activate its virtual memory map by setting the pid
    // change to the module pid so the initialization of the memory manager
    // works on the new virtual address space
    SETPID(this->getId());

    MemoryManagerCfdCl* module_memManager =
             new(memaddr)  MemoryManagerCfdCl((void*) (LOG_MODULE_SPACE_START + ( heap_start -  (unint4) phy_start)),
                     (void*) (LOG_MODULE_SPACE_START + size ) );

    this->memManager = module_memManager;

    // change back to the kernel address space
    SETPID(0);

    // create initial thread for this task
    new ThreadCfdCl( (void*) 0, (void*) 0, this, module_memManager, 256, (void*) 0 , false );

}

Module::~Module() {
	// TODO Auto-generated destructor stub
}

ErrorT Module::executeModuleFunction(void* addr, void* exit, void* arguments) {

	#ifndef HAS_MemoryManager_HatLayerCfd
	return cError;
	#endif

	ThreadCfdCl *pThread = (ThreadCfdCl*) this->getThreadDB()->getHead()->getData();
	if (pThread->isNew()) {
		// we can use this thread .. set its entry method we are calling
		pThread->startRoutinePointer = (void*) addr;
		pThread->exitRoutinePointer = (void*) exit;
		pThread->arguments = arguments;

		#ifdef REALTIME
			pThread->absoluteDeadline  = pCurrentRunningThread->absoluteDeadline;
			pThread->effectivePriority = pCurrentRunningThread->effectivePriority;
			// set arrival time
		#endif

		pThread->run();

		waitingThread = pCurrentRunningThread;

		// block this thread as it is waiting for the pThread to finish
		pCurrentRunningThread->block();

		// returns here upon finished module thread

		return cOk;

	} else {
		// already used by someone else??
		return  cError;
	}
}

void Module::moduleReturn() {
	// set the thread back to new status so it can be used for the next call
	pCurrentRunningThread->status.setBits( cNewFlag );

	// unblock the waiting thread
	this->waitingThread->unblock();

	// dispatch now
	theOS->getCPUDispatcher()->dispatch();

	// we will not return here
	while(1);
}
