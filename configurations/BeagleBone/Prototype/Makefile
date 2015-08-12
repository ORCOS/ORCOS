SHELL:=/bin/bash
#Configuration Makefile

CWD = .
override RELATIVE_SOURCE_PATH = ../../..

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Tools Settings
#---------------------------------------------------------------------------------------------------------------------------------------
# For windows we must specify the ld linking directories with absolute path 
GCC_LIB_DIR	= D:/toolchains/armgcc-4-7-4/lib/gcc/arm-none-eabi/4.7.4/armv7-ar/thumb/
LIBC_DIR	= D:/toolchains/armgcc-4-7-4/arm-none-eabi/lib/armv7-ar/thumb/

# Path to the GCC binaries
ARMEABI	= /cygdrive/d/toolchains/armgcc-4-7-4/bin/arm-none-eabi

CC      = $(ARMEABI)-gcc
CXX     = $(ARMEABI)-c++
AS      = $(ARMEABI)-as
AR      = $(ARMEABI)-ar
SIZE	= $(ARMEABI)-size
OBJDUMP = $(ARMEABI)-objdump
LD		= $(ARMEABI)-ld
OBJCOPY = $(ARMEABI)-objcopy 
STRIP   = $(ARMEABI)-strip

#SH Tools
RM      = rm -rf
RN      = mv
CP      = cp
SHELL 	= sh
COPY 	= cp
SED 	= sed

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                       Directory Settings
#---------------------------------------------------------------------------------------------------------------------------------------

#This is where the kernel source can be found
export KERNEL_DIR     = $(CWD)/$(RELATIVE_SOURCE_PATH)/source/
export KERNEL_LIB_DIR = $(CWD)/$(RELATIVE_SOURCE_PATH)/library/bin/armv7/

#Output Directory. This is where the objects and the final binary go
export OUTPUT_DIR  = ./output/
export MODULES_DIR = ./modules/

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Compile/Link Settings
#---------------------------------------------------------------------------------------------------------------------------------------

# The Linker Script used for this configuration
LINKERSCRIPT = $(ARCH_DIR)/kernel.ld   

#add dwarf debugging support (increases binray size by multiple KB due to method string table beeing added)
export ADD_DWARF := 1

# -fno-if-conversion2 enforces to not use THUMB ITT blocks..
# it seams like they are not correctly executed after context restore ...
# although the PSR is correct .. (ITSTATE)

OPT_FLAGS = -O2 -mthumb -mcpu=cortex-a8  -DARM_THUMB=1 -fno-builtin -flto -fno-section-anchors -mno-unaligned-access
#-fno-if-conversion2
OPT_FLAGS += -mfpu=neon
# -mthumb-interwork

# -ffunction-sections 
#Command line arguments to compile .c files.
CFLAGS  = -Wall -g -I$(KERNEL_DIR)inc/ -I./make/ -I$(KERNEL_DIR)comm/lwip/include/ -I$(KERNEL_DIR)comm/lwip/ -I. -I$(KERNEL_DIR) -I$(KERNEL_LIB_DIR)/../../ $(ARCH_INCLUDES) 
CFLAGS += -fno-exceptions -fno-unwind-tables -fno-stack-protector -Wno-write-strings -c $(CPU_FLAGS)
#CFLAGS +=  -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wconversion

#Command line arguments to compile .cc files.
CPFLAGS  = -Wall -g -I$(KERNEL_DIR)inc/ -I./make/ -I$(KERNEL_DIR)comm/lwip/include/ -I$(KERNEL_DIR)comm/lwip/ -I. -I$(KERNEL_DIR) -I$(KERNEL_LIB_DIR)/../../ $(ARCH_INCLUDES) 
CPFLAGS += -fno-exceptions -fno-unwind-tables -fno-rtti -fno-stack-protector -c -Wno-write-strings -Wuninitialized -Woverloaded-virtual $(CPU_FLAGS)
#CPFLAGS +=  -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wconversion -msoft-float 

#Command line arguments to the linker.
LDFLAGS = -L$(LIBC_DIR) -L$(GCC_LIB_DIR) -Wl,-Map=output/kernel.map  -lgcc -Wl,--script=$(LINKERSCRIPT) -nostartfiles $(OPT_FLAGS)

#Command line arguments for the gcc to assemble .S files.
ASFLAGS = -c -g -I$(KERNEL_DIR)inc/ -I. -I./make/ -I$(KERNEL_DIR) $(ARCH_INCLUDES) -flto  -fno-exceptions -fno-rtti -fno-stack-protector -mcpu=cortex-a8 -mfpu=neon

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Makefile Inclusion
#---------------------------------------------------------------------------------------------------------------------------------------

#include kernel make file
-include $(KERNEL_DIR)/kernel.mk

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                     User Defined Make Targets
#---------------------------------------------------------------------------------------------------------------------------------------
	
