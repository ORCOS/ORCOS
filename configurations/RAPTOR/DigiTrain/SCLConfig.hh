
// configuration of class Mutex

// configuration of member Scheduler of class Mutex
#define Mutex_Scheduler_hh <scheduler/PriorityThreadScheduler.hh>
#define Mutex_Scheduler_cc <scheduler/PriorityThreadScheduler.cc>
#define Mutex_SchedulerCfdCl PriorityThreadScheduler
#define Mutex_SchedulerCfdT PriorityThreadScheduler*
#define DEF_Mutex_SchedulerCfd \
public: \
    void setScheduler(PriorityThreadScheduler* o) { } \
    PriorityThreadScheduler* getScheduler() { return 0; }

// configuration of class Semaphore

// configuration of member Scheduler of class Semaphore
#define Semaphore_Scheduler_hh <scheduler/PriorityThreadScheduler.hh>
#define Semaphore_Scheduler_cc <scheduler/PriorityThreadScheduler.cc>
#define Semaphore_SchedulerCfdCl PriorityThreadScheduler
#define Semaphore_SchedulerCfdT PriorityThreadScheduler*
#define DEF_Semaphore_SchedulerCfd \
public: \
    void setScheduler(PriorityThreadScheduler* o) { } \
    PriorityThreadScheduler* getScheduler() { return 0; }

// configuration of class SingleCPUDispatcher

// configuration of member Scheduler of class SingleCPUDispatcher
#define SingleCPUDispatcher_Scheduler_hh <scheduler/RateMonotonicThreadScheduler.hh>
#define SingleCPUDispatcher_Scheduler_cc <scheduler/RateMonotonicThreadScheduler.cc>
#define SingleCPUDispatcher_SchedulerCfdCl RateMonotonicThreadScheduler
#define SingleCPUDispatcher_SchedulerCfdT RateMonotonicThreadScheduler*
#define HAS_SingleCPUDispatcher_SchedulerCfd 1
#define SingleCPUDispatcher_Scheduler_IN_USERSPACE 0
#define DEF_SingleCPUDispatcher_SchedulerCfd \
private: \
    RateMonotonicThreadScheduler* SchedulerCfd; \
public: \
    void setScheduler(RateMonotonicThreadScheduler* o) {SchedulerCfd = o;} \
    RateMonotonicThreadScheduler* getScheduler() { return (RateMonotonicThreadScheduler*) SchedulerCfd; }
#define NEW_SingleCPUDispatcher_SchedulerCfd SingleCPUDispatcher_SchedulerCfdCl()

// configuration of class Thread
#define ThreadCfd_hh <process/RealTimeThread.hh>
#define ThreadCfdCl RealTimeThread

// configuration of class MemoryManager
#define MemoryManagerCfd_hh <mem/SequentialFitMemManager.hh>
#define MemoryManagerCfdCl SequentialFitMemManager

// configuration of member Seg of class MemoryManager
#define MemoryManager_Seg_hh <mem/MemResource.hh>
#define MemoryManager_Seg_cc <mem/MemResource.cc>
#define MemoryManager_SegCfdCl MemResource
#define MemoryManager_SegCfdT MemResource*
#define HAS_MemoryManager_SegCfd 1
#define MemoryManager_Seg_IN_USERSPACE 0
#define DEF_MemoryManager_SegCfd \
private: \
    MemResource* SegCfd; \
public: \
    void setSeg(MemResource* o) {SegCfd = o;} \
    MemResource* getSeg() { return (MemResource*) SegCfd; }
#define NEW_MemoryManager_SegCfd MemoryManager_SegCfdCl()

// configuration of member HatLayer of class MemoryManager
#define MemoryManager_HatLayer_hh <arch/PPC40x/PPC405HatLayer.hh>
#define MemoryManager_HatLayer_cc <arch/PPC40x/PPC405HatLayer.cc>
#define MemoryManager_HatLayerCfdCl PPC405HatLayer
#define MemoryManager_HatLayerCfdT PPC405HatLayer*
#define DEF_MemoryManager_HatLayerCfd \
public: \
    void setHatLayer(PPC405HatLayer* o) { } \
    PPC405HatLayer* getHatLayer() { return 0; }

// configuration of member FirstFit of class MemoryManager
#define MemoryManager_FirstFit_hh <.hh>
#define MemoryManager_FirstFit_cc <.cc>
#define MemoryManager_FirstFitCfdCl 
#define MemoryManager_FirstFitCfdT *
#define DEF_MemoryManager_FirstFitCfd \
public: \
    void setFirstFit(* o) { } \
    * getFirstFit() { return 0; }

