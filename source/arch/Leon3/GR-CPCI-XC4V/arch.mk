#
# arch.mk contains all architecture dependent makefile-settings, as kernel.mk must not contain any of them.
#

#ARCH_OBJ contains all architecture dependent object files to be compiled and linked into the kernel.
ARCH_OBJ += Leon3InterruptHandler.o handleInterrupt.o Leon3_GRETH.o
ARCH_OBJ += Leon3IntervalTimer.o LEON_UART.o startThread.o Leon3Processor.o Leon3Clock.o LED.o
ARCH_OBJ += Leon3_IRQMP.o LockedQueue.o ShmDriver.o 
ARCH_OBJ += Virtex4Board.o
ARCH_OBJ += assemblerFunctions.o Leon3HatLayer.o
ARCH_OBJ += windowHandler.o initTrapHandler.o Leon3InterruptHandlerHooks.o alignmentHandler.o
ARCH_OBJ += _startup.o AISMemManager.o FailureMonitor.o
 

#ARCH_VPATH defines architecture dependent VPATH-entries (see VPATH below).
ARCH_VPATH = $(KERNEL_DIR)arch/Leon3 $(KERNEL_DIR)arch/Leon3/ShmDriver $(KERNEL_DIR)arch/Leon3/GR-CPCI-XC4V

#Where to look for header files in the architecture directories.
ARCH_INCLUDES = -I$(KERNEL_DIR)arch/Leon3/GR-CPCI-XC4V/ -I$(KERNEL_DIR)arch/Leon3/ -I$(KERNEL_DIR)arch/Leon3/ShmDriver/