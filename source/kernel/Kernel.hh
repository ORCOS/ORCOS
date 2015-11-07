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

#define LINEFEED "\r\n"

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
#include "../process/KernelTask.hh"
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
#include Kernel_Ramdisk_hh
#include Kernel_Tracer_hh
#include FileSystems_FATFileSystem_hh
#include FileSystems_RamFilesystem_hh
#include Kernel_InterruptManager_hh
#include Kernel_Dispatcher_hh

#include "debug/Trace.hh"

/*------------------------------------------------------
 *                  USB Imports
 * ------------------------------------------------------ */
#if HAS_Board_USB_HCCfd
#include "arch/shared/usb/USBDriverLibrary.hh"
#endif

#include "KernelServiceThread.hh"

/*------------------------------------------------------
 *               Main Kernel Methods
 * ------------------------------------------------------ */

extern Board_ClockCfdCl* theClock;
extern Board_TimerCfdCl* theTimer;


/*****************************************************************************
 * Method: kwait(int milliseconds)
 *
 * @description
 *  Waits for the given number of milliseconds
 *
 * @params
 *  milliseconds:   The number of milliseconds to wait
 *
 * @returns
 *
 *---------------------------------------------------------------------------*/
void kwait(int milliseconds);

/*****************************************************************************
 * Method: kwait_us(int microseconds)
 *
 * @description
 *  Waits for the given number of micro seconds
 *
 * @params
 *  milliseconds:   The number of micro seconds to wait
 *
 * @returns
 *
 *---------------------------------------------------------------------------*/
void kwait_us(int microseconds);

void kwait_us_nonmem(int microseconds);

#define TIMEOUT_WAIT(condition, timeout_us) ({  \
    int __timeout = timeout_us; \
    while ((condition) && (__timeout)) \
    { \
        __timeout--; \
        kwait_us_nonmem(1); \
    } \
    (__timeout <= 0); \
})


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
    KernelTask* theKernelTask;
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

    /*****************************************************************************
     * Method: initDeviceDrivers()
     *
     * @description
     *  Initializes the Kernel Device driver infrastructure.
     *  Instantiates the board.
     *---------------------------------------------------------------------------*/
    void initDeviceDrivers();

    /*****************************************************************************
    * Method: initInitialTaskSet()
    *
    * @description
    *  Initializes the initial tasks provided inside the kernel image.
    *---------------------------------------------------------------------------*/
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

    /*****************************************************************************
    * Method: initialize()
    *
    * @description
    *  Initializes the Kernel.
    *---------------------------------------------------------------------------*/
    void initialize() __attribute__((noreturn));

    /*****************************************************************************
    * Method: getTimerDevice()
    *
    * @description
    *  Returns the current cpu timer.
    *
    * @returns
    *  Board_TimerCfdCl* The current Timer device used.
    *---------------------------------------------------------------------------*/
    inline Board_TimerCfdCl* getTimerDevice() {
        return (theTimer);
    }

    /*****************************************************************************
    * Method: getClock()
    *
    * @description
    *  Returns the current cpu clock.
    *
    * @returns
    *  Board_ClockCfdCl* The current clock device used.
    *---------------------------------------------------------------------------*/
    inline Board_ClockCfdCl* getClock() {
        return (theClock);
    }

    /*****************************************************************************
    * Method: getCPUScheduler()
    *
    * @description
    *  Returns the current configured cpu scheduler
    *
    * @returns
    *  Kernel_SchedulerCfdCl* The cpu scheduler
    *---------------------------------------------------------------------------*/
    Kernel_SchedulerCfdCl* getCPUScheduler();

#if ENABLE_NETWORKING
    /*****************************************************************************
    * Method: getProtocolPool()
    *
    * @description
    *  Returns the networking protocol pool containing the supported
    *  protocols. The Socket class uses this pool on creation.
    *
    * @returns
    *  ProtocolPool* The protocol pool
    *---------------------------------------------------------------------------*/
    inline ProtocolPool* getProtocolPool() {
        return (this->protopool);
    }
