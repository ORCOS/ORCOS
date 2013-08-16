
// configuration of class Mutex

// configuration of member Scheduler of class Mutex
#define Mutex_Scheduler_hh <scheduler/RoundRobinThreadScheduler.hh>
#define Mutex_Scheduler_cc <scheduler/RoundRobinThreadScheduler.cc>
#define Mutex_SchedulerCfdCl RoundRobinThreadScheduler
#define Mutex_SchedulerCfdT RoundRobinThreadScheduler*
#define HAS_Mutex_SchedulerCfd 1
#define Mutex_Scheduler_IN_USERSPACE 0
#define DEF_Mutex_SchedulerCfd \
private: \
    RoundRobinThreadScheduler* SchedulerCfd; \
public: \
    void setScheduler(RoundRobinThreadScheduler* o) {SchedulerCfd = o;} \
    RoundRobinThreadScheduler* getScheduler() { return (RoundRobinThreadScheduler*) SchedulerCfd; }
#define NEW_Mutex_SchedulerCfd Mutex_SchedulerCfdCl()

// configuration of class Semaphore

// configuration of member Scheduler of class Semaphore
#define Semaphore_Scheduler_hh <scheduler/RoundRobinThreadScheduler.hh>
#define Semaphore_Scheduler_cc <scheduler/RoundRobinThreadScheduler.cc>
#define Semaphore_SchedulerCfdCl RoundRobinThreadScheduler
#define Semaphore_SchedulerCfdT RoundRobinThreadScheduler*
#define HAS_Semaphore_SchedulerCfd 1
#define Semaphore_Scheduler_IN_USERSPACE 0
#define DEF_Semaphore_SchedulerCfd \
private: \
    RoundRobinThreadScheduler* SchedulerCfd; \
public: \
    void setScheduler(RoundRobinThreadScheduler* o) {SchedulerCfd = o;} \
    RoundRobinThreadScheduler* getScheduler() { return (RoundRobinThreadScheduler*) SchedulerCfd; }
#define NEW_Semaphore_SchedulerCfd Semaphore_SchedulerCfdCl()

// configuration of class SingleCPUDispatcher

// configuration of member Scheduler of class SingleCPUDispatcher
#define SingleCPUDispatcher_Scheduler_hh <scheduler/EarliestDeadlineFirstThreadScheduler.hh>
#define SingleCPUDispatcher_Scheduler_cc <scheduler/EarliestDeadlineFirstThreadScheduler.cc>
#define SingleCPUDispatcher_SchedulerCfdCl EarliestDeadlineFirstThreadScheduler
#define SingleCPUDispatcher_SchedulerCfdT EarliestDeadlineFirstThreadScheduler*
#define HAS_SingleCPUDispatcher_SchedulerCfd 1
#define SingleCPUDispatcher_Scheduler_IN_USERSPACE 0
#define DEF_SingleCPUDispatcher_SchedulerCfd \
private: \
    EarliestDeadlineFirstThreadScheduler* SchedulerCfd; \
public: \
    void setScheduler(EarliestDeadlineFirstThreadScheduler* o) {SchedulerCfd = o;} \
    EarliestDeadlineFirstThreadScheduler* getScheduler() { return (EarliestDeadlineFirstThreadScheduler*) SchedulerCfd; }
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
#define MemoryManager_HatLayer_hh <arch/ARM/ARMv4T/ARMv7/ARMv7HatLayer.hh>
#define MemoryManager_HatLayer_cc <arch/ARM/ARMv4T/ARMv7/ARMv7HatLayer.cc>
#define MemoryManager_HatLayerCfdCl ARMv7HatLayer
#define MemoryManager_HatLayerCfdT ARMv7HatLayer*
#define HAS_MemoryManager_HatLayerCfd 1
#define MemoryManager_HatLayer_IN_USERSPACE 0
#define DEF_MemoryManager_HatLayerCfd \
private: \
    ARMv7HatLayer* HatLayerCfd; \
public: \
    void setHatLayer(ARMv7HatLayer* o) {HatLayerCfd = o;} \
    ARMv7HatLayer* getHatLayer() { return (ARMv7HatLayer*) HatLayerCfd; }
#define NEW_MemoryManager_HatLayerCfd MemoryManager_HatLayerCfdCl()

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
#define BoardCfd_hh <arch/ARM/ARMv4T/ARMv7/Omap3530/BeagleBoardxM/BeagleBoardxM.hh>
#define BoardCfdCl BeagleBoardxM