// configuration of member NextFit of class MemoryManager
#define MemoryManager_NextFit_hh <.hh>
#define MemoryManager_NextFit_cc <.cc>
#define MemoryManager_NextFitCfdCl 
#define MemoryManager_NextFitCfdT *
#define DEF_MemoryManager_NextFitCfd \
public: \
    void setNextFit(* o) { } \
    * getNextFit() { return 0; }

// configuration of member BestFit of class MemoryManager
#define MemoryManager_BestFit_hh <.hh>
#define MemoryManager_BestFit_cc <.cc>
#define MemoryManager_BestFitCfdCl 
#define MemoryManager_BestFitCfdT *
#define DEF_MemoryManager_BestFitCfd \
public: \
    void setBestFit(* o) { } \
    * getBestFit() { return 0; }

// configuration of member WorstFit of class MemoryManager
#define MemoryManager_WorstFit_hh <.hh>
#define MemoryManager_WorstFit_cc <.cc>
#define MemoryManager_WorstFitCfdCl 
#define MemoryManager_WorstFitCfdT *
#define DEF_MemoryManager_WorstFitCfd \
public: \
    void setWorstFit(* o) { } \
    * getWorstFit() { return 0; }

// configuration of class Board
#define BoardCfd_hh <arch/PPC40x/RAPTOR/RaptorBoard.hh>
#define BoardCfdCl RaptorBoard

// configuration of member Processor of class Board
#define Board_Processor_hh <arch/PPC40x/PPC405FXProcessor.hh>
#define Board_Processor_cc <arch/PPC40x/PPC405FXProcessor.cc>
#define Board_ProcessorCfdCl PPC405FXProcessor
#define Board_ProcessorCfdT PPC405FXProcessor*
#define HAS_Board_ProcessorCfd 1
#define Board_Processor_IN_USERSPACE 0
#define DEF_Board_ProcessorCfd \
private: \
    PPC405FXProcessor* ProcessorCfd; \
public: \
    void setProcessor(PPC405FXProcessor* o) {ProcessorCfd = o;} \
    PPC405FXProcessor* getProcessor() { return (PPC405FXProcessor*) ProcessorCfd; }
#define NEW_Board_ProcessorCfd Board_ProcessorCfdCl()

// configuration of member Timer of class Board
#define Board_Timer_hh <arch/PPC40x/PPC405ProgrammableIntervalTimer.hh>
#define Board_Timer_cc <arch/PPC40x/PPC405ProgrammableIntervalTimer.cc>
#define Board_TimerCfdCl PPC405ProgrammableIntervalTimer
#define Board_TimerCfdT PPC405ProgrammableIntervalTimer*
#define HAS_Board_TimerCfd 1
#define Board_Timer_IN_USERSPACE 0
#define DEF_Board_TimerCfd \
private: \
    PPC405ProgrammableIntervalTimer* TimerCfd; \
public: \
    void setTimer(PPC405ProgrammableIntervalTimer* o) {TimerCfd = o;} \
    PPC405ProgrammableIntervalTimer* getTimer() { return (PPC405ProgrammableIntervalTimer*) TimerCfd; }
#define NEW_Board_TimerCfd Board_TimerCfdCl()

// configuration of member Clock of class Board
#define Board_Clock_hh <arch/PPC40x/PPC405Clock.hh>
#define Board_Clock_cc <arch/PPC40x/PPC405Clock.cc>
#define Board_ClockCfdCl PPC405Clock
#define Board_ClockCfdT PPC405Clock*
#define HAS_Board_ClockCfd 1
#define Board_Clock_IN_USERSPACE 0
#define Board_Clock_NAME "clock"
#define DEF_Board_ClockCfd \
private: \
    PPC405Clock* ClockCfd; \
public: \
    void setClock(PPC405Clock* o) {ClockCfd = o;} \
    PPC405Clock* getClock() { return (PPC405Clock*) ClockCfd; }
#define NEW_Board_ClockCfd Board_ClockCfdCl("clock")

// configuration of member Watchdog of class Board
#define Board_Watchdog_hh <arch/PPC40x/PPC405Watchdog.hh>
#define Board_Watchdog_cc <arch/PPC40x/PPC405Watchdog.cc>
#define Board_WatchdogCfdCl PPC405Watchdog
#define Board_WatchdogCfdT PPC405Watchdog*
#define Board_Watchdog_NAME "watchdog"
#define DEF_Board_WatchdogCfd \
public: \
    void setWatchdog(PPC405Watchdog* o) { } \
    PPC405Watchdog* getWatchdog() { return 0; }

