#
# arch.mk contains all architecture dependent makefile-settings, as kernel.mk must not contain any of them.
#

#ARCH_OBJ contains all architecture dependent object files to be compiled and linked into the kernel.
ARCH_OBJ += Omap3530_startup.o
ARCH_OBJ += Omap3530Clock.o  
ARCH_OBJ += ARMv4TInterruptHandler.o 
ARCH_OBJ += BeagleBoard.o 
ARCH_OBJ += handleInterrupt.o
ARCH_OBJ += assemblerFunctions.o ARMv4TInterruptHandlerHooks.o
ARCH_OBJ += startThread.o
ARCH_OBJ += ARMv7HatLayer.o


#ARCH_VPATH defines architecture dependent VPATH-entries (see VPATH below).
ARCH_VPATH = $(KERNEL_DIR)arch/ARM/ARMv4T $(KERNEL_DIR)arch/ARM/ARMv4T/ARMv7 $(KERNEL_DIR)arch/ARM/ARMv4T/ARMv7/Omap3530 $(KERNEL_DIR)arch/ARM/ARMv4T/ARMv7/Omap3530/BeagleBoard 

#Where to look for header files in the architecture directories.
ARCH_INCLUDES = -I$(KERNEL_DIR)arch/ARM/ARMv4T/ -I$(KERNEL_DIR)arch/ARM/ARMv4T/ARMv7 -I$(KERNEL_DIR)arch/ARM/ARMv4T/ARMv7/Omap3530 -I$(KERNEL_DIR)arch/ARM/ARMv4T/ARMv7/Omap3530/BeagleBoard/

UIMAGE_ARCH	= arm
TARGET  	= armv7

CPU_FLAGS   = -mcpu=cortex-a8 -mfpu=neon -mthumb -DARM_THUMB=1 -mno-unaligned-access