// configuration of member Processor of class Board
#define Board_Processor_hh <arch/ARM/ARMv4T/ARMv4TProcessor.hh>
#define Board_Processor_cc <arch/ARM/ARMv4T/ARMv4TProcessor.cc>
#define Board_ProcessorCfdCl ARMv4TProcessor
#define Board_ProcessorCfdT ARMv4TProcessor*
#define HAS_Board_ProcessorCfd 1
#define Board_Processor_IN_USERSPACE 0
#define DEF_Board_ProcessorCfd \
private: \
    ARMv4TProcessor* ProcessorCfd; \
public: \
    void setProcessor(ARMv4TProcessor* o) {ProcessorCfd = o;} \
    ARMv4TProcessor* getProcessor() { return (ARMv4TProcessor*) ProcessorCfd; }
#define NEW_Board_ProcessorCfd Board_ProcessorCfdCl()

// configuration of member Timer of class Board
#define Board_Timer_hh <arch/ARM/ARMv4T/ARMv7/Omap3530/BeagleBoardGPTimer2.hh>
#define Board_Timer_cc <arch/ARM/ARMv4T/ARMv7/Omap3530/BeagleBoardGPTimer2.cc>
#define Board_TimerCfdCl BeagleBoardGPTimer2
#define Board_TimerCfdT BeagleBoardGPTimer2*
#define HAS_Board_TimerCfd 1
#define Board_Timer_IN_USERSPACE 0
#define DEF_Board_TimerCfd \
private: \
    BeagleBoardGPTimer2* TimerCfd; \
public: \
    void setTimer(BeagleBoardGPTimer2* o) {TimerCfd = o;} \
    BeagleBoardGPTimer2* getTimer() { return (BeagleBoardGPTimer2*) TimerCfd; }
#define NEW_Board_TimerCfd Board_TimerCfdCl()

// configuration of member Clock of class Board
#define Board_Clock_hh <arch/ARM/ARMv4T/ARMv7/Omap3530/Omap3530Clock.hh>
#define Board_Clock_cc <arch/ARM/ARMv4T/ARMv7/Omap3530/Omap3530Clock.cc>
#define Board_ClockCfdCl Omap3530Clock
#define Board_ClockCfdT Omap3530Clock*
#define HAS_Board_ClockCfd 1
#define Board_Clock_IN_USERSPACE 0
#define Board_Clock_NAME "clock"
#define DEF_Board_ClockCfd \
private: \
    Omap3530Clock* ClockCfd; \
public: \
    void setClock(Omap3530Clock* o) {ClockCfd = o;} \
    Omap3530Clock* getClock() { return (Omap3530Clock*) ClockCfd; }
#define NEW_Board_ClockCfd Board_ClockCfdCl("clock")

// configuration of member Watchdog of class Board
#define Board_Watchdog_hh <arch/none/DummyWatchdog.hh>
#define Board_Watchdog_cc <arch/none/DummyWatchdog.cc>
#define Board_WatchdogCfdCl DummyWatchdog
#define Board_WatchdogCfdT DummyWatchdog*
#define Board_Watchdog_NAME "watchdog"
#define DEF_Board_WatchdogCfd \
public: \
    void setWatchdog(DummyWatchdog* o) { } \
    DummyWatchdog* getWatchdog() { return 0; }

// configuration of member UART of class Board
#define Board_UART_hh <arch/ARM/ARMv4T/ARMv7/Omap3530/BeagleBoardUART.hh>
#define Board_UART_cc <arch/ARM/ARMv4T/ARMv7/Omap3530/BeagleBoardUART.cc>
#define Board_UARTCfdCl BeagleBoardUART
#define Board_UARTCfdT BeagleBoardUART*
#define HAS_Board_UARTCfd 1
#define Board_UART_IN_USERSPACE 0
#define Board_UART_NAME "serial0"
#define Board_UART_MMIO_PHYS_ADDR 0x49020000
#define Board_UART_MMIO_LENGTH (unint4)1024
#define DEF_Board_UARTCfd \
private: \
    BeagleBoardUART* UARTCfd; \
public: \
    void setUART(BeagleBoardUART* o) {UARTCfd = o;} \
    BeagleBoardUART* getUART() { return (BeagleBoardUART*) UARTCfd; }
#define NEW_Board_UARTCfd Board_UARTCfdCl("serial0",0x49020000)

