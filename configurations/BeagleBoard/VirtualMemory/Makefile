#Configuration Makefile
CWD = $(shell pwd)
RELATIVE_SOURCE_PATH = ../../..


#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Tools Settings
#---------------------------------------------------------------------------------------------------------------------------------------
#GCC
CC      = /opt/arm-none-eabi/bin/arm-none-eabi-gcc
CXX     = /opt/arm-none-eabi/bin/arm-none-eabi-c++
AS      = /opt/arm-none-eabi/bin/arm-none-eabi-as
AR      = /opt/arm-none-eabi/bin/arm-none-eabi-ar
SIZE	= /opt/arm-none-eabi/bin/arm-none-eabi-size
OBJDUMP = /opt/arm-none-eabi/bin/arm-none-eabi-objdump
LD		= /opt/arm-none-eabi/bin/arm-none-eabi-ld
OBJCOPY = /opt/arm-none-eabi/bin/arm-none-eabi-objcopy 
STRIP   = /opt/arm-none-eabi/bin/arm-none-eabi-strip

#SH Tools
RM      	= rm -rf
RN      	= mv
CP      	= cp
SHELL 		= sh
COPY 		= cp
SED 		= sed

#SCL SETTINGS
SCL 	= java -jar $(RELATIVE_SOURCE_PATH)/tools/SCL/dist/scl2.jar SCLConfig.xml  $(RELATIVE_SOURCE_PATH)/tools/SCL/dist/scl2.xsd --check-deps $(RELATIVE_SOURCE_PATH)/source/SCLdependencies.xml  $(RELATIVE_SOURCE_PATH)/tools/SCL/dist/scl2deps.xsd 

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                       Directory Settings
#---------------------------------------------------------------------------------------------------------------------------------------

#TODO Generate by scl
# all task directories for this configuration. used for building see kernel.mk target: tasks, tasks_clean
TASKS = ./tasks/task/ ./tasks/task2

#This is where the kernel source can be found
KERNEL_DIR = $(CWD)/$(RELATIVE_SOURCE_PATH)/source/
KERNEL_LIB_DIR = $(CWD)/$(RELATIVE_SOURCE_PATH)/source/lib/

#This is the architecture directory we want to use
ARCH_DIR = $(KERNEL_DIR)arch/ARM/ARMv4T/ARMv7/BeagleBoard/

#Only needed if xmd or bdi are used
XMD_DIR = ./xmd/
BDI_DIR = ./bdi/

BDI_CONFIG = omap35xx.cfg
BDI_REGDEF = regOMAP3500.def

#Output Directory. This is where the objects and the final binary go
OUTPUT_DIR = ./output/

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Compile/Link Settings
#---------------------------------------------------------------------------------------------------------------------------------------

# The Linker Script used for this configuration
LINKERSCRIPT = kernel.ld   

#Optimization flag
OPT_FLAGS = -O0

#Command line arguments to compile .c files.
CFLAGS = -Wall -g -I$(KERNEL_DIR)inc/ -I$(KERNEL_DIR)comm/lwip/include/ -I$(KERNEL_DIR)comm/lwip/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -ffunction-sections -fno-exceptions -fno-rtti -msoft-float -fno-stack-protector -c $(OPT_FLAGS) 

#Command line arguments to compile .cc files.
CPFLAGS = -Wall -g -I$(KERNEL_DIR)inc/ -I$(KERNEL_DIR)comm/lwip/include/ -I$(KERNEL_DIR)comm/lwip/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -ffunction-sections -fno-exceptions -fno-rtti -msoft-float -fno-stack-protector -c -Wuninitialized -Woverloaded-virtual -c $(OPT_FLAGS)

#Command line arguments to the linker.
LDFLAGS = -o output/kernel.elf -L/opt/arm-none-eabi/arm-none-eabi/lib -L/opt/arm-none-eabi/lib/gcc/arm-none-eabi/4.4.1/thumb -lc -lgcc --script=$(LINKERSCRIPT)
#LDFLAGS = -o output/kernel.elf -L/opt/arm-none-eabi/arm-none-eabi/lib -L/opt/arm-none-eabi/lib/gcc/arm-none-eabi/4.4.1/thumb  --script=$(LINKERSCRIPT)

#Command line arguments for the gcc to assemble .S files.
ASFLAGS = -c -g -I$(KERNEL_DIR)inc/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -fno-exceptions -fno-rtti -msoft-float -fno-stack-protector

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
	