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
#include "db/LinkedList.hh"
#include "process/Task.hh"
#include "comm/ProtocolPool.hh"
#if USE_WORKERTASK
#include "process/WorkerTask.hh"
#endif
#include Kernel_Scheduler_hh
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
#include Kernel_InterruptManager_hh
#include Kernel_Dispatcher_hh

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

    //! The Filesystem Manager
    SimpleFileManager* fileManager;

#if HAS_Board_USB_HCCfd
    // Library containing usb device drivers
    USBDriverLibrary* usbDriverLib;
#endif

    // The task manager which initializes tasks
    TaskManager* taskManager;

    // The error Handler for fatal task and system errors
    TaskErrorHandler* errorHandler;

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
    CharacterDevice* stdInputDevice;
    CharacterDevice* stdOutputDevice;

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
    void initialize() __attribute__((noreturn));

    //! The timer device of the system
    inline Board_TimerCfdCl* getTimerDevice() {
        return (board->getTimer());
    }

    //! The clock of the system
    inline Board_ClockCfdCl* getClock() {
        if (board != 0)
            return (board->getClock());
        return (0);
    }

#if USE_TRACE
private:
    Trace* tracer;

public:

    Trace* getTrace()
    {
        return (tracer);
    }
#endif

    /*!
     * \brief Returns the default scheduler for the cpu
     */
    Kernel_SchedulerCfdCl* getCPUScheduler();

#if ENABLE_NETWORKING
    inline ProtocolPool* getProtocolPool() {
        return (this->protopool);
    }
#endif

    inline SimpleFileManager* getFileManager() {
        return (this->fileManager);
    }

#if USE_WORKERTASK
    /* Returns the worker thread of the kernel */
    WorkerTask* getWorkerTask() {
        return (this->theWorkerTask);
    }
#endif

    //! Returns the database of all registered tasks
    inline LinkedList* getTaskDatabase() {
        return (this->taskManager->getTaskDatabase());
    }

    inline TaskManager* getTaskManager() {
        return (this->taskManager);
    }

    inline TaskErrorHandler* getErrorHandler() {
        return (this->errorHandler);
    }

    void setStdOutputDevice(CharacterDevice* outputDevice);

    CharacterDevice* getStdOutputDevice();

#if HAS_Board_USB_HCCfd
    //! Returns the usb driver library of the kernel
    inline USBDriverLibrary* getUSBDriverLibrary() {
        return (usbDriverLib);
    }
#endif

    inline BoardCfdCl* getBoard() {
        return (board);
    }

    DEF_Board_HatLayerCfd


    DEF_Kernel_RamManagerCfd


    DEF_FileSystems_PartitionManagerCfd


    DEF_Kernel_ServiceDiscoveryCfd


    DEF_Kernel_LoggerCfd


    DEF_Kernel_PowerManagerCfd


    DEF_Kernel_MigrationManagerCfd


    DEF_Kernel_RamdiskCfd


    DEF_Kernel_InterruptManagerCfd


    DEF_Kernel_DispatcherCfd


    DEF_Kernel_MemoryManagerCfd
};

#endif /*KERNEL_H_*/