// configuration of member LED of class Board
#define Board_LED_hh <arch/none/DummyCharDriver.hh>
#define Board_LED_cc <arch/none/DummyCharDriver.cc>
#define Board_LEDCfdCl DummyCharDriver
#define Board_LEDCfdT DummyCharDriver*
#define Board_LED_NAME "led0"
#define Board_LED_MMIO_PHYS_ADDR 0
#define DEF_Board_LEDCfd \
public: \
    void setLED(DummyCharDriver* o) { } \
    DummyCharDriver* getLED() { return 0; }

// configuration of member InterruptHandler of class Board
#define Board_InterruptHandler_hh <arch/ARM/ARMv4T/ARMv4TInterruptHandler.hh>
#define Board_InterruptHandler_cc <arch/ARM/ARMv4T/ARMv4TInterruptHandler.cc>
#define Board_InterruptHandlerCfdCl ARMv4TInterruptHandler
#define Board_InterruptHandlerCfdT ARMv4TInterruptHandler*
#define HAS_Board_InterruptHandlerCfd 1
#define Board_InterruptHandler_IN_USERSPACE 0
#define DEF_Board_InterruptHandlerCfd \
private: \
    ARMv4TInterruptHandler* InterruptHandlerCfd; \
public: \
    void setInterruptHandler(ARMv4TInterruptHandler* o) {InterruptHandlerCfd = o;} \
    ARMv4TInterruptHandler* getInterruptHandler() { return (ARMv4TInterruptHandler*) InterruptHandlerCfd; }
#define NEW_Board_InterruptHandlerCfd Board_InterruptHandlerCfdCl()

// configuration of member InterruptController of class Board
#define Board_InterruptController_hh <arch/ARM/ARMv4T/ARMv7/Omap3530/BeagleBoardInterruptController.hh>
#define Board_InterruptController_cc <arch/ARM/ARMv4T/ARMv7/Omap3530/BeagleBoardInterruptController.cc>
#define Board_InterruptControllerCfdCl BeagleBoardInterruptController
#define Board_InterruptControllerCfdT BeagleBoardInterruptController*
#define HAS_Board_InterruptControllerCfd 1
#define Board_InterruptController_IN_USERSPACE 0
#define DEF_Board_InterruptControllerCfd \
private: \
    BeagleBoardInterruptController* InterruptControllerCfd; \
public: \
    void setInterruptController(BeagleBoardInterruptController* o) {InterruptControllerCfd = o;} \
    BeagleBoardInterruptController* getInterruptController() { return (BeagleBoardInterruptController*) InterruptControllerCfd; }
#define NEW_Board_InterruptControllerCfd Board_InterruptControllerCfdCl()

// configuration of member ETH of class Board
#define Board_ETH_hh <arch/none/DummyCommDriver.hh>
#define Board_ETH_cc <arch/none/DummyCommDriver.cc>
#define Board_ETHCfdCl DummyCommDriver
#define Board_ETHCfdT DummyCommDriver*
#define Board_ETH_NAME "eth0"
#define Board_ETH_MMIO_PHYS_ADDR 0x90000000
#define Board_ETH_MMIO_LENGTH (unint4)1024*1024
#define Board_ETH_UNIQUEID 0x0,0x1,0x2,0x3,0x4
#define DEF_Board_ETHCfd \
public: \
    void setETH(DummyCommDriver* o) { } \
    DummyCommDriver* getETH() { return 0; }

// configuration of member SHM of class Board
#define Board_SHM_hh <arch/none/DummyCommDriver.hh>
#define Board_SHM_cc <arch/none/DummyCommDriver.cc>
#define Board_SHMCfdCl DummyCommDriver
#define Board_SHMCfdT DummyCommDriver*
#define DEF_Board_SHMCfd \
public: \
    void setSHM(DummyCommDriver* o) { } \
    DummyCommDriver* getSHM() { return 0; }

// configuration of member UART2 of class Board
#define Board_UART2_hh <arch/ARM/ARMv4T/ARMv7/Omap3530/Omap3530i2c.hh>
#define Board_UART2_cc <arch/ARM/ARMv4T/ARMv7/Omap3530/Omap3530i2c.cc>
#define Board_UART2CfdCl Omap3530i2c
#define Board_UART2CfdT Omap3530i2c*
#define HAS_Board_UART2Cfd 1
#define Board_UART2_IN_USERSPACE 0
#define Board_UART2_NAME "i2c"
#define Board_UART2_MMIO_PHYS_ADDR 0x48070000
#define DEF_Board_UART2Cfd \
private: \
    Omap3530i2c* UART2Cfd; \