// configuration of member UART of class Board
#define Board_UART_hh <arch/PPC40x/RAPTOR/OPB_UART_Lite.hh>
#define Board_UART_cc <arch/PPC40x/RAPTOR/OPB_UART_Lite.cc>
#define Board_UARTCfdCl OPB_UART_Lite
#define Board_UARTCfdT OPB_UART_Lite*
#define HAS_Board_UARTCfd 1
#define Board_UART_IN_USERSPACE 0
#define Board_UART_NAME "serial0"
#define Board_UART_MMIO_PHYS_ADDR 0x90210000
#define Board_UART_MMIO_LENGTH (unint4)16
#define DEF_Board_UARTCfd \
private: \
    OPB_UART_Lite* UARTCfd; \
public: \
    void setUART(OPB_UART_Lite* o) {UARTCfd = o;} \
    OPB_UART_Lite* getUART() { return (OPB_UART_Lite*) UARTCfd; }
#define NEW_Board_UARTCfd Board_UARTCfdCl("serial0",0x90210000)

// configuration of member UART2 of class Board
#define Board_UART2_hh <arch/PPC40x/RAPTOR/UART16550.hh>
#define Board_UART2_cc <arch/PPC40x/RAPTOR/UART16550.cc>
#define Board_UART2CfdCl UART16550
#define Board_UART2CfdT UART16550*
#define Board_UART2_NAME "serial1"
#define Board_UART2_MMIO_PHYS_ADDR 0x90300000
#define Board_UART2_MMIO_LENGTH (unint4)16
#define DEF_Board_UART2Cfd \
public: \
    void setUART2(UART16550* o) { } \
    UART16550* getUART2() { return 0; }

// configuration of member LED of class Board
#define Board_LED_hh <arch/PPC40x/RAPTOR/LED.hh>
#define Board_LED_cc <arch/PPC40x/RAPTOR/LED.cc>
#define Board_LEDCfdCl LED
#define Board_LEDCfdT LED*
#define HAS_Board_LEDCfd 1
#define Board_LED_IN_USERSPACE 0
#define Board_LED_NAME "led0"
#define Board_LED_MMIO_PHYS_ADDR 0x90220000
#define Board_LED_MMIO_LENGTH (unint4)16
#define DEF_Board_LEDCfd \
private: \
    LED* LEDCfd; \
public: \
    void setLED(LED* o) {LEDCfd = o;} \
    LED* getLED() { return (LED*) LEDCfd; }
#define NEW_Board_LEDCfd Board_LEDCfdCl("led0",0x90220000)

// configuration of member InterruptHandler of class Board
#define Board_InterruptHandler_hh <arch/PPC40x/PPC405InterruptHandler.hh>
#define Board_InterruptHandler_cc <arch/PPC40x/PPC405InterruptHandler.cc>
#define Board_InterruptHandlerCfdCl PPC405InterruptHandler
#define Board_InterruptHandlerCfdT PPC405InterruptHandler*
#define HAS_Board_InterruptHandlerCfd 1
#define Board_InterruptHandler_IN_USERSPACE 0
#define DEF_Board_InterruptHandlerCfd \
private: \
    PPC405InterruptHandler* InterruptHandlerCfd; \
public: \
    void setInterruptHandler(PPC405InterruptHandler* o) {InterruptHandlerCfd = o;} \
    PPC405InterruptHandler* getInterruptHandler() { return (PPC405InterruptHandler*) InterruptHandlerCfd; }
#define NEW_Board_InterruptHandlerCfd Board_InterruptHandlerCfdCl()

// configuration of member InterruptController of class Board
#define Board_InterruptController_hh <arch/PPC40x/RAPTOR/OPB_Interrupt_Controller.hh>
#define Board_InterruptController_cc <arch/PPC40x/RAPTOR/OPB_Interrupt_Controller.cc>
#define Board_InterruptControllerCfdCl OPB_Interrupt_Controller
#define Board_InterruptControllerCfdT OPB_Interrupt_Controller*
#define HAS_Board_InterruptControllerCfd 1
#define Board_InterruptController_IN_USERSPACE 0
#define DEF_Board_InterruptControllerCfd \
private: \
    OPB_Interrupt_Controller* InterruptControllerCfd; \
public: \
    void setInterruptController(OPB_Interrupt_Controller* o) {InterruptControllerCfd = o;} \
    OPB_Interrupt_Controller* getInterruptController() { return (OPB_Interrupt_Controller*) InterruptControllerCfd; }
#define NEW_Board_InterruptControllerCfd Board_InterruptControllerCfdCl()

