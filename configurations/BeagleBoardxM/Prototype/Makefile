SHELL:=/bin/bash
#Configuration Makefile

#CWD = $(shell pwd)
CWD = .
RELATIVE_SOURCE_PATH = ../../..

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Tools Settings
#---------------------------------------------------------------------------------------------------------------------------------------
#GCC SETTINGS
# for windows we must specify the ld linking directories with absolute path 
GCC_LIB_DIR=D:/toolchains/armgcc-4-7-4/lib/gcc/arm-none-eabi/4.7.4/armv7-ar/thumb/
LIBC_DIR=D:/toolchains/armgcc-4-7-4/arm-none-eabi/lib/armv7-ar/thumb/

ARMEABI= /cygdrive/d/toolchains/armgcc-4-7-4/bin/arm-none-eabi

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
RM      	= rm -rf
RN      	= mv
CP      	= cp
SHELL 		= sh
COPY 		= cp
SED 		= sed

#SCL SETTINGS
SCL 	= java -jar $(RELATIVE_SOURCE_PATH)/tools/SCL/dist/scl2.jar SCLConfig.xml  $(RELATIVE_SOURCE_PATH)/tools/SCL/dist/scl2.xsd --check-deps $(RELATIVE_SOURCE_PATH)/source/SCLdependencies.xml  $(RELATIVE_SOURCE_PATH)/tools/SCL/dist/scl2deps.xsd --verbose=3 

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                       Directory Settings
#---------------------------------------------------------------------------------------------------------------------------------------

#This is where the kernel source can be found
KERNEL_DIR = $(CWD)/$(RELATIVE_SOURCE_PATH)/source/
KERNEL_LIB_DIR = $(CWD)/$(RELATIVE_SOURCE_PATH)/library/bin/armv7/


#Output Directory. This is where the objects and the final binary go
OUTPUT_DIR = ./output/
MODULES_DIR = ./modules/

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Compile/Link Settings
#---------------------------------------------------------------------------------------------------------------------------------------

# The Linker Script used for this configuration
LINKERSCRIPT = $(ARCH_DIR)/kernel.ld   

#Optimization flag -flto
OPT_FLAGS = -O2 -mthumb -mcpu=cortex-a8  -mthumb-interwork -DARM_THUMB=1 -fno-builtin -flto -mno-unaligned-access -fno-section-anchors

#Command line arguments to compile .c files.
CFLAGS = -Wall -g -I$(KERNEL_DIR)inc/ -I./make/ -I$(KERNEL_DIR)comm/lwip/include/ -I$(KERNEL_DIR)comm/lwip/ -I. -I$(KERNEL_DIR) -I$(KERNEL_LIB_DIR)/../../ $(ARCH_INCLUDES) -ffunction-sections -fno-exceptions -fno-unwind-tables -msoft-float -fno-stack-protector -Wno-write-strings -c $(CPU_FLAGS) 
#CFLAGS +=  -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wconversion

#Command line arguments to compile .cc files.
CPFLAGS = -Wall -g -I$(KERNEL_DIR)inc/ -I./make/ -I$(KERNEL_DIR)comm/lwip/include/ -I$(KERNEL_DIR)comm/lwip/ -I. -I$(KERNEL_DIR) -I$(KERNEL_LIB_DIR)/../../ $(ARCH_INCLUDES) -ffunction-sections  -fno-exceptions -fno-unwind-tables -fno-rtti -msoft-float -fno-stack-protector -c -Wno-write-strings -Wuninitialized -Woverloaded-virtual $(CPU_FLAGS) 
#CPFLAGS +=  -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wconversion

#Command line arguments to the linker.
LDFLAGS = -o output/kernel.elf -L$(LIBC_DIR) -L$(GCC_LIB_DIR) -Map output/kernel.map  -lgcc   --script=$(LINKERSCRIPT)

#Command line arguments for the gcc to assemble .S files.
ASFLAGS = -c -g -I$(KERNEL_DIR)inc/ -I. -I./make/ -I$(KERNEL_DIR) $(ARCH_INCLUDES) -flto  -fno-exceptions -fno-rtti -msoft-float -fno-stack-protector -mcpu=cortex-a8

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Makefile Inclusion
#---------------------------------------------------------------------------------------------------------------------------------------

#STACK_USAGE_SCRIPT = $(KERNEL_DIR)../tools/PPCStackUsage/stack_usage.py

#include architekture make file
-include $(ARCH_DIR)/arch.mk

#include kernel make file
-include $(KERNEL_DIR)/kernel.mk

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                     User Defined Make Targets
#---------------------------------------------------------------------------------------------------------------------------------------
	
	
#Only needed if xmd or bdi are used
XMD_DIR = ./xmd/
BDI_DIR = ./tftpboot/

BDI_CONFIG = omap35xx.cfg
BDI_REGDEF = regOMAP3500.def