public: \
    void setUART2(Omap3530i2c* o) {UART2Cfd = o;} \
    Omap3530i2c* getUART2() { return (Omap3530i2c*) UART2Cfd; }
#define NEW_Board_UART2Cfd Board_UART2CfdCl("i2c",0x48070000)

// configuration of member HCI of class Board
#define Board_HCI_hh <arch/none/DummyCharDriver.hh>
#define Board_HCI_cc <arch/none/DummyCharDriver.cc>
#define Board_HCICfdCl DummyCharDriver
#define Board_HCICfdT DummyCharDriver*
#define DEF_Board_HCICfd \
public: \
    void setHCI(DummyCharDriver* o) { } \
    DummyCharDriver* getHCI() { return 0; }

// configuration of member USB_HC of class Board
#define Board_USB_HC_hh <arch/ARM/ARMv4T/ARMv7/Omap3530/HighSpeedUSBHostController.hh>
#define Board_USB_HC_cc <arch/ARM/ARMv4T/ARMv7/Omap3530/HighSpeedUSBHostController.cc>
#define Board_USB_HCCfdCl HighSpeedUSBHostController
#define Board_USB_HCCfdT HighSpeedUSBHostController*
#define HAS_Board_USB_HCCfd 1
#define Board_USB_HC_IN_USERSPACE 0
#define DEF_Board_USB_HCCfd \
private: \
    HighSpeedUSBHostController* USB_HCCfd; \
public: \
    void setUSB_HC(HighSpeedUSBHostController* o) {USB_HCCfd = o;} \
    HighSpeedUSBHostController* getUSB_HC() { return (HighSpeedUSBHostController*) USB_HCCfd; }
#define NEW_Board_USB_HCCfd Board_USB_HCCfdCl()

// configuration of member GPIO1 of class Board
#define Board_GPIO1_hh <arch/ARM/ARMv4T/ARMv7/Omap3530/OmapGPIO.hh>
#define Board_GPIO1_cc <arch/ARM/ARMv4T/ARMv7/Omap3530/OmapGPIO.cc>
#define Board_GPIO1CfdCl OmapGPIO
#define Board_GPIO1CfdT OmapGPIO*
#define HAS_Board_GPIO1Cfd 1
#define Board_GPIO1_IN_USERSPACE 0
#define Board_GPIO1_NAME "gpio1"
#define Board_GPIO1_MMIO_PHYS_ADDR 0x48310000
#define DEF_Board_GPIO1Cfd \
private: \
    OmapGPIO* GPIO1Cfd; \
public: \
    void setGPIO1(OmapGPIO* o) {GPIO1Cfd = o;} \
    OmapGPIO* getGPIO1() { return (OmapGPIO*) GPIO1Cfd; }
#define NEW_Board_GPIO1Cfd Board_GPIO1CfdCl("gpio1",0x48310000)

// configuration of member GPIO2 of class Board
#define Board_GPIO2_hh <arch/ARM/ARMv4T/ARMv7/Omap3530/OmapGPIO.hh>
#define Board_GPIO2_cc <arch/ARM/ARMv4T/ARMv7/Omap3530/OmapGPIO.cc>
#define Board_GPIO2CfdCl OmapGPIO
#define Board_GPIO2CfdT OmapGPIO*
#define HAS_Board_GPIO2Cfd 1
#define Board_GPIO2_IN_USERSPACE 0
#define Board_GPIO2_NAME "gpio2"
#define Board_GPIO2_MMIO_PHYS_ADDR 0x49050000
#define DEF_Board_GPIO2Cfd \
private: \
    OmapGPIO* GPIO2Cfd; \
public: \
    void setGPIO2(OmapGPIO* o) {GPIO2Cfd = o;} \
    OmapGPIO* getGPIO2() { return (OmapGPIO*) GPIO2Cfd; }
#define NEW_Board_GPIO2Cfd Board_GPIO2CfdCl("gpio2",0x49050000)

// configuration of member GPIO3 of class Board
#define Board_GPIO3_hh <arch/ARM/ARMv4T/ARMv7/Omap3530/OmapGPIO.hh>
#define Board_GPIO3_cc <arch/ARM/ARMv4T/ARMv7/Omap3530/OmapGPIO.cc>
#define Board_GPIO3CfdCl OmapGPIO
#define Board_GPIO3CfdT OmapGPIO*
#define HAS_Board_GPIO3Cfd 1
#define Board_GPIO3_IN_USERSPACE 0
#define Board_GPIO3_NAME "gpio3"
#define Board_GPIO3_MMIO_PHYS_ADDR 0x49052000
#define DEF_Board_GPIO3Cfd \
private: \
    OmapGPIO* GPIO3Cfd; \
