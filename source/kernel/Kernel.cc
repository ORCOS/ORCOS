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
#include "lib/defines.h"
#include "lwip/netif.h"
#include "comm/hcibluetooth/HCI.hh"

#define LINEFEED "\r"

extern Kernel* theOS;

Board_ClockCfdCl* theClock = 0;
Board_TimerCfdCl* theTimer = 0;

/*! The LoopBack network interface for lwip */
//static struct netif LoopBackNetif;

// set netmask for this device
//static struct ip4_addr lo_netmask;
//static struct ip4_addr lo_IpAddr;

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

    // set all members to 0 for safety reasons
    cpuManager 	= 0;
    fileManager = 0;
    taskManager = 0;
    protopool	= 0;
    stdInputDevice 	= 0;
    stdOutputDevice = 0;
    board 		= 0;

    //-------------------------------------------------------
    // Initialize all static member variables here!
    // Important to do that before creating any objects of that kind
    //--------------------------------------------------------
    Resource::initialize();
    Task::initialize();
    Thread::initialize();
    CommDeviceDriver::initialize();
    BlockDeviceDriver::initialize();
    USBDevice::initialize();

    this->errorHandler = new TaskErrorHandler();
    // create the Ram Manager using a simple paging algorithm
    this->ramManager = new PagedRamMemManager();
    // create the cpudispatcher with scheduler
    this->cpuManager 	= new SingleCPUDispatcher( );
    // create the file	manager implementing the filesystem
    this->fileManager = new SimpleFileManager( );
    // set the idlethread of the cpudispatcher
    this->cpuManager->setIdleThread( new IdleThread( ) );
    // create the "/usb/" directory which will contain all usb drivers
    this->usbDriverLib 	= new USBDriverLibrary();
    // create the PartitionMananger which handles block device partitions
    this->partitionManager = new PartitionManager();
    // create the Task Manager which holds the list of all tasks
    this->taskManager = new TaskManager();
    // be sure the initial loaded set of tasks is registered at the ramManager
    taskManager->registerMemPages();

    // Add support for smsc95xx ethernet over USB devices
    new SMSC95xxUSBDeviceDriverFactory("smsc95xx");

    // Add support for USB SCSI Bulk only Mass Storage Devices
    new MassStorageSCSIUSBDeviceDriverFactory("msd_scsi_bot");

#if USE_TRACE
    // create the debug trace class
   // trace = new Trace();
#endif

#if HAS_PROCFS_ENABLED
    Directory* procDir = new SimpleProcfs();
    fileManager->registerResource(procDir);
#endif

    // Initialize the Internet Protocol Stack
    lwip_init();

    // now initialize the device drivers
    // since some thread classes rely on classes like the timer or the clock
    this->initDeviceDrivers();

    // initialize protocol pool here since it depends on the device drivers
    // all commdevices need to be created before the protocol pool is created
    protopool = new ProtocolPool( );

#ifdef HAS_Kernel_LoggerCfd
    LoggerCfd = new NEW_Kernel_LoggerCfd;
#endif
    LOG(KERNEL,INFO,(KERNEL,INFO,"Initialized Device Driver"));
    LOG(KERNEL,INFO,(KERNEL,INFO,"Created Protocol Pool"));

    LOG(KERNEL,INFO,(KERNEL,INFO,"Platform RAM: [0x%x - 0x%x]",&__RAM_START, &__RAM_END));

    // output some memory layout and usage information
    LOG(KERNEL,INFO,(KERNEL,INFO,".text_start at 0x%x, .text_end at 0x%x" ,&_text_start,&_text_end));
    LOG(KERNEL,INFO,(KERNEL,INFO,".text size  %d" ,(int) &_text_end - (int) &_text_start));

    if ((int) &_heap_start - (int) &_data_start <= 0) ERROR("Data Area mangled! Check ELF/Linkerscript for used but not specified sections!");

    LOG(KERNEL,INFO,(KERNEL,INFO,".data_start at 0x%x, .data_end at 0x%x" ,&_data_start,&_heap_start));
    LOG(KERNEL,INFO,(KERNEL,INFO,".data size  %d" ,(int) &_heap_start - (int) &_data_start));
    LOG(KERNEL,INFO,(KERNEL,INFO,".heap_start at 0x%x, .heap_end at 0x%x" ,&_heap_start,&_heap_end));
    LOG(KERNEL,INFO,(KERNEL,INFO,".heap size  %d" ,(int) &_heap_end - (int) &_heap_start));

    LOG(KERNEL,INFO,(KERNEL,INFO,".__stack at 0x%x" ,&__stack));
    LOG(KERNEL,INFO,(KERNEL,INFO,"Kernel Ends at 0x%x" ,&__KERNELEND));

