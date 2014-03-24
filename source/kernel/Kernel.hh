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

#ifndef KERNEL_H_
#define KERNEL_H_

/* Main Configuration Include */
#include "SCLConfig.hh"

/***************************************
 *             Includes
 ***************************************/
#include "inc/types.hh"
#include Kernel_MemoryManager_hh
#include BoardCfd_hh
#include Board_HatLayer_hh
#include "db/LinkedListDatabase.hh"
#include "process/Task.hh"
#include "comm/ProtocolPool.hh"
#if USE_WORKERTASK
#include "process/WorkerTask.hh"
#endif
#include Kernel_Scheduler_hh
#include "scheduler/SingleCPUDispatcher.hh"
#include "filesystem/SimpleFileManager.hh"
#include "filesystem/PartitionManager.hh"
#include "hal/InterruptManager.hh"
#include Kernel_RamManager_hh
#include "robust/TaskErrorHandler.hh"
#include Kernel_TaskManager_hh
#include Kernel_Logger_hh
#include Kernel_PowerManager_hh
#include Kernel_MigrationManager_hh
#include Kernel_ServiceDiscovery_hh
#include Kernel_Ramdisk_hh

#include "debug/Trace.hh"


/*------------------------------------------------------
 *                  USB Imports
 * ------------------------------------------------------ */
#if HAS_Board_USB_HCCfd
#include "arch/shared/usb/USBDriverLibrary.hh"

#if HAS_USBDriver_SMSC95xxCfd
#include "arch/shared/usb/SMSC95xxUSBDeviceDriver.hh"
#endif

#if HAS_USBDriver_MassStorageCfd
#include "arch/shared/usb/MassStorageSCSIUSBDeviceDriver.hh"
#endif
#endif

#include "comm/lwipTMR.hh"

/*------------------------------------------------------
 *               Main Kernel Methods
 * ------------------------------------------------------ */

void kwait(int milliseconds);

void kwait_us(int microseconds);

/*------------------------------------------------------
 *               The KERNEL class
 * ------------------------------------------------------ */



/*!
 * \brief The Kernel class, as a main organization unit of the ORCOS system.
 *
 *
 * This class is the kernel of ORCOS and keeps references to all
 * configured components as there are the board, the scheduler a.s.o.
 */
class Kernel {

private:
   //! The Memory Manager
    DEF_Kernel_MemoryManagerCfd;

private:
    //! The CPU Manager
    SingleCPUDispatcher*	cpuManager;

    //! The Filesystem Manager
    SimpleFileManager* 		fileManager;

#if HAS_Board_USB_HCCfd
    // Library containing usb deviice drivers
    USBDriverLibrary* 		usbDriverLib;
#endif

    // Partition Manager which intializes attached file systems
    PartitionManager* 		partitionManager;

    // The task manager which initializes tasks
    TaskManager* 			taskManager;

    /* The global interrupt manager handling /scheduling irqs*/
    InterruptManager*		irqManager;

    DEF_Board_HatLayerCfd;

    DEF_Kernel_RamManagerCfd;

    DEF_FileSystems_PartitionManagerCfd;

    // The error Handler for fata task and system errors
    TaskErrorHandler*		errorHandler;

    //! The configured board class
    BoardCfdCl* board;

#if USE_WORKERTASK
    //! The worker thread of our kernel responsible for handling special interrupts (realtime support)
    WorkerTask* theWorkerTask;
#endif

#if ENABLE_NETWORKING
    /*!
     * \brief the Protocol Pool of the kernel which holds the references to all protocols available in the os.
     */
    ProtocolPool* protopool;
#endif

#if USE_TRACE
    // The execution tracer
    Trace* trace;
#endif

    /*!
     *  \brief Initialize the device drivers and the associated hardware components
     */
    void initDeviceDrivers();

    /*!
     * \brief Initialize all initial tasks
     */
    void initInitialTaskSet();

    /*!
     * CharacterDevices for standart out/input of text.
     * This can be some kind of UART oder keyboard/display devices...
     */
    CharacterDeviceDriver* stdInputDevice;
    CharacterDeviceDriver* stdOutputDevice;

public:

    /*!
     *  \brief Constructor of the Kernel
     */
    Kernel();

    /*!
     *  \brief Destructor
     */
    ~Kernel();

    /*!
     * \brief Initialize the Kernel.
     *
     * This involves creating all initial Task CBs, creating the scheduler, cpumanager and initializing
     * the hardware device drivers.
     */
    void initialize()  __attribute__((noreturn));;


    //! The timer device of the system
    Board_TimerCfdCl* getTimerDevice() {
        return (board->getTimer());
    }
    ;

    //! The clock of the system
    Board_ClockCfdCl* getClock() {
    	if (board != 0)
    		return (board->getClock());
    	return 0;
    }
    ;

#if USE_TRACE
private:
    Trace* tracer;

public:

	Trace* getTrace() {
    	return (tracer);
    }
#endif

    /*!
     * \brief Returns the default scheduler for the cpu
     *
     * If multiple cpus are availabe the scheduler for the first cpu will be returned.
     */
    Kernel_SchedulerCfdCl* getCPUScheduler();

    SingleCPUDispatcher* getCPUDispatcher() {
        return (this->cpuManager);
    }
    ;

#if ENABLE_NETWORKING
    ProtocolPool* getProtocolPool() {
        return (this->protopool);
    }
    ;
#endif


    SimpleFileManager* getFileManager() {
        return (this->fileManager);
    }
    ;

#if USE_WORKERTASK
    /* Returns the worker thread of the kernel */
    WorkerTask* getWorkerTask() {
    	return (this->theWorkerTask);
    };
#endif

    //! Returns the database of all registered tasks
    LinkedListDatabase* getTaskDatabase() {
        return (this->taskManager->getTaskDatabase());
    }
    ;

    TaskManager* getTaskManager() {
    	return (this->taskManager);
    }

    InterruptManager* getInterruptManager() {
       	return (this->irqManager);
    }

    TaskErrorHandler* getErrorHandler() {
    	return (this->errorHandler);
    }

    void setStdOutputDevice( CharacterDeviceDriver* outputDevice );

    CharacterDeviceDriver* getStdOutputDevice();

#if HAS_Board_USB_HCCfd
    //! Returns the usb driver library of the kernel
    USBDriverLibrary* getUSBDriverLibrary() {
    	return (usbDriverLib);
    }
#endif

    BoardCfdCl* getBoard() {
        return (board);
    }


    DEF_Kernel_ServiceDiscoveryCfd;

    DEF_Kernel_LoggerCfd;

    DEF_Kernel_PowerManagerCfd;

    DEF_Kernel_MigrationManagerCfd;

    DEF_Kernel_RamdiskCfd;

};


#endif /*KERNEL_H_*/