public: \
    void setGPIO3(OmapGPIO* o) {GPIO3Cfd = o;} \
    OmapGPIO* getGPIO3() { return (OmapGPIO*) GPIO3Cfd; }
#define NEW_Board_GPIO3Cfd Board_GPIO3CfdCl("gpio3",0x49052000)

// configuration of member GPIO4 of class Board
#define Board_GPIO4_hh <arch/ARM/ARMv4T/ARMv7/Omap3530/OmapGPIO.hh>
#define Board_GPIO4_cc <arch/ARM/ARMv4T/ARMv7/Omap3530/OmapGPIO.cc>
#define Board_GPIO4CfdCl OmapGPIO
#define Board_GPIO4CfdT OmapGPIO*
#define HAS_Board_GPIO4Cfd 1
#define Board_GPIO4_IN_USERSPACE 0
#define Board_GPIO4_NAME "gpio4"
#define Board_GPIO4_MMIO_PHYS_ADDR 0x49054000
#define DEF_Board_GPIO4Cfd \
private: \
    OmapGPIO* GPIO4Cfd; \
public: \
    void setGPIO4(OmapGPIO* o) {GPIO4Cfd = o;} \
    OmapGPIO* getGPIO4() { return (OmapGPIO*) GPIO4Cfd; }
#define NEW_Board_GPIO4Cfd Board_GPIO4CfdCl("gpio4",0x49054000)

// configuration of member GPIO5 of class Board
#define Board_GPIO5_hh <arch/ARM/ARMv4T/ARMv7/Omap3530/OmapGPIO.hh>
#define Board_GPIO5_cc <arch/ARM/ARMv4T/ARMv7/Omap3530/OmapGPIO.cc>
#define Board_GPIO5CfdCl OmapGPIO
#define Board_GPIO5CfdT OmapGPIO*
#define HAS_Board_GPIO5Cfd 1
#define Board_GPIO5_IN_USERSPACE 0
#define Board_GPIO5_NAME "gpio5"
#define Board_GPIO5_MMIO_PHYS_ADDR 0x49056000
#define DEF_Board_GPIO5Cfd \
private: \
    OmapGPIO* GPIO5Cfd; \
public: \
    void setGPIO5(OmapGPIO* o) {GPIO5Cfd = o;} \
    OmapGPIO* getGPIO5() { return (OmapGPIO*) GPIO5Cfd; }
#define NEW_Board_GPIO5Cfd Board_GPIO5CfdCl("gpio5",0x49056000)

// configuration of member GPIO6 of class Board
#define Board_GPIO6_hh <arch/ARM/ARMv4T/ARMv7/Omap3530/OmapGPIO.hh>
#define Board_GPIO6_cc <arch/ARM/ARMv4T/ARMv7/Omap3530/OmapGPIO.cc>
#define Board_GPIO6CfdCl OmapGPIO
#define Board_GPIO6CfdT OmapGPIO*
#define HAS_Board_GPIO6Cfd 1
#define Board_GPIO6_IN_USERSPACE 0
#define Board_GPIO6_NAME "gpio6"
#define Board_GPIO6_MMIO_PHYS_ADDR 0x49058000
#define DEF_Board_GPIO6Cfd \
private: \
    OmapGPIO* GPIO6Cfd; \
public: \
    void setGPIO6(OmapGPIO* o) {GPIO6Cfd = o;} \
    OmapGPIO* getGPIO6() { return (OmapGPIO*) GPIO6Cfd; }
#define NEW_Board_GPIO6Cfd Board_GPIO6CfdCl("gpio6",0x49058000)

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
#define HAS_Kernel_TaskManagerCfd 1
#define Kernel_TaskManager_IN_USERSPACE 0
#define DEF_Kernel_TaskManagerCfd \
private: \
    TaskManager* TaskManagerCfd; \
public: \
    void setTaskManager(TaskManager* o) {TaskManagerCfd = o;} \
    TaskManager* getTaskManager() { return (TaskManager*) TaskManagerCfd; }
#define NEW_Kernel_TaskManagerCfd Kernel_TaskManagerCfdCl()

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
#define HAS_SyscallManager_task_stopSyscallCfd 1
#define SyscallManager_task_stopSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_task_stopSyscallCfd \
private: \
    * task_stopSyscallCfd; \
