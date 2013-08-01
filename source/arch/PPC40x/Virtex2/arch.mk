#
# arch.mk contains all architecture dependent makefile-settings, as kernel.mk must not contain any of them.
#

#ARCH_OBJ contains all architecture dependent object files to be compiled and linked into the kernel.
ARCH_OBJ += PPC405Watchdog.o PPC405InterruptHandler.o handleInterrupt.o PPC405ProgrammableIntervalTimer.o
ARCH_OBJ += PPC405FixedIntervalTimer.o SegmentDisplay.o LED.o  OPB_UART_Lite.o startThread.o PPC405FXProcessor.o PPC405Clock.o
ARCH_OBJ += Virtex2Board.o
ARCH_OBJ += PPC405HatLayer.o PPC405TlbEntry.o
ARCH_OBJ += assemblerFunctions.o
ARCH_OBJ += _startup.o PPC405InterruptHandlerHooks.o

#ARCH_VPATH defines architecture dependent VPATH-entries (see VPATH below).
ARCH_VPATH = $(KERNEL_DIR)arch/PPC40x $(KERNEL_DIR)arch/PPC40x/Virtex2

#Where to look for header files in the architecture directories.
ARCH_INCLUDES = -I$(KERNEL_DIR)arch/PPC40x/Virtex2/ -I$(KERNEL_DIR)arch/PPC40x/