#if USE_SAFE_KERNEL_STACKS
    LOG(KERNEL,INFO,(KERNEL,INFO,"Available Safe Kernel Stacks: %d." ,((int) &__stack - (int) &_heap_end) / KERNEL_STACK_SIZE));
#endif

#ifdef HAS_MemoryManager_HatLayerCfd
    // create the hat layer object.
    // this will also create the initial memory mappings
    MemoryManager_HatLayerCfdCl::initialize();
    HatLayerCfd = new MemoryManager_HatLayerCfdCl();
#endif

#ifdef HAS_MemoryManager_HatLayerCfd
    // now enable HAT for the task creation
    // get a pointer to the hat layer (independent from the MM)

    LOG(KERNEL,INFO,(KERNEL,INFO,"Enabling HAT."));
    this->getHatLayer()->enableHAT();

#endif

#if USE_WORKERTASK
    LOG(KERNEL,INFO,(KERNEL,INFO,"Initializing Workertask."));
    // initialize the worker task
    theWorkerTask = new WorkerTask();
    this->getTaskDatabase()->addTail(theWorkerTask);
    theWorkerTask->myTaskDbItem = this->getTaskDatabase()->getTail();

#if LWIP_TCP | LWIP_ARP
	PeriodicFunctionCall* jobparam = new PeriodicFunctionCall;
	jobparam->functioncall.objectptr = new lwipTMR; // call this object
	jobparam->functioncall.parameterptr = 0; // store the index of the request
	jobparam->functioncall.time = theOS->getClock()->getTimeSinceStartup() + 200 ms ; // call the first time in 200 ms
	jobparam->period = 250 ms ; // set to 200 ms

	theWorkerTask->addJob(PeriodicFunctionCallJob, 0,jobparam, 250000);
#endif

#ifdef HAS_Board_HCICfd
    LOG(KERNEL,INFO,(KERNEL,INFO,"Initializing HCI..."));

	/*PeriodicFunctionCall* hcijobparam = new PeriodicFunctionCall;
	hcijobparam->functioncall.objectptr = new hciTMR; // call this object
	//hcijobparam->functioncall.parameterptr = 0; // store the index of the request
	hcijobparam->functioncall.time = theOS->getClock()->getTimeSinceStartup() + 1000 ms ;
	hcijobparam->period = 1000 ms ;

	theWorkerTask->addJob(PeriodicFunctionCallJob, 0,hcijobparam, 1000000);*/

    HCI* bt_dev = new HCI();

	LOG(KERNEL,INFO,(KERNEL,INFO,"Initialized HCI."));
#endif

#else
#if LWIP_TCP | LWIP_ARP
	LOG(KERNEL,WARN,(KERNEL,WARN,"TCP and ARP will not work correctly without workerthreads!!"));
	#warning "LWIP_TCP or LWIP_ARP defined without supporting Workerthreads.. \n The IPstack will not work correctly without workerthreads.."
#endif
#endif

	LOG(KERNEL,INFO,(KERNEL,INFO,"Initializing Task Set"));

	taskManager->initialize();



#ifdef HAS_Kernel_ServiceDiscoveryCfd
    ServiceDiscoveryCfd = new NEW_Kernel_ServiceDiscoveryCfd;
	LOG(KERNEL, INFO, (KERNEL, INFO, "ServiceDiscovery at:0x%x",getServiceDiscovery()));
#endif