// configuration of member ETH of class Board
#define Board_ETH_hh <arch/PPC40x/RAPTOR/PLB_EMAC0.hh>
#define Board_ETH_cc <arch/PPC40x/RAPTOR/PLB_EMAC0.cc>
#define Board_ETHCfdCl PLB_EMAC0
#define Board_ETHCfdT PLB_EMAC0*
#define HAS_Board_ETHCfd 1
#define Board_ETH_IN_USERSPACE 0
#define Board_ETH_NAME "eth0"
#define Board_ETH_MMIO_PHYS_ADDR 0x90000000
#define Board_ETH_MMIO_LENGTH (unint4)1024*1024
#define Board_ETH_UNIQUEID 0x00,0x13,0x72,0x74,0x34,0x99
#define DEF_Board_ETHCfd \
private: \
    PLB_EMAC0* ETHCfd; \
public: \
    void setETH(PLB_EMAC0* o) {ETHCfd = o;} \
    PLB_EMAC0* getETH() { return (PLB_EMAC0*) ETHCfd; }
#define NEW_Board_ETHCfd Board_ETHCfdCl("eth0",0x90000000)

// configuration of member SHM of class Board
#define Board_SHM_hh <arch/Leon3/ShmDriver/ShmDriver.hh>
#define Board_SHM_cc <arch/Leon3/ShmDriver/ShmDriver.cc>
#define Board_SHMCfdCl ShmDriver
#define Board_SHMCfdT ShmDriver*
#define DEF_Board_SHMCfd \
public: \
    void setSHM(ShmDriver* o) { } \
    ShmDriver* getSHM() { return 0; }

// configuration of class Kernel
#define KernelCfd_hh <kernel/Kernel.hh>
#define KernelCfdCl Kernel

// configuration of member Logger of class Kernel
#define Kernel_Logger_hh <debug/Logger.hh>
#define Kernel_Logger_cc <debug/Logger.cc>
#define Kernel_LoggerCfdCl Logger
#define Kernel_LoggerCfdT Logger*
#define HAS_Kernel_LoggerCfd 1
#define Kernel_Logger_IN_USERSPACE 0
#define DEF_Kernel_LoggerCfd \
private: \
    Logger* LoggerCfd; \
public: \
    void setLogger(Logger* o) {LoggerCfd = o;} \
    Logger* getLogger() { return (Logger*) LoggerCfd; }
#define NEW_Kernel_LoggerCfd Kernel_LoggerCfdCl()

// configuration of member PowerManager of class Kernel
#define Kernel_PowerManager_hh <hal/PowerManager.hh>
#define Kernel_PowerManager_cc <hal/PowerManager.cc>
#define Kernel_PowerManagerCfdCl PowerManager
#define Kernel_PowerManagerCfdT PowerManager*
#define DEF_Kernel_PowerManagerCfd \
public: \
    void setPowerManager(PowerManager* o) { } \
    PowerManager* getPowerManager() { return 0; }

// configuration of member TaskManager of class Kernel
#define Kernel_TaskManager_hh <process/TaskManager.hh>
#define Kernel_TaskManager_cc <process/TaskManager.cc>
#define Kernel_TaskManagerCfdCl TaskManager
#define Kernel_TaskManagerCfdT TaskManager*
#define DEF_Kernel_TaskManagerCfd \
public: \
    void setTaskManager(TaskManager* o) { } \
    TaskManager* getTaskManager() { return 0; }

// configuration of member MigrationManager of class Kernel
#define Kernel_MigrationManager_hh <migration/MigrationManager.hh>
#define Kernel_MigrationManager_cc <migration/MigrationManager.cc>
#define Kernel_MigrationManagerCfdCl MigrationManager
#define Kernel_MigrationManagerCfdT MigrationManager*
#define DEF_Kernel_MigrationManagerCfd \
public: \
    void setMigrationManager(MigrationManager* o) { } \
    MigrationManager* getMigrationManager() { return 0; }

// configuration of member ServiceDiscovery of class Kernel
#define Kernel_ServiceDiscovery_hh <comm/servicediscovery/SNServiceDiscovery.hh>
#define Kernel_ServiceDiscovery_cc <comm/servicediscovery/SNServiceDiscovery.cc>
#define Kernel_ServiceDiscoveryCfdCl SNServiceDiscovery
#define Kernel_ServiceDiscoveryCfdT SNServiceDiscovery*
#define DEF_Kernel_ServiceDiscoveryCfd \
public: \
    void setServiceDiscovery(SNServiceDiscovery* o) { } \
    SNServiceDiscovery* getServiceDiscovery() { return 0; }

// configuration of class SyscallManager
#define SyscallManagerCfd_hh <syscalls/SyscallManager.hh>
#define SyscallManagerCfdCl SyscallManager

// configuration of member sleepSyscall of class SyscallManager
#define SyscallManager_sleepSyscall_hh <.hh>
#define SyscallManager_sleepSyscall_cc <.cc>
#define SyscallManager_sleepSyscallCfdCl 
#define SyscallManager_sleepSyscallCfdT *
#define HAS_SyscallManager_sleepSyscallCfd 1
#define SyscallManager_sleepSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_sleepSyscallCfd \
private: \
    * sleepSyscallCfd; \
