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

#include "Kernel.hh"
#include "inc/sprintf.hh"
#include "lwip/netif.h"

#define LINEFEED "\r"

extern Kernel* theOS;

Board_ClockCfdCl* theClock = 0;
Board_TimerCfdCl* theTimer = 0;

extern void* _text_start;
extern void* _heap_start;
extern void* _heap_end;
extern void* _text_end;
extern void* _data_start;
extern void* __stack;
extern void* __KERNELEND;
extern void* __RAM_END;
extern void* __RAM_START;

extern "C" void lwip_init();
extern void dwarf_init();

#ifndef PRINT_BOOT_LOGO
#define PRINT_BOOT_LOGO 1
#endif

#define orcos_string "         __           __          __             __           __ "LINEFEED"\
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



/*--------------------------------------------------------------------------*
 ** Kernel::Kernel
 *
 * actually the constructor is never called
 *---------------------------------------------------------------------------*/
Kernel::Kernel() {

}

/*--------------------------------------------------------------------------*
 ** Kernel::~Kernel
 *---------------------------------------------------------------------------*/
Kernel::~Kernel() {

}

/*--------------------------------------------------------------------------*
 ** Kernel::initialize
 *---------------------------------------------------------------------------*/
void Kernel::initialize() {

    /* set all members to 0 for safety reasons */
    fileManager         = 0;
    taskManager         = 0;
    stdInputDevice      = 0;
    stdOutputDevice     = 0;
    board               = 0;
    errorHandler        = 0;

    /*-------------------------------------------------------
     Initialize all static member variables here!
     Important to do that before creating any objects of that kind
     --------------------------------------------------------*/
    Resource::initialize();
    Task::initialize();
    Thread::initialize();
    CommDeviceDriver::initialize();
    BlockDeviceDriver::initialize();


    this->errorHandler = new TaskErrorHandler();
#ifdef HAS_Kernel_RamManagerCfd
    /* create the Ram Manager using a simple paging algorithm */
    this->RamManagerCfd = new NEW_Kernel_RamManagerCfd;
#endif
    /* create the cpudispatcher with scheduler */
    this->DispatcherCfd = new NEW_Kernel_DispatcherCfd;
    /* create the file	manager implementing the filesystem */
    this->fileManager = new SimpleFileManager();
    /* create the PartitionMananger which handles block device partitions */
#ifdef HAS_FileSystems_PartitionManagerCfd
    /* create the Ram Manager using a simple paging algorithm */
    this->PartitionManagerCfd = new NEW_FileSystems_PartitionManagerCfd;
#endif

#if USE_TRACE
    this->tracer = new Trace();
#endif

    /* create the Task Manager which holds the list of all tasks */
    this->taskManager = new TaskManager();

    /* be sure the initial loaded set of tasks is registered at the ramManager */
    taskManager->registerMemPages();

#if HAS_Kernel_InterruptManagerCfd
    /* create the interrupt manager instance to allow device drivers to register
     * their irq numbers and handlers. */
    this->InterruptManagerCfd = new NEW_Kernel_InterruptManagerCfd;
#endif

#if HAS_Board_USB_HCCfd
    USBDevice::initialize();

#if HAS_USBDriver_FactoryCfd
    /* create the "/usb/" directory which will contain all usb drivers */
    this->usbDriverLib = new USBDriverLibrary();
#endif

#if HAS_USBDriver_SMSC95xxCfd
    /* Add support for smsc95xx ethernet over USB devices */
    new SMSC95xxUSBDeviceDriverFactory("smsc95xx");
#endif

#if HAS_USBDriver_MassStorageCfd
    /* Add support for USB SCSI Bulk only Mass Storage Devices */
    new MassStorageSCSIUSBDeviceDriverFactory("msd_scsi_bot");
#endif
#endif

#ifdef HAS_Kernel_RamdiskCfd
    INIT_Kernel_RamdiskCfd;
    this->RamdiskCfd = new NEW_Kernel_RamdiskCfd;
#endif

#if HAS_PROCFS_ENABLED
    Directory* procDir = new SimpleProcfs();
    fileManager->registerResource(procDir);
#endif

#if HAS_Kernel_LoggerCfd
    LoggerCfd = new NEW_Kernel_LoggerCfd;
#endif

    /* Initialize the Internet Protocol Stack */
#if ENABLE_NETWORKING
    lwip_init();
#endif

    dwarf_init();

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
    LOG(KERNEL, INFO, "Platform RAM: [0x%x - 0x%x]",&__RAM_START, &__RAM_END);
    /* output some memory layout and usage information */
    LOG(KERNEL, INFO, ".text_start at 0x%x, .text_end at 0x%x" ,&_text_start,&_text_end);
    LOG(KERNEL, INFO, ".text size  %d" ,(int) &_text_end - (int) &_text_start);
    LOG(KERNEL, INFO, ".data_start at 0x%x, .data_end at 0x%x" ,&_data_start,&_heap_start);
    LOG(KERNEL, INFO, ".data size  %d" ,(int) &_heap_start - (int) &_data_start);
    LOG(KERNEL, INFO, ".heap_start at 0x%x, .heap_end at 0x%x" ,&_heap_start,&_heap_end);
    LOG(KERNEL, INFO, ".heap size  %d" ,(int) &_heap_end - (int) &_heap_start);
    LOG(KERNEL, INFO, ".__stack at 0x%x" ,&__stack);
    LOG(KERNEL, INFO, "Kernel Ends at 0x%x" ,&__KERNELEND);

    dwarf_init();
    // if ((int) &_heap_start - (int) &_data_start <= 0) ERROR("Data Area mangled! Check ELF/Linkerscript for used but not specified sections!");

#if USE_SAFE_KERNEL_STACKS
    /* This is a PPC extension for context switches */
    LOG(KERNEL,INFO,"Available Safe Kernel Stacks: %d." ,((int) &__stack - (int) &_heap_end) / KERNEL_STACK_SIZE);
#endif

#ifdef HAS_Board_HatLayerCfd
    /* create the hat layer object.
     this will also create the initial memory mappings */
    Board_HatLayerCfdCl::initialize();
    HatLayerCfd = new Board_HatLayerCfdCl();
#endif

#ifdef HAS_Board_HatLayerCfd
    /* now enable HAT for the task creation */
    LOG(KERNEL, INFO, "Enabling HAT.");
    this->getHatLayer()->enableHAT();
#endif

    /*
     * Initialize Workertask before user tasks
     */
#if USE_WORKERTASK
    LOG(KERNEL, INFO, "Initializing Workertask.");
    // initialize the worker task
    theWorkerTask = new WorkerTask();
    theWorkerTask->setName("Kernel");
    this->getTaskDatabase()->addTail(theWorkerTask);
    theWorkerTask->myTaskDbItem = this->getTaskDatabase()->getTail();

#if (LWIP_TCP | LWIP_ARP) && ENABLE_NETWORKING
    PeriodicFunctionCall* jobparam      = new PeriodicFunctionCall;
    jobparam->functioncall.objectptr    = new lwipTMR; /* call this object */
    jobparam->functioncall.parameterptr = 0; /* no parameter */
    jobparam->functioncall.time         = 0; /* call directly!! */
    jobparam->period                    = 250 ms;  /* set to 250 ms */
    theWorkerTask->addJob(PeriodicFunctionCallJob, 0, jobparam, 250000);

    LOG(KERNEL, INFO, "Querying NTP Server for Time");
    TimedFunctionCall* ntpupdate        = new TimedFunctionCall;
    ntpupdate->objectptr                = theOS->getClock();
    ntpupdate->parameterptr             = 0;
    ntpupdate->time                     = 10000 ms;
    theWorkerTask->addJob(TimedFunctionCallJob, 0, ntpupdate, 100);
#endif

#else
#if (LWIP_TCP | LWIP_ARP) && ENABLE_NETWORKING
    LOG(KERNEL,WARN,(KERNEL,WARN,"TCP and ARP will not work correctly without workerthreads!!"));
#warning "LWIP_TCP or LWIP_ARP defined without supporting Workerthreads.. \n The IPstack will not work correctly without workerthreads.."
#endif
#endif

    /*
     * Now initialize the user tasks deployed within this Image.
     * The Task Manager will check the integrity and create memory maps.
     */
#if USE_TRACE
    this->tracer->init();
#endif

    LOG(KERNEL, INFO, "Initializing Task Set");
    taskManager->initialize();

    /*
     * Initialize Service Discovery if configured
     */
#ifdef HAS_Kernel_ServiceDiscoveryCfd
    ServiceDiscoveryCfd = new NEW_Kernel_ServiceDiscoveryCfd;
    LOG(KERNEL, INFO, (KERNEL, INFO, "ServiceDiscovery at:0x%x",getServiceDiscovery()));
#endif

    /*
     * Initialize the Migration Manager if configured
     */
#ifdef HAS_Kernel_MigrationManagerCfd
    MigrationManagerCfd = new NEW_Kernel_MigrationManagerCfd;
    LOG(KERNEL, INFO, (KERNEL, INFO, "MigrationManager at:0x%x",getMigrationManager()));
#endif

    /* Now we are done. start scheduling.
     this gives the scheduler the chance to setup
     needed components (e.g timer period) and
     pre-calculate the schedule if applicable */
    this->getCPUScheduler()->startScheduling();
    LOG(KERNEL, INFO, "Scheduler initialized");
    LOG(KERNEL, INFO, "ORCOS completely booted");
    LOG(KERNEL, INFO, "Starting Dispatch Process");

#if 0
    LOG(KERNEL,INFO,"sizeof(Resource)        :%d",sizeof(Resource));
    LOG(KERNEL,INFO,"sizeof(GenericDevice)   :%d",sizeof(GenericDeviceDriver));
    LOG(KERNEL,INFO,"sizeof(CharacterDevice) :%d",sizeof(CharacterDevice));
    LOG(KERNEL,INFO,"sizeof(File)            :%d",sizeof(File));
    LOG(KERNEL,INFO,"sizeof(Directory)       :%d",sizeof(Directory));
#endif

#if PRINT_BOOT_LOGO
    /*
     * Print some Boot Logo
     */
    printf_p(orcos_string);
    printf_p(" v1.4 ");
    printf_p(__DATE__);
    printf_p(theOS->getBoard()->getBoardInfo());
#endif

    /* Reset time again */
    theTimer->setTimer(0);
    theTimer->enable();

    /* invoke the dispatcher so it starts the first thread */
    this->DispatcherCfd->dispatch();
    while (true)    { }
}

