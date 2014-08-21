/*
 * Module.hh
 *
 *  Created on: 27.01.2013
 *      Author: Daniel
 */

#ifndef MODULE_HH_
#define MODULE_HH_

#include "Task.hh"
#include "SCLConfig.hh"

#ifndef LOG_MODULE_SPACE_START
//#warning "LOG_MODULE_SPACE_START not defined in SCLConfig.hh. Defaulting to address 0x50000!"
#define LOG_MODULE_SPACE_START 0x50000
#endif

/*!
 * \brief The module class implements the runtime instance of a module, it thus inherits the
 * 		  task class. It contains its own memory manager and has access to all and additional syscall
 * 		  functionalities.
 *
 */
class Module: public Task {

protected:

    Kernel_ThreadCfdCl *waitingThread;
    /*!
     * Starts a new thread to execute the module function at "addr". Blocks the current thread
     * and resume it with the return value.
     */
    ErrorT executeModuleFunction(void* addr, void* exit, void* arguments);

public:
    Module(unint4 phy_address, unint4 phy_end, unint4 heap_start);
    virtual ~Module();

    /*!
     * Called by the module upon finished execution.
     * Called from syscall handler.
     *
     * Unblocks the waiting thread.
     */
    void moduleReturn() __attribute__((noreturn));

};

#endif /* MODULE_HH_ */