#ifdef HAS_Kernel_MigrationManagerCfd
    MigrationManagerCfd = new NEW_Kernel_MigrationManagerCfd;
    LOG(KERNEL, INFO, (KERNEL, INFO, "MigrationManager at:0x%x",getMigrationManager()));

    /*sockaddr destination;

    destination.port_data =   1;
    destination.sa_data =     IP4_ADDR(127,0,0,1);

    Task* t = (Task*) this->taskDatabase->getTail()->getData();
    MigrationManagerCfd->migrateTask(t,&destination);*/
#endif

    // Now we are done. start scheduling.
    // this gives the scheduler the chance to setup
    // needed components (e.g timer period) and
    // precalculate the schedule if applicable
    this->getCPUScheduler()->startScheduling();

    LOG(KERNEL,INFO,(KERNEL,INFO,"Scheduler initialized"));


    theTimer->setTimer(0);
    theTimer->enable();


    LOG(KERNEL,INFO,(KERNEL,INFO,"Enabled Hardware Timer"));
    LOG(KERNEL,INFO,(KERNEL,INFO,"Starting Dispatch Process"));
    LOG(KERNEL,INFO,(KERNEL,INFO,"ORCOS completely booted"));

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

    printf(orcos_string);
    printf("              v1.2a ");
    printf(__DATE__);
    printf(" running.."LINEFEED);
    printf(theOS->getBoard()->getBoardInfo());


  /*  char testbytes[] __attribute__((aligned(4))) = {0x0,0x1,0xf2,0x03,0xf4,0xf5,0xf6,0xf7};
    u32_t checksum = chksum((u16_t*) &testbytes,8);

    printf(" checksum: %x",checksum);*/


    // invoke the cpumanager so it starts the first thread
    this->cpuManager->dispatch( 0 );
    while (true) {}
}

/*--------------------------------------------------------------------------*
 ** Kernel::initDeviceDrivers
 *---------------------------------------------------------------------------*/
void Kernel::initDeviceDrivers() {


    // Instance Board Class
    board = new BoardCfdCl( );
    theClock = board->getClock();

    // Realtime clock is mandatory!
    ASSERT(theClock);

    theTimer = board->getTimer();

    // Timer is mandatory!
    ASSERT(theTimer);


#ifdef HAS_Kernel_PowerManagerCfd
    // init the PowerManager
    PowerManagerCfd = new NEW_Kernel_PowerManagerCfd;
#endif

    // set StandardOutput
#ifdef HAS_Board_UARTCfd
 //  setStdOutputDevice( board->getUART() );
#else
    setStdOutputDevice(0);
#endif

#ifdef HAS_Board_ETHCfd

#if !USE_WORKERTASK
#error "YOU MUST USE WORKTASKS IF YOU WANT TO COMMUNICATE OVER ETHERNET"
#endif

    extern struct netif tEMAC0Netif;
    netif_set_up(&tEMAC0Netif);
#endif

}

/*--------------------------------------------------------------------------*
 ** Kernel:::setMemManager
 *---------------------------------------------------------------------------*/
void Kernel::setMemManager( MemoryManagerCfdCl* mm ) {
    this->memManager = mm;
}

/*--------------------------------------------------------------------------*
 ** Kernel:::getMemManager
 *---------------------------------------------------------------------------*/
MemoryManagerCfdCl* Kernel::getMemManager() {
    return this->memManager;
}

/*--------------------------------------------------------------------------*
 ** Kernel:::getCPUScheduler
 *---------------------------------------------------------------------------*/
SingleCPUDispatcher_SchedulerCfdT Kernel::getCPUScheduler() {
    if ( cpuManager ) {
        return this->cpuManager->getScheduler();
    }
    else {
        return 0;
    }
}


/*!--------------------------------------------------------------------------*
 ** Kernel::setStdOutputDevice(OPB_UART_Lite outputDevice)
 *---------------------------------------------------------------------------*/
void Kernel::setStdOutputDevice( CharacterDeviceDriver* outputDevice ) {
    this->stdOutputDevice = outputDevice;
}

/*!--------------------------------------------------------------------------*
 ** OPB_UART_Lite Kernel::getStdOutputDevice()
 *---------------------------------------------------------------------------*/
CharacterDeviceDriver* Kernel::getStdOutputDevice() {
    return this->stdOutputDevice;
}