public: \
    void setsleepSyscall(* o) {sleepSyscallCfd = o;} \
    * getsleepSyscall() { return (*) sleepSyscallCfd; }
#define NEW_SyscallManager_sleepSyscallCfd SyscallManager_sleepSyscallCfdCl()

// configuration of member fputcSyscall of class SyscallManager
#define SyscallManager_fputcSyscall_hh <.hh>
#define SyscallManager_fputcSyscall_cc <.cc>
#define SyscallManager_fputcSyscallCfdCl 
#define SyscallManager_fputcSyscallCfdT *
#define HAS_SyscallManager_fputcSyscallCfd 1
#define SyscallManager_fputcSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_fputcSyscallCfd \
private: \
    * fputcSyscallCfd; \
public: \
    void setfputcSyscall(* o) {fputcSyscallCfd = o;} \
    * getfputcSyscall() { return (*) fputcSyscallCfd; }
#define NEW_SyscallManager_fputcSyscallCfd SyscallManager_fputcSyscallCfdCl()

// configuration of member fgetcSyscall of class SyscallManager
#define SyscallManager_fgetcSyscall_hh <.hh>
#define SyscallManager_fgetcSyscall_cc <.cc>
#define SyscallManager_fgetcSyscallCfdCl 
#define SyscallManager_fgetcSyscallCfdT *
#define HAS_SyscallManager_fgetcSyscallCfd 1
#define SyscallManager_fgetcSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_fgetcSyscallCfd \
private: \
    * fgetcSyscallCfd; \
public: \
    void setfgetcSyscall(* o) {fgetcSyscallCfd = o;} \
    * getfgetcSyscall() { return (*) fgetcSyscallCfd; }
#define NEW_SyscallManager_fgetcSyscallCfd SyscallManager_fgetcSyscallCfdCl()

// configuration of member thread_createSyscall of class SyscallManager
#define SyscallManager_thread_createSyscall_hh <.hh>
#define SyscallManager_thread_createSyscall_cc <.cc>
#define SyscallManager_thread_createSyscallCfdCl 
#define SyscallManager_thread_createSyscallCfdT *
#define HAS_SyscallManager_thread_createSyscallCfd 1
#define SyscallManager_thread_createSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_thread_createSyscallCfd \
private: \
    * thread_createSyscallCfd; \
public: \
    void setthread_createSyscall(* o) {thread_createSyscallCfd = o;} \
    * getthread_createSyscall() { return (*) thread_createSyscallCfd; }
#define NEW_SyscallManager_thread_createSyscallCfd SyscallManager_thread_createSyscallCfdCl()

// configuration of member thread_runSyscall of class SyscallManager
#define SyscallManager_thread_runSyscall_hh <.hh>
#define SyscallManager_thread_runSyscall_cc <.cc>
#define SyscallManager_thread_runSyscallCfdCl 
#define SyscallManager_thread_runSyscallCfdT *
#define HAS_SyscallManager_thread_runSyscallCfd 1
#define SyscallManager_thread_runSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_thread_runSyscallCfd \
private: \
    * thread_runSyscallCfd; \
public: \
    void setthread_runSyscall(* o) {thread_runSyscallCfd = o;} \
    * getthread_runSyscall() { return (*) thread_runSyscallCfd; }
#define NEW_SyscallManager_thread_runSyscallCfd SyscallManager_thread_runSyscallCfdCl()

// configuration of member fwriteSyscall of class SyscallManager
#define SyscallManager_fwriteSyscall_hh <.hh>
#define SyscallManager_fwriteSyscall_cc <.cc>
#define SyscallManager_fwriteSyscallCfdCl 
#define SyscallManager_fwriteSyscallCfdT *
#define HAS_SyscallManager_fwriteSyscallCfd 1
#define SyscallManager_fwriteSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_fwriteSyscallCfd \
private: \
    * fwriteSyscallCfd; \
public: \
    void setfwriteSyscall(* o) {fwriteSyscallCfd = o;} \
    * getfwriteSyscall() { return (*) fwriteSyscallCfd; }
#define NEW_SyscallManager_fwriteSyscallCfd SyscallManager_fwriteSyscallCfdCl()

// configuration of member freadSyscall of class SyscallManager
#define SyscallManager_freadSyscall_hh <.hh>
#define SyscallManager_freadSyscall_cc <.cc>
#define SyscallManager_freadSyscallCfdCl 
#define SyscallManager_freadSyscallCfdT *
#define HAS_SyscallManager_freadSyscallCfd 1
#define SyscallManager_freadSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_freadSyscallCfd \
private: \
    * freadSyscallCfd; \
