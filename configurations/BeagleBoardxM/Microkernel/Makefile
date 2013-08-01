SHELL:=/bin/bash
#Configuration Makefile

#CWD = $(shell pwd)
CWD = .
RELATIVE_SOURCE_PATH = ../../..


#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Tools Settings
#---------------------------------------------------------------------------------------------------------------------------------------
#GCC
ARM_GCC_DIR=arm-2009q3
GCC_VERSION=4.7.2
# for windows we must specify the ld directory 
GCC_LIB_DIR=C:/yagarto-20121222/lib/gcc/arm-none-eabi/$(GCC_VERSION)/thumb/
LIBC_DIR=C:/yagarto-20121222/arm-none-eabi/lib/thumb/

CC      = /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-gcc
CXX     = /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-c++
AS      = /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-as
AR      = /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-ar
SIZE	= /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-size
OBJDUMP = /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-objdump
LD		= /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-ld
OBJCOPY = /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-objcopy 
STRIP   = /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-strip

#SH Tools
RM      	= rm -rf
RN      	= mv
CP      	= cp
SHELL 		= sh
COPY 		= cp
SED 		= sed

#SCL SETTINGS
#SCL 	= java -jar $(RELATIVE_SOURCE_PATH)/SCL/dist/scl2.jar SCLConfig.xml  $(RELATIVE_SOURCE_PATH)/SCL/dist/scl2.xsd --check-deps $(RELATIVE_SOURCE_PATH)/source/SCLdependencies.xml  $(RELATIVE_SOURCE_PATH)/SCL/dist/scl2deps.xsd 
SCL 	= java -jar $(RELATIVE_SOURCE_PATH)/tools/SCL/dist/scl2.jar SCLConfig.xml  $(RELATIVE_SOURCE_PATH)/tools/SCL/dist/scl2.xsd --check-deps $(RELATIVE_SOURCE_PATH)/source/SCLdependencies.xml  $(RELATIVE_SOURCE_PATH)/tools/SCL/dist/scl2deps.xsd 

#Initial Microkernel Test
#---------------------------------------------------------------------------------------------------------------------------------------
#                                                       Directory Settings
#---------------------------------------------------------------------------------------------------------------------------------------

#TODO Generate by scl
# all task directories for this configuration. used for building see kernel.mk target: tasks, tasks_clean
TASKS = ./tasks/task/ 
#./tasks/task2


#This is where the kernel source can be found
KERNEL_DIR = $(CWD)/$(RELATIVE_SOURCE_PATH)/source/
KERNEL_LIB_DIR = $(CWD)/$(RELATIVE_SOURCE_PATH)/source/lib/


#Only needed if xmd or bdi are used
XMD_DIR = ./xmd/
BDI_DIR = ./tftpboot/

BDI_CONFIG = omap35xx.cfg
BDI_REGDEF = regOMAP3500.def

#Output Directory. This is where the objects and the final binary go
OUTPUT_DIR = ./output/
MODULES_DIR = ./modules/

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Compile/Link Settings
#---------------------------------------------------------------------------------------------------------------------------------------

# The Linker Script used for this configuration
LINKERSCRIPT = kernel.ld   

#Optimization flag
OPT_FLAGS = -O2
USER_LIB_OPT_FLAGS = -O0

#Command line arguments to compile .c files.
CFLAGS = -Wall -g -I$(KERNEL_DIR)inc/ -I$(KERNEL_DIR)comm/lwip/include/ -I$(KERNEL_DIR)comm/lwip/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -ffunction-sections -fno-exceptions -fno-unwind-tables -msoft-float -fno-stack-protector -Wno-write-strings -c  -march=armv4t -mthumb -mthumb-interwork
#$(OPT_FLAGS) 

#Command line arguments to compile .cc files.
CPFLAGS = -Wall -g -I$(KERNEL_DIR)inc/ -I$(KERNEL_DIR)comm/lwip/include/ -I$(KERNEL_DIR)comm/lwip/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -ffunction-sections -fno-exceptions -fno-unwind-tables -fno-rtti -msoft-float -march=armv4t -fno-stack-protector -c -Wno-write-strings -Wuninitialized -Woverloaded-virtual -c 
#$(OPT_FLAGS)

#Command line arguments to the linker.
#LDFLAGS = -o output/kernel.elf -L/opt/$(ARM_GCC_DIR)/arm-none-eabi/lib -L/opt/$(ARM_GCC_DIR)/lib/gcc/arm-none-eabi/$(GCC_VERSION)/thumb -lc -lgcc --script=$(LINKERSCRIPT)

LDFLAGS = -o output/kernel.elf -L$(LIBC_DIR) -L$(GCC_LIB_DIR) -Map output/kernel.map -lc -lgcc --script=$(LINKERSCRIPT)

#Command line arguments for the gcc to assemble .S files.
ASFLAGS = -c -g -I$(KERNEL_DIR)inc/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -fno-exceptions -fno-rtti -msoft-float -fno-stack-protector -march=armv4t

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Makefile Inclusion
#---------------------------------------------------------------------------------------------------------------------------------------

#STACK_USAGE_SCRIPT = $(KERNEL_DIR)../tools/PPCStackUsage/stack_usage.py

#include architekture make file
#TODO: autogenerate this by scl
-include $(ARCH_DIR)/arch.mk

#include kernel make file
-include $(KERNEL_DIR)/kernel.mk

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                     User Defined Make Targets
#---------------------------------------------------------------------------------------------------------------------------------------
	