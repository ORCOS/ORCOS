#Configuration Makefile

#Relative path to the directory containing the source directory
DIR_PREFIX = ../../..

# Current working directory
CWD = $(shell pwd)


#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Tools Settings
#---------------------------------------------------------------------------------------------------------------------------------------
#GCC
GCC_PATH	= /opt/freescale-4.4
CC      	= $(GCC_PATH)/bin/powerpc-eabi-gcc
CXX     	= $(GCC_PATH)/bin/powerpc-eabi-c++
AS      	= $(GCC_PATH)/bin/powerpc-eabi-as
AR      	= $(GCC_PATH)/bin/powerpc-eabi-ar
SIZE		= $(GCC_PATH)/bin/powerpc-eabi-size
OBJDUMP 	= $(GCC_PATH)/bin/powerpc-eabi-objdump
LD			= $(GCC_PATH)/bin/powerpc-eabi-ld
OBJCOPY 	= $(GCC_PATH)/bin/powerpc-eabi-objcopy 
STRIP   	= $(GCC_PATH)/bin/powerpc-eabi-strip
GCC_LIB		= $(GCC_PATH)/powerpc-eabi/lib/

#SH Tools
RM      	= rm -rf
RN      	= mv
CP      	= cp
SHELL 		= sh
COPY 		= cp
SED 		= sed

# BDI SETTINGS
BDICOMMAND = "$(KERNEL_DIR)../tools/BDICommand/BDICommand.py"
BDIADRESS  = "192.168.1.1"
BDIPORT	   = "23"

#SCL SETTINGS
SCL 	= java -jar $(DIR_PREFIX)/tools/SCL/dist/scl2.jar SCLConfig.xml  $(DIR_PREFIX)/tools/SCL/dist/scl2.xsd --check-deps $(DIR_PREFIX)/source/SCLdependencies.xml  $(DIR_PREFIX)/tools/SCL/dist/scl2deps.xsd 

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                       Directory Settings
#---------------------------------------------------------------------------------------------------------------------------------------

#TODO Generate by scl
TASKS = ./tasks/remotecommtask1/

#This is the architecture directory we want to use
ARCH_DIR = $(KERNEL_DIR)arch/PPC40x/

#Only needed if xmd or bdi are used
BDI_DIR = ./BDI
XMD_DIR = ./XMD

#Output Directory. This is where the objects and the final binary go
OUTPUT_DIR = ./output/

#This is where the kernel source can be found
KERNEL_DIR = $(CWD)/$(DIR_PREFIX)/source/
KERNEL_LIB_DIR = $(CWD)/$(DIR_PREFIX)/source/lib/

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Compile/Link Settings
#---------------------------------------------------------------------------------------------------------------------------------------

# The Linker Script used for this configuration
LINKERSCRIPT = kernel.ld   

#Optimization flag
OPT_FLAGS = -O2

#Command line arguments to compile .c files.
CFLAGS = -Wall -g -I$(KERNEL_DIR)inc/ -I$(KERNEL_DIR)comm/lwip/include/ -I$(KERNEL_DIR)comm/lwip/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -fno-exceptions -fno-rtti -msoft-float -fno-stack-protector -c $(OPT_FLAGS)

#Command line arguments to compile .cc files.
CPFLAGS = -Wall -g -I$(KERNEL_DIR)inc/ -I$(KERNEL_DIR)comm/lwip/include/ -I$(KERNEL_DIR)comm/lwip/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -fno-exceptions -fno-rtti -msoft-float -fno-stack-protector -Wuninitialized -Woverloaded-virtual -c $(OPT_FLAGS)

#Command line arguments to the linker.
LDFLAGS = -o output/kernel.elf --entry=startORCOS --script=$(LINKERSCRIPT) -L/usr/local/powerpc-eabi/lib/gcc/powerpc-eabi/4.2.3/nof

#Command line arguments for the gcc to assemble .S files.
ASFLAGS = -g -I$(KERNEL_DIR)inc/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -fno-exceptions  -fno-rtti -msoft-float -Xassembler -m405 -c

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Makefile Inclusion
#---------------------------------------------------------------------------------------------------------------------------------------

STACK_USAGE_SCRIPT = $(KERNEL_DIR)../tools/PPCStackUsage/stack_usage.py

#include architekture make file
#TODO: autogenerate this by scl
-include $(ARCH_DIR)/RAPTOR/arch.mk

#include kernel make file
-include $(KERNEL_DIR)/kernel.mk