public: \
    void setfreadSyscall(* o) {freadSyscallCfd = o;} \
    * getfreadSyscall() { return (*) freadSyscallCfd; }
#define NEW_SyscallManager_freadSyscallCfd SyscallManager_freadSyscallCfdCl()

// configuration of member fopenSyscall of class SyscallManager
#define SyscallManager_fopenSyscall_hh <.hh>
#define SyscallManager_fopenSyscall_cc <.cc>
#define SyscallManager_fopenSyscallCfdCl 
#define SyscallManager_fopenSyscallCfdT *
#define HAS_SyscallManager_fopenSyscallCfd 1
#define SyscallManager_fopenSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_fopenSyscallCfd \
private: \
    * fopenSyscallCfd; \
public: \
    void setfopenSyscall(* o) {fopenSyscallCfd = o;} \
    * getfopenSyscall() { return (*) fopenSyscallCfd; }
#define NEW_SyscallManager_fopenSyscallCfd SyscallManager_fopenSyscallCfdCl()

// configuration of member fcloseSyscall of class SyscallManager
#define SyscallManager_fcloseSyscall_hh <.hh>
#define SyscallManager_fcloseSyscall_cc <.cc>
#define SyscallManager_fcloseSyscallCfdCl 
#define SyscallManager_fcloseSyscallCfdT *
#define HAS_SyscallManager_fcloseSyscallCfd 1
#define SyscallManager_fcloseSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_fcloseSyscallCfd \
private: \
    * fcloseSyscallCfd; \
public: \
    void setfcloseSyscall(* o) {fcloseSyscallCfd = o;} \
    * getfcloseSyscall() { return (*) fcloseSyscallCfd; }
#define NEW_SyscallManager_fcloseSyscallCfd SyscallManager_fcloseSyscallCfdCl()

// configuration of member newSyscall of class SyscallManager
#define SyscallManager_newSyscall_hh <.hh>
#define SyscallManager_newSyscall_cc <.cc>
#define SyscallManager_newSyscallCfdCl 
#define SyscallManager_newSyscallCfdT *
#define HAS_SyscallManager_newSyscallCfd 1
#define SyscallManager_newSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_newSyscallCfd \
private: \
    * newSyscallCfd; \
public: \
    void setnewSyscall(* o) {newSyscallCfd = o;} \
    * getnewSyscall() { return (*) newSyscallCfd; }
#define NEW_SyscallManager_newSyscallCfd SyscallManager_newSyscallCfdCl()

// configuration of member task_stopSyscall of class SyscallManager
#define SyscallManager_task_stopSyscall_hh <.hh>
#define SyscallManager_task_stopSyscall_cc <.cc>
#define SyscallManager_task_stopSyscallCfdCl 
#define SyscallManager_task_stopSyscallCfdT *
#define DEF_SyscallManager_task_stopSyscallCfd \
public: \
    void settask_stopSyscall(* o) { } \
    * gettask_stopSyscall() { return 0; }

// configuration of member task_resumeSyscall of class SyscallManager
#define SyscallManager_task_resumeSyscall_hh <.hh>
#define SyscallManager_task_resumeSyscall_cc <.cc>
#define SyscallManager_task_resumeSyscallCfdCl 
#define SyscallManager_task_resumeSyscallCfdT *
#define DEF_SyscallManager_task_resumeSyscallCfd \
public: \
    void settask_resumeSyscall(* o) { } \
    * gettask_resumeSyscall() { return 0; }

// configuration of member deleteSyscall of class SyscallManager
#define SyscallManager_deleteSyscall_hh <.hh>
#define SyscallManager_deleteSyscall_cc <.cc>
#define SyscallManager_deleteSyscallCfdCl 
#define SyscallManager_deleteSyscallCfdT *
#define HAS_SyscallManager_deleteSyscallCfd 1
#define SyscallManager_deleteSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_deleteSyscallCfd \
private: \
    * deleteSyscallCfd; \
public: \
    void setdeleteSyscall(* o) {deleteSyscallCfd = o;} \
    * getdeleteSyscall() { return (*) deleteSyscallCfd; }
#define NEW_SyscallManager_deleteSyscallCfd SyscallManager_deleteSyscallCfdCl()

// configuration of member socketSyscall of class SyscallManager
#define SyscallManager_socketSyscall_hh <.hh>
#define SyscallManager_socketSyscall_cc <.cc>
#define SyscallManager_socketSyscallCfdCl 
#define SyscallManager_socketSyscallCfdT *
#define HAS_SyscallManager_socketSyscallCfd 1
#define SyscallManager_socketSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_socketSyscallCfd \
private: \
    * socketSyscallCfd; \