/*--------------------------------------------------------------------------*
 ** Kernel::initDeviceDrivers
 *---------------------------------------------------------------------------*/
void Kernel::initDeviceDrivers() {

    /* - Instantiate Board Class -
     * Give components of the board a chance to
     * reference each other by using theOS->getBoard() pointer. */
    board = new BoardCfdCl();
    board->initialize();            /* now initialize */
    theClock = board->getClock();   /* Get global reference to Clock */
    theTimer = board->getTimer();   /* Get global reference to the Timer*/

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

/*--------------------------------------------------------------------------*
 ** Kernel:::getCPUScheduler
 *---------------------------------------------------------------------------*/
Kernel_SchedulerCfdT Kernel::getCPUScheduler()
{
    if ( DispatcherCfd )
    {
        return (this->DispatcherCfd->getScheduler());
    }
    else
    {
        return (0);
    }
}

/*!--------------------------------------------------------------------------*
 ** Kernel::setStdOutputDevice(OPB_UART_Lite outputDevice)
 *---------------------------------------------------------------------------*/
void Kernel::setStdOutputDevice(CharacterDevice* outputDevice) {
    this->stdOutputDevice = outputDevice;
}

/*!--------------------------------------------------------------------------*
 ** OPB_UART_Lite Kernel::getStdOutputDevice()
 *---------------------------------------------------------------------------*/
CharacterDevice* Kernel::getStdOutputDevice() {
    return (this->stdOutputDevice);
}