#endif

    /*****************************************************************************
    * Method: getFileManager()
    *
    * @description
    *  Returns the file manager.
    *
    * @returns
    *  SimpleFileManager* The the file manager used by the kernel.
    *---------------------------------------------------------------------------*/
    inline SimpleFileManager* getFileManager() {
        return (this->fileManager);
    }

#if USE_WORKERTASK
    /*****************************************************************************
    * Method: getWorkerTask()
    *
    * @description
    *  Returns the WorkerTask used for scheduling aperiodic jobs as e.g.
    *  IRQ requests to provide a low latency environment.
    *
    * @returns
    *  WorkerTask* The worker task.
    *---------------------------------------------------------------------------*/
    KernelTask* getKernelTask() {
        return (this->theKernelTask);
    }
#endif

    /*****************************************************************************
    * Method: getTaskDatabase()
    *
    * @description
    *  Returns the list of currently active tasks inside the system
    *
    * @returns
    *  LinkedList* The list of all tasks used inside the system
    *---------------------------------------------------------------------------*/
    inline LinkedList* getTaskDatabase() {
        return (this->taskManager->getTaskDatabase());
    }

    /*****************************************************************************
    * Method: getTaskManager()
    *
    * @description
    *  Returns the task manager responsible for handler task loading/unloading,
    *  task checking etc.
    *
    * @returns
    *  TaskManager* The task manager
    *---------------------------------------------------------------------------*/
    inline TaskManager* getTaskManager() {
        return (this->taskManager);
    }

    /*****************************************************************************
    * Method: getErrorHandler()
    *
    * @description
    *  Returns the error handler
    *
    * @returns
    *  TaskErrorHandler* The task error handler
    *---------------------------------------------------------------------------*/
    inline TaskErrorHandler* getErrorHandler() {
        return (this->errorHandler);
    }

    /*****************************************************************************
    * Method: setStdOutputDevice(CharacterDevice* outputDevice)
    *
    * @description
    *  Sets the standard output device
    *
    *  @params
    *   outputDevice: The new standard output device
    *
    *---------------------------------------------------------------------------*/
    void setStdOutputDevice(CharacterDevice* outputDevice);

    /*****************************************************************************
    * Method: getStdOutputDevice()
    *
    * @description
    *  Returns the current standard output device
    *
    *  @returns
    *   CharacterDevice: The standard output device
    *
    *---------------------------------------------------------------------------*/
    CharacterDevice* getStdOutputDevice();

#if HAS_Board_USB_HCCfd
    /*****************************************************************************
    * Method: getUSBDriverLibrary()
    *
    * @description
    *  Returns the USB Driver Library containing the list of
    *  USB driver factories. This library is queried by the
    *  USB EHCI implementation upon device detection for the corresponding
    *  USB device attached.
    *
    *  @returns
    *   USBDriverLibrary: The USB Driver library
    *
    *---------------------------------------------------------------------------*/
    inline USBDriverLibrary* getUSBDriverLibrary() {
        return (usbDriverLib);
    }
#endif

    /*****************************************************************************
    * Method: getBoard()
    *
    * @description
    *  Returns the board object containing references to the device driver (not
    *  necessarily kernel driver but also user space device driver)
    *
    *  @returns
    *   BoardCfdCl: The Board
    *
    *---------------------------------------------------------------------------*/
    inline BoardCfdCl* getBoard() {
        return (board);
    }

    DEF_Board_HatLayerCfd


    DEF_Kernel_RamManagerCfd


    DEF_FileSystems_PartitionManagerCfd


    DEF_Kernel_LoggerCfd


    DEF_Kernel_PowerManagerCfd


    DEF_Kernel_RamdiskCfd


    DEF_FileSystems_RamFilesystemCfd


    DEF_Kernel_InterruptManagerCfd


    DEF_Kernel_DispatcherCfd


    DEF_Kernel_MemoryManagerCfd

    DEF_Kernel_TracerCfd
};


#endif /*KERNEL_H_*/

