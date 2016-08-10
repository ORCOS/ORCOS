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

#include <filesystem/SysFs.hh>
#include "Kernel.hh"
#include "inc/sprintf.hh"
#include "lwip/netif.h"
#include "filesystem/RamSharedMemoryDevice.hh"

extern Kernel* theOS;

/********************************************
 *  Global References to important Components
 ********************************************/
/* Reference to the clock */
Board_ClockCfdCl* theClock = 0;
/* Reference to the timer used for scheduling */
Board_TimerCfdCl* theTimer = 0;
/* Reference to the interrupt controller if available */
#ifdef HAS_Board_InterruptControllerCfd
Board_InterruptControllerCfdCl* theInterruptController = 0;
#endif

/********************************************
 * Mutexes needed for subsystems
 ********************************************/
Mutex* comStackMutex;
void*  sysArchMutex;

/********************************************
 * Phyiscal addresses provided by linker script
 ********************************************/
extern void* _text_start;
extern void* _heap_start;
extern void* _heap_end;
extern void* _text_end;
extern void* _data_start;
extern void* __stack;
extern void* __KERNELEND;
extern void* __RAM_END;
extern void* __RAM_START;

/*****************************************************************************
 * Method: lwip_init()
 *
 * @description
 *  Initializes the lwip stack (TCP/UDP,etc.)
 *****************************************************************************/
extern "C" void lwip_init();

/*****************************************************************************
 * Method: dwarf_init()
 *
 * @description
 *  Initializes the dwarf debugging component
 *  used for stack backtracing on errors.
 *****************************************************************************/
extern void dwarf_init();

#ifndef PRINT_BOOT_LOGO
#define PRINT_BOOT_LOGO 1
#endif

#define ORCOS_LOGO "         __           __          __             __           __ "LINEFEED"\
        /\\ \\         /\\ \\        /\\ \\           /\\ \\         / /\\ "LINEFEED"\
       /  \\ \\       /  \\ \\      /  \\ \\         /  \\ \\       / /  \\ "LINEFEED"\
      / /\\ \\ \\     / /\\ \\ \\    / /\\ \\ \\       / /\\ \\ \\     / / /\\ \\__ "LINEFEED"\
     / / /\\ \\ \\   / / /\\ \\_\\  / / /\\ \\ \\     / / /\\ \\ \\   / / /\\ \\___\\ "LINEFEED"\
    / / /  \\ \\_\\ / / /_/ / / / / /  \\ \\_\\   / / /  \\ \\_\\  \\ \\ \\ \\/___/ "LINEFEED"\
   / / /   / / // / /__\\/ / / / /    \\/_/  / / /   / / /   \\ \\ \\ "LINEFEED"\
  / / /   / / // / /_____/ / / /          / / /   / / /_    \\ \\ \\ "LINEFEED"\
 / / /___/ / // / /\\ \\ \\  / / /________  / / /___/ / //_/\\__/ / / "LINEFEED"\
/ / /____\\/ // / /  \\ \\ \\/ / /_________\\/ / /____\\/ / \\ \\/___/ / "LINEFEED"\
\\/_________/ \\/_/    \\_\\/\\/____________/\\/_________/   \\_____\\/ "LINEFEED"\
"LINEFEED""

#define ORCOS_VERSION " 0.8a "

/*--------------------------------------------------------------------------*
 ** Kernel::Kernel
 *
 * actually the constructor is never called
 *---------------------------------------------------------------------------*/
// cppcheck-suppress uninitMemberVar
Kernel::Kernel() {
}

/*--------------------------------------------------------------------------*
 ** Kernel::~Kernel
 *---------------------------------------------------------------------------*/
Kernel::~Kernel() {
}

#include <mem/FixedSizeMemPool.hh>
FixedSizeMemPool<32, 10> testPool;

/*****************************************************************************
 * Method: Kernel::initialize()
 *
 * @description
 *  Initializes the Kernel object. This is the main entry of the kernel c++ code
 *  after kernelmain has been called. The kernel memory manager already exists.
 *  The initialization routine creates all kernel components,
 *  initializes the architecture dependent board class which in turn
 *  initializes all device driver. Afterwards all initial task contained
 *  inside the kernel image are loaded and started.
 *
 * @params
 *
 * @returns
 *
 *---------------------------------------------------------------------------*/