public: \
    void settask_stopSyscall(* o) {task_stopSyscallCfd = o;} \
    * gettask_stopSyscall() { return (*) task_stopSyscallCfd; }
#define NEW_SyscallManager_task_stopSyscallCfd SyscallManager_task_stopSyscallCfdCl()

// configuration of member task_resumeSyscall of class SyscallManager
#define SyscallManager_task_resumeSyscall_hh <.hh>
#define SyscallManager_task_resumeSyscall_cc <.cc>
#define SyscallManager_task_resumeSyscallCfdCl 
#define SyscallManager_task_resumeSyscallCfdT *
#define HAS_SyscallManager_task_resumeSyscallCfd 1
#define SyscallManager_task_resumeSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_task_resumeSyscallCfd \
private: \
    * task_resumeSyscallCfd; \
public: \
    void settask_resumeSyscall(* o) {task_resumeSyscallCfd = o;} \
    * gettask_resumeSyscall() { return (*) task_resumeSyscallCfd; }
#define NEW_SyscallManager_task_resumeSyscallCfd SyscallManager_task_resumeSyscallCfdCl()

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
#define DEF_SyscallManager_signal_waitSyscallCfd \
public: \
    void setsignal_waitSyscall(* o) { } \
    * getsignal_waitSyscall() { return 0; }

// configuration of member signal_signalSyscall of class SyscallManager
#define SyscallManager_signal_signalSyscall_hh <.hh>
#define SyscallManager_signal_signalSyscall_cc <.cc>
#define SyscallManager_signal_signalSyscallCfdCl 
#define SyscallManager_signal_signalSyscallCfdT *
#define DEF_SyscallManager_signal_signalSyscallCfd \
public: \
    void setsignal_signalSyscall(* o) { } \
    * getsignal_signalSyscall() { return 0; }

// configuration of member fcreateSyscall of class SyscallManager
#define SyscallManager_fcreateSyscall_hh <.hh>
#define SyscallManager_fcreateSyscall_cc <.cc>
#define SyscallManager_fcreateSyscallCfdCl 
#define SyscallManager_fcreateSyscallCfdT *
#define DEF_SyscallManager_fcreateSyscallCfd \
public: \
    void setfcreateSyscall(* o) { } \
    * getfcreateSyscall() { return 0; }

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

// configuration of member listenSyscall of class SyscallManager
#define SyscallManager_listenSyscall_hh <.hh>
#define SyscallManager_listenSyscall_cc <.cc>
#define SyscallManager_listenSyscallCfdCl 
#define SyscallManager_listenSyscallCfdT *
#define HAS_SyscallManager_listenSyscallCfd 1
#define SyscallManager_listenSyscall_IN_USERSPACE 0
#define DEF_SyscallManager_listenSyscallCfd \
private: \
    * listenSyscallCfd; \
public: \
    void setlistenSyscall(* o) {listenSyscallCfd = o;} \
    * getlistenSyscall() { return (*) listenSyscallCfd; }
#define NEW_SyscallManager_listenSyscallCfd SyscallManager_listenSyscallCfdCl()

#define ENABLE_NESTED_INTERRUPTS 0

#define HAS_PROCFS_ENABLED 1

#define USE_WORKERTASK 1

#define NUM_WORKERTHREADS 2

#define LOG_TASK_SPACE_START 0x100000

#define NUM_ADDRESS_PROTOCOLS 0

#define NUM_TRANSPORT_PROTOCOLS 0

#define USE_SIMPLEADDRESSPROTOCOL 0

#define USE_ARP 0

#define USE_SIMPLETRANSPORTPROTOCOL 0

#define MMIO_START_ADDRESS 0x48000000

#define MMIO_AREA_LENGTH 0xFFFFFF

#define ICACHE_ENABLE 1

#define DCACHE_ENABLE 1

#define DEFAULT_USER_STACK_SIZE 4096

#define STACK_GROWS_DOWNWARDS 1

#define RESERVED_BYTES_FOR_STACKFRAME 12

#define CLEAR_THREAD_STACKS 1

#define WORKERTHREAD_STACK_SIZE 4096

#define WORKERTHREAD_UART_PRIORITY_PARAM 5000

#define WORKERTHREAD_ETH_PRIORITY_PARAM 2000

#define USE_SAFE_KERNEL_STACKS 0

#define KERNEL_STACK_SIZE 4096

#define ALIGN_VAL 4

#define __EARLY_SERIAL_SUPPORT__ 1

#define __DEBUG__ 0

#define PLATFORM_ARM 1