public: \
    void setsocketSyscall(* o) {socketSyscallCfd = o;} \
    * getsocketSyscall() { return (*) socketSyscallCfd; }
#define NEW_SyscallManager_socketSyscallCfd SyscallManager_socketSyscallCfdCl()

// configuration of member bindSyscall of class SyscallManager
#define SyscallManager_bindSyscall_hh <.hh>
#define SyscallManager_bindSyscall_cc <.cc>
#define SyscallManager_bindSyscallCfdCl 
#define SyscallManager_bindSyscallCfdT *
#define HAS_SyscallManager_bindSyscallCfd 1
#define SyscallManager_bindSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_bindSyscallCfd \
private: \
    * bindSyscallCfd; \
public: \
    void setbindSyscall(* o) {bindSyscallCfd = o;} \
    * getbindSyscall() { return (*) bindSyscallCfd; }
#define NEW_SyscallManager_bindSyscallCfd SyscallManager_bindSyscallCfdCl()

// configuration of member sendtoSyscall of class SyscallManager
#define SyscallManager_sendtoSyscall_hh <.hh>
#define SyscallManager_sendtoSyscall_cc <.cc>
#define SyscallManager_sendtoSyscallCfdCl 
#define SyscallManager_sendtoSyscallCfdT *
#define HAS_SyscallManager_sendtoSyscallCfd 1
#define SyscallManager_sendtoSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_sendtoSyscallCfd \
private: \
    * sendtoSyscallCfd; \
public: \
    void setsendtoSyscall(* o) {sendtoSyscallCfd = o;} \
    * getsendtoSyscall() { return (*) sendtoSyscallCfd; }
#define NEW_SyscallManager_sendtoSyscallCfd SyscallManager_sendtoSyscallCfdCl()

// configuration of member recvSyscall of class SyscallManager
#define SyscallManager_recvSyscall_hh <.hh>
#define SyscallManager_recvSyscall_cc <.cc>
#define SyscallManager_recvSyscallCfdCl 
#define SyscallManager_recvSyscallCfdT *
#define HAS_SyscallManager_recvSyscallCfd 1
#define SyscallManager_recvSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_recvSyscallCfd \
private: \
    * recvSyscallCfd; \
public: \
    void setrecvSyscall(* o) {recvSyscallCfd = o;} \
    * getrecvSyscall() { return (*) recvSyscallCfd; }
#define NEW_SyscallManager_recvSyscallCfd SyscallManager_recvSyscallCfdCl()

// configuration of member add_devaddrSyscall of class SyscallManager
#define SyscallManager_add_devaddrSyscall_hh <.hh>
#define SyscallManager_add_devaddrSyscall_cc <.cc>
#define SyscallManager_add_devaddrSyscallCfdCl 
#define SyscallManager_add_devaddrSyscallCfdT *
#define DEF_SyscallManager_add_devaddrSyscallCfd \
public: \
    void setadd_devaddrSyscall(* o) { } \
    * getadd_devaddrSyscall() { return 0; }

// configuration of member thread_selfSyscall of class SyscallManager
#define SyscallManager_thread_selfSyscall_hh <.hh>
#define SyscallManager_thread_selfSyscall_cc <.cc>
#define SyscallManager_thread_selfSyscallCfdCl 
#define SyscallManager_thread_selfSyscallCfdT *
#define HAS_SyscallManager_thread_selfSyscallCfd 1
#define SyscallManager_thread_selfSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_thread_selfSyscallCfd \
private: \
    * thread_selfSyscallCfd; \
public: \
    void setthread_selfSyscall(* o) {thread_selfSyscallCfd = o;} \
    * getthread_selfSyscall() { return (*) thread_selfSyscallCfd; }
#define NEW_SyscallManager_thread_selfSyscallCfd SyscallManager_thread_selfSyscallCfdCl()

// configuration of member thread_yieldSyscall of class SyscallManager
#define SyscallManager_thread_yieldSyscall_hh <.hh>
#define SyscallManager_thread_yieldSyscall_cc <.cc>
#define SyscallManager_thread_yieldSyscallCfdCl 
#define SyscallManager_thread_yieldSyscallCfdT *
#define HAS_SyscallManager_thread_yieldSyscallCfd 1
#define SyscallManager_thread_yieldSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_thread_yieldSyscallCfd \
private: \
    * thread_yieldSyscallCfd; \
public: \
    void setthread_yieldSyscall(* o) {thread_yieldSyscallCfd = o;} \
    * getthread_yieldSyscall() { return (*) thread_yieldSyscallCfd; }