void Kernel::initialize() {
    _disableInterrupts();
    /* set all members to 0 for safety reasons */
    fileManager         = 0;
    taskManager         = 0;
    stdInputDevice      = 0;
    stdOutputDevice     = 0;
    board               = 0;
    errorHandler        = 0;
    theClock            = 0;
    theTimer            = 0;
    /*-------------------------------------------------------
     Initialize all static member variables here!
     Important to do that before creating any objects of that kind
     --------------------------------------------------------*/
    Resource::initialize();
    Task::initialize();
    Thread::initialize();
    CommDeviceDriver::initialize();
    BlockDeviceDriver::initialize();
    Partition::initialize();

    this->errorHandler          = new TaskErrorHandler();
    /* create the cpu dispatcher with scheduler */
    this->DispatcherCfd         = new NEW_Kernel_DispatcherCfd;
    /* create the file manager implementing the file system */
    this->fileManager           = new SimpleFileManager();
    /* Initialize the Ram Manager if configured */
    INIT_Kernel_RamManager;
    /* Initialize the Parition Manager if configured */
    INIT_FileSystems_PartitionManager;
    /* Initialize the Logger if configured */
    INIT_Kernel_Logger;

    this->idleThread = new IdleThread();

    /* initial early board init */
    board = new BoardCfdCl();
    board->early_init();

    INIT_Kernel_Tracer;

#if SYSFS_SUPPORT
    /* initialize the sysfs aka kernelvariable sub directory system*/
    KernelVariable::init();
    Directory* memDir = new Directory("mem");
    fileManager->getDirectory("/sys")->add(memDir);
    SYSFS_ADD_RO_UINT_NAMED(memDir, "used"          , theOS->getMemoryManager()->getSegment()->usedBytes);
    SYSFS_ADD_RO_UINT_NAMED(memDir, "total"         , theOS->getMemoryManager()->getSegment()->memSegSize);
    SYSFS_ADD_RO_UINT_NAMED(memDir, "memAddr"       , theOS->getMemoryManager()->getSegment()->startAddr);
    SYSFS_ADD_RO_UINT_NAMED(memDir, "last_allocated", theOS->getMemoryManager()->getSegment()->lastAllocated);
#endif

    /* create the Task Manager which holds the list of all kernel image tasks */
    this->taskManager = new TaskManager();

    /* be sure the initial loaded set of tasks is registered at the ramManager */
    this->taskManager->registerMemPages();

    /* create the interrupt manager instance to allow device drivers to register
     * irqs and handlers. */
    INIT_Kernel_InterruptManager;

#if HAS_Board_USB_HCCfd
    USBDevice::initialize();

    /* create the "/usb/" directory which will contain all usb drivers */
    this->usbDriverLib = new USBDriverLibrary();
#endif

    /* global communication stack mutex.. protects the communication stack (lwip) from
     * concurrent thread execution.. This is surely not optimal and should be improved
     * in the future. */
    comStackMutex = new Mutex("ComStack");

    /* Initialize the Internet Protocol Stack */
#if ENABLE_NETWORKING
    lwip_init();
#endif

    /* initialize debug environment */
    dwarf_init();

    INIT_Kernel_Ramdisk;

    INIT_FileSystems_RamFilesystem;

#ifdef HAS_Kernel_RamManagerCfd
    /* /dev/mem only usable with ram manager support */
    new RamSharedMemoryDevice();
#endif

#ifdef HAS_Board_HatLayerCfd
    /* create the hat layer object.
     this will also create the initial memory mappings */
    Board_HatLayerCfdCl::initialize();
    HatLayerCfd = new Board_HatLayerCfdCl();
#endif

#if USE_WORKERTASK
    LOG(KERNEL, INFO, "Initializing Kernel Task");
    /* initialize the worker task */
    theKernelTask = new KernelTask();
    theKernelTask->setName("[kwork]");
    this->getTaskDatabase()->addTail(theKernelTask);
#endif

    /* now initialize the device drivers
     since some thread classes rely on classes like the timer or the clock */
    this->initDeviceDrivers();

    /* initialize protocol pool here since it depends on the device drivers
     all commdevices need to be created before the protocol pool is created */
#if ENABLE_NETWORKING
    protopool = new ProtocolPool();
    LOG(KERNEL, INFO, "Created Protocol Pool");
#endif

    LOG(KERNEL, INFO, "Initialized Device Driver");
    LOG(KERNEL, INFO, "Platform RAM: [0x%x - 0x%x]", &__RAM_START, &__RAM_END);
    /* output some memory layout and usage information */
    LOG(KERNEL, INFO, ".text_start at 0x%x, .text_end at 0x%x" , &_text_start, &_text_end);
    LOG(KERNEL, INFO, ".text size  %d" , (int) &_text_end - (int) &_text_start);
    LOG(KERNEL, INFO, ".data_start at 0x%x, .data_end at 0x%x" , &_data_start, &_heap_start);
    LOG(KERNEL, INFO, ".data size  %d" , (int) &_heap_start - (int) &_data_start);
    LOG(KERNEL, INFO, ".heap_start at 0x%x, . heap_end at 0x%x" , &_heap_start, &_heap_end);
    LOG(KERNEL, INFO, ".heap size  %d" , (int) &_heap_end - (int) &_heap_start);
    LOG(KERNEL, INFO, ".__stack at 0x%x" , &__stack);
    LOG(KERNEL, INFO, "Kernel Ends at 0x%x" , &__KERNELEND);

    // if ((int) &_heap_start - (int) &_data_start <= 0) ERROR("Data Area mangled! Check ELF/Linkerscript for used but not specified sections!");


#ifdef HAS_Board_HatLayerCfd
    /* now enable HAT for the task creation */
    LOG(KERNEL, INFO, "Enabling HAT.");
    this->getHatLayer()->enableHAT();
#endif

    /*
     * Initialize Kernel Threads before user tasks
     */
#if USE_WORKERTASK
    KernelThread* wt = theKernelTask->getCallbackThread(0, this->idleThread, 0, 0, 0);
    if (wt) {
        wt->setName("idle");
    }
    /* add periodic kernel thread to perform basic OS services
     * permanently occupies one kernel thread */
    wt = theKernelTask->getPeriodicThread(0, new KernelServiceThread, 0, 250 ms, 8000);
    if (wt) {
        wt->setName("kernel");
    }

#if ENABLE_NETWORKING
    /* add periodic kernel thread callback to update system time using NTP.
     * Occupies one kernel thread. */
    //LOG(KERNEL, INFO, "Querying NTP Server for Time");
    //theKernelTask->getPeriodicThread(0, theOS->getClock(), 2000 ms, 3600ULL * 1000 ms, 1000);
#endif

#else // if USE_WORKERTASK
#if (LWIP_TCP | LWIP_ARP) && ENABLE_NETWORKING
    LOG(KERNEL, WARN, (KERNEL, WARN, "TCP and ARP will not work correctly without workerthreads!!"));
#warning "LWIP_TCP or LWIP_ARP defined without supporting Workerthreads.. \n The IPstack will not work correctly without workerthreads.."
#endif
#endif

#if USE_TRACE
    this->getTracer()->init();
#endif

    LOG(KERNEL, INFO, "Initializing Task Set");
    taskManager->initialize();

    /* Now we are done. start scheduling.
     this gives the scheduler the chance to setup
     needed components (e.g timer period) and
     pre-calculate the schedule if applicable */
    this->getCPUScheduler()->startScheduling();
    LOG(KERNEL, INFO, "Scheduler initialized");
    LOG(KERNEL, INFO, "ORCOS completely booted");
    LOG(KERNEL, INFO, "Starting Dispatch Process");

    /* Initial boot log flush before scheduling starts */
    getLogger()->flush();

#if PRINT_BOOT_LOGO
    printf_p(ORCOS_LOGO);
    printf_p(ORCOS_VERSION);
    printf_p(__DATE__);
    printf_p(theOS->getBoard()->getBoardInfo());
#endif

    /* Reset time again */
    //theTimer->setTimer(0);
    theTimer->enable();

    /* invoke the dispatcher so it starts the first thread */
    this->DispatcherCfd->dispatch();
    while (true)    { }
}

