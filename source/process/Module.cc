/*
 * Module.cc
 *
 *  Created on: 27.01.2013
 *     Copyright & Author: Daniel
 */

#include "Module.hh"
#include "kernel/Kernel.hh"
#include "assemblerFunctions.hh"

extern Kernel* theOS;
extern Kernel_ThreadCfdCl* pCurrentRunningThread;

Module::Module(unint4 phy_start, unint4 phy_end, unint4 heap_start) {
#ifndef HAS_Board_HatLayerCfd
#warning "User Space Device Driver/Modules can only be supported with HatLayer (virtual memory) enabled!!"
    return;
#endif

    register Kernel_MemoryManagerCfdCl* OSMemManager =
            theOS->getMemoryManager();
    void* memaddr =
            OSMemManager->alloc(sizeof(Kernel_MemoryManagerCfdCl), true);

    unint4 size = phy_end - phy_start;
    // create the vm map for the task! protection = 7 = RWX, ZoneSelect = 3
    theOS->getHatLayer()->map(reinterpret_cast<void*>(LOG_MODULE_SPACE_START),
                              reinterpret_cast<void*>(phy_start),
                              size,
                              7,
                              3,
                              this->getId(),
                              !ICACHE_ENABLE);

    // now since the task is mapped activate its virtual memory map by setting the pid
    // change to the module pid so the initialization of the memory manager
    // works on the new virtual address space
    SETPID(this->getId());

    // change back to the kernel address space
    SETPID(0);

    // create initial thread for this task
   // new Kernel_ThreadCfdCl( 0,  0, this, module_memManager, 256, 0, false);
}

Module::~Module() {
    // TODO Auto-generated destructor stub
}

/*****************************************************************************
 * Method: Module::executeModuleFunction(void* addr, void* exit, void* arguments)
 *
 * @description
 *
 *******************************************************************************/
ErrorT Module::executeModuleFunction(void* addr, void* exit, void* arguments) {
#ifndef HAS_Board_HatLayerCfd
    return cError;
#endif

    Kernel_ThreadCfdCl *pThread = static_cast<Kernel_ThreadCfdCl*>(this->getThreadDB()->getHead()->getData());
    if (pThread->isNew()) {
        // we can use this thread .. set its entry method we are calling
        pThread->startRoutinePointer    =  addr;
        pThread->exitRoutinePointer     =  exit;
        pThread->arguments              = arguments;

#ifdef REALTIME
        pThread->absoluteDeadline = pCurrentRunningThread->absoluteDeadline;
        pThread->effectivePriority = pCurrentRunningThread->effectivePriority;
        // set arrival time
#endif
        pThread->run();

        waitingThread = pCurrentRunningThread;

        // block this thread as it is waiting for the pThread to finish
        pCurrentRunningThread->block();

        // returns here upon finished module thread
        return (cOk);
    } else {
        // already used by someone else??
        return (cError);
    }
}

/*****************************************************************************
 * Method: Module::moduleReturn()
 *
 * @description
 *
 *******************************************************************************/
void Module::moduleReturn() {
    // set the thread back to new status so it can be used for the next call
    pCurrentRunningThread->status.setBits(cNewFlag);

    // unblock the waiting thread
    this->waitingThread->unblock();

    // dispatch now
    theOS->getDispatcher()->dispatch();

    __builtin_unreachable();
}
