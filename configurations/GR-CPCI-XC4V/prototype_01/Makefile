#Configuration Makefile

#Relative path to the directory containing the source directory
DIR_PREFIX = ../../..

# Current working directory
CWD = $(shell pwd)


#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Tools Settings
#---------------------------------------------------------------------------------------------------------------------------------------
#GCC Version
GCC_VERSION = 4.3.3

#Location of compiler binaries
COMP_DIR = /opt/xgcc-sparc-elf-$(GCC_VERSION)

#The tools used
CC      = $(COMP_DIR)/bin/xgcc-sparc-elf-gcc-$(GCC_VERSION)
CXX     = $(COMP_DIR)/bin/xgcc-sparc-elf-c++-$(GCC_VERSION)
AS      = $(COMP_DIR)/bin/xgcc-sparc-elf-as-$(GCC_VERSION)
AR      = $(COMP_DIR)/bin/xgcc-sparc-elf-ar-$(GCC_VERSION)
RM      = rm -f
RN      = mv
CP      = cp
SHELL 	= sh
REMOVE 	= rm -f
COPY 	= cp
SED 	= sed
SIZE	= $(COMP_DIR)/bin/xgcc-sparc-elf-size-$(GCC_VERSION)
OBJDUMP = $(COMP_DIR)/bin/xgcc-sparc-elf-objdump-$(GCC_VERSION)
LD		= $(COMP_DIR)/bin/xgcc-sparc-elf-ld-$(GCC_VERSION)
OBJCOPY = $(COMP_DIR)/bin/xgcc-sparc-elf-objcopy-$(GCC_VERSION)
STRIP   = $(COMP_DIR)/bin/xgcc-sparc-elf-strip-$(GCC_VERSION)

#SCL SETTINGS
SCL 	= java -jar $(DIR_PREFIX)/tools/SCL/dist/scl2.jar SCLConfig.xml  $(DIR_PREFIX)/tools/SCL/dist/scl2.xsd --check-deps $(DIR_PREFIX)/source/SCLdependencies.xml  $(DIR_PREFIX)/tools/SCL/dist/scl2deps.xsd 

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                       Directory Settings
#---------------------------------------------------------------------------------------------------------------------------------------

#TODO Generate by scl
TASKS = ./tasks/task1/ ./tasks/task2/ ./tasks/debugtask/

#This is the architecture directory we want to use
ARCH_DIR =  $(KERNEL_DIR)arch/Leon3/

#Output Directory. This is where the objects and the final binary go
OUTPUT_DIR = ./output/
XMD_DIR = ./xmd/
BDI_DIR = ./bdi/

#This is where the kernel source can be found
KERNEL_DIR = $(CWD)/$(DIR_PREFIX)/source/
KERNEL_LIB_DIR = $(CWD)/$(DIR_PREFIX)/source/lib/

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Compile/Link Settings
#---------------------------------------------------------------------------------------------------------------------------------------

# The Linker Script used for this configuration
LINKERSCRIPT = kernel.ld   

#Optimization flag
OPT_FLAGS =

#Command line arguments to compile .c files.
CFLAGS  = -Wall -c -g -I$(KERNEL_DIR)inc/ -I$(KERNEL_DIR)comm/lwip/include/ -I$(KERNEL_DIR)comm/lwip/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -fno-exceptions -msoft-float -mcpu=v8  $(OPT_FLAGS)
CFLAGS_TASKS = -mcpu=v8

#Command line arguments to compile .cc files.
CPFLAGS = -Wall -c -g -I$(KERNEL_DIR)inc/ -I$(KERNEL_DIR)comm/lwip/include/ -I$(KERNEL_DIR)comm/lwip/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -fno-exceptions -fno-rtti -msoft-float -mcpu=v8  $(OPT_FLAGS)

#Command line arguments to the linker.
LDFLAGS = -o output/kernel.elf --entry=startORCOS --script=$(LINKERSCRIPT) $(ADD_LDFLAGS)

#Command line arguments for the gcc to assemble .S files.
ASFLAGS = -c -g -I$(KERNEL_DIR)inc/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -fno-exceptions  -fno-rtti -msoft-float -mcpu=v8

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Makefile Inclusion
#---------------------------------------------------------------------------------------------------------------------------------------

#include architekture make file
#TODO: autogenerate this by scl
-include $(KERNEL_DIR)arch/Leon3/GR-CPCI-XC4V/arch.mk

#include kernel make file
-include $(KERNEL_DIR)/kernel.mk

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                     User Defined Make Targets
#---------------------------------------------------------------------------------------------------------------------------------------

COPY_DIR = ./GRMON
copy: all tasks
	$(CP) ./tasks/task1/task1.elf $(COPY_DIR)/task1.elf
	$(CP) ./tasks/task2/task2.elf $(COPY_DIR)/task2.elf
	$(CP) ./tasks/debugtask/debugtask.elf $(COPY_DIR)/debugtask.elf
	$(CP) $(OUTPUT_DIR)kernel.elf $(COPY_DIR)/kernel.elf
	$(CP) prototype_01.batch $(COPY_DIR)/prototype_01.batch