#define NEW_SyscallManager_thread_yieldSyscallCfd SyscallManager_thread_yieldSyscallCfdCl()

// configuration of member signal_waitSyscall of class SyscallManager
#define SyscallManager_signal_waitSyscall_hh <.hh>
#define SyscallManager_signal_waitSyscall_cc <.cc>
#define SyscallManager_signal_waitSyscallCfdCl 
#define SyscallManager_signal_waitSyscallCfdT *
#define HAS_SyscallManager_signal_waitSyscallCfd 1
#define SyscallManager_signal_waitSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_signal_waitSyscallCfd \
private: \
    * signal_waitSyscallCfd; \
public: \
    void setsignal_waitSyscall(* o) {signal_waitSyscallCfd = o;} \
    * getsignal_waitSyscall() { return (*) signal_waitSyscallCfd; }
#define NEW_SyscallManager_signal_waitSyscallCfd SyscallManager_signal_waitSyscallCfdCl()

// configuration of member signal_signalSyscall of class SyscallManager
#define SyscallManager_signal_signalSyscall_hh <.hh>
#define SyscallManager_signal_signalSyscall_cc <.cc>
#define SyscallManager_signal_signalSyscallCfdCl 
#define SyscallManager_signal_signalSyscallCfdT *
#define HAS_SyscallManager_signal_signalSyscallCfd 1
#define SyscallManager_signal_signalSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_signal_signalSyscallCfd \
private: \
    * signal_signalSyscallCfd; \
public: \
    void setsignal_signalSyscall(* o) {signal_signalSyscallCfd = o;} \
    * getsignal_signalSyscall() { return (*) signal_signalSyscallCfd; }
#define NEW_SyscallManager_signal_signalSyscallCfd SyscallManager_signal_signalSyscallCfdCl()

// configuration of member connectSyscall of class SyscallManager
#define SyscallManager_connectSyscall_hh <.hh>
#define SyscallManager_connectSyscall_cc <.cc>
#define SyscallManager_connectSyscallCfdCl 
#define SyscallManager_connectSyscallCfdT *
#define HAS_SyscallManager_connectSyscallCfd 1
#define SyscallManager_connectSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_connectSyscallCfd \
private: \
    * connectSyscallCfd; \
public: \
    void setconnectSyscall(* o) {connectSyscallCfd = o;} \
    * getconnectSyscall() { return (*) connectSyscallCfd; }
#define NEW_SyscallManager_connectSyscallCfd SyscallManager_connectSyscallCfdCl()

// configuration of member fcreateSyscall of class SyscallManager
#define SyscallManager_fcreateSyscall_hh <.hh>
#define SyscallManager_fcreateSyscall_cc <.cc>
#define SyscallManager_fcreateSyscallCfdCl 
#define SyscallManager_fcreateSyscallCfdT *
#define HAS_SyscallManager_fcreateSyscallCfd 1
#define SyscallManager_fcreateSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_fcreateSyscallCfd \
private: \
    * fcreateSyscallCfd; \
public: \
    void setfcreateSyscall(* o) {fcreateSyscallCfd = o;} \
    * getfcreateSyscall() { return (*) fcreateSyscallCfd; }
#define NEW_SyscallManager_fcreateSyscallCfd SyscallManager_fcreateSyscallCfdCl()

#define ENABLE_NESTED_INTERRUPTS 0

#define HAS_PROCFS_ENABLED 0

#define USE_WORKERTASK 1

#define NUM_WORKERTHREADS 3

#define LOG_TASK_SPACE_START 0x400000

#define NUM_ADDRESS_PROTOCOLS 0

#define NUM_TRANSPORT_PROTOCOLS 0

#define USE_SIMPLEADDRESSPROTOCOL 0

#define USE_ARP 0

#define USE_SIMPLETRANSPORTPROTOCOL 0

#define MMIO_START_ADDRESS 0x90000000

#define MMIO_AREA_LENGTH 0xFFFFFF

#define ICACHE_ENABLE 1

#define DCACHE_ENABLE 1

#define DEFAULT_USER_STACK_SIZE 2048

#define STACK_GROWS_DOWNWARDS 1

#define CLEAR_THREAD_STACKS 1

#define RESERVED_BYTES_FOR_STACKFRAME 12

#define WORKERTHREAD_STACK_SIZE 2048

#define WORKERTHREAD_UART_PRIORITY_PARAM 50000

#define WORKERTHREAD_ETH_PRIORITY_PARAM 10000

#define USE_SAFE_KERNEL_STACKS 0

#define KERNEL_STACK_SIZE 1024

#define USE_PIP 0

#define ALIGN_VAL 4

#define __EARLY_SERIAL_SUPPORT__ 1

#define __DEBUG__ 1