/*****************************************************************************
 * Method: Kernel::initDeviceDrivers()
 *
 * @description
 *  Initializes the Kernel Device driver infrastructure.
 *  Instantiates the board.
 *
 * @params
 *
 * @returns
 *
 *---------------------------------------------------------------------------*/
void Kernel::initDeviceDrivers() {
    /* - Instantiate Board Class -
     * Give components of the board a chance to
     * reference each other by using theOS->getBoard() pointer. */
    board->initialize();            /* now initialize */
    theClock = board->getClock();   /* Get global reference to Clock */
    theTimer = board->getTimer();   /* Get global reference to the Timer*/
#ifdef HAS_Board_InterruptControllerCfd
    theInterruptController = board->getInterruptController();
#endif
    ASSERT(theClock);               /* Realtime clock is mandatory! */
    ASSERT(theTimer);               /* Timer is mandatory! */

#ifdef HAS_Kernel_PowerManagerCfd
    /* Initialize the PowerManager */
    PowerManagerCfd = new NEW_Kernel_PowerManagerCfd;
#endif

    /* set StandardOutput */
#ifdef HAS_Board_UARTCfd
    // CharacterDeviceDriver* theOS->getFileManager()->getResourceByNameandType(STDOUT,cStreamDevice);
    //  setStdOutputDevice( board->getUART() );
#else
    setStdOutputDevice(0);
#endif
}

/*****************************************************************************
 * Method: Kernel::getCPUScheduler()
 *
 * @description
 *  Returns the configured cpu scheduler.
 *
 * @params
 *
 * @returns
 *  Kernel_SchedulerCfdT:   The scheduler object or null if not yet initialized.
 *                          However this should never be the case when this code
 *                          is called.
 *---------------------------------------------------------------------------*/
Kernel_SchedulerCfdT Kernel::getCPUScheduler() {
    if ( DispatcherCfd ) {
        return (this->DispatcherCfd->getScheduler());
    } else {
        return (0);
    }
}

/*****************************************************************************
 * Method: Kernel::setStdOutputDevice(CharacterDevice* outputDevice)
 *
 * @description
 *  Returns the configured cpu scheduler.
 *
 * @params
 *  outputDevice: The new ouput device to be used as standard out.
 *---------------------------------------------------------------------------*/
void Kernel::setStdOutputDevice(CharacterDevice* outputDevice) {
    this->stdOutputDevice = outputDevice;
}

/*****************************************************************************
 * Method: Kernel::getStdOutputDevice()
 *
 * @description
 *  Returns the configured cpu scheduler.
 *
 * @params
 *
 * @returns
 *  CharacterDevice*: Pointer to the current standard output device or null.
 *---------------------------------------------------------------------------*/
CharacterDevice* Kernel::getStdOutputDevice() {
    return (this->stdOutputDevice);
}
