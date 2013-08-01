#Configuration Makefile

#Relative path to the directory containing the source directory
DIR_PREFIX = ../../..

# Current working directory
CWD = $(shell pwd)


#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Tools Settings
#---------------------------------------------------------------------------------------------------------------------------------------
#The tools used
CC      = /usr/local/powerpc-eabi/bin/powerpc-eabi-gcc
CXX     = /usr/local/powerpc-eabi/bin/powerpc-eabi-c++
AS      = /usr/local/powerpc-eabi/bin/powerpc-eabi-as
AR      = /usr/local/powerpc-eabi/bin/powerpc-eabi-ar
RM      = rm -f
RN      = mv
CP      = cp
SHELL 	= sh
REMOVE 	= rm -f
COPY 	= cp
SED 	= sed
SIZE	= /usr/local/powerpc-eabi/bin/powerpc-eabi-size
OBJDUMP = /usr/local/powerpc-eabi/bin/powerpc-eabi-objdump
LD		= /usr/local/powerpc-eabi/bin/powerpc-eabi-ld
OBJCOPY = /usr/local/powerpc-eabi/bin/powerpc-eabi-objcopy 
STRIP   = /usr/local/powerpc-eabi/bin/powerpc-eabi-strip

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
TASKS = $(DIR_PREFIX)/tasks/task1/

#This is the architecture directory we want to use
ARCH_DIR = $(KERNEL_DIR)arch/PPC40x/

#Only needed if xmd or bdi are used
BDI_DIR = /cygdrive/c/BDI
XMD_DIR = /cygdrive/c/xmd

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
OPT_FLAGS = 
#-O2

#Command line arguments to compile .c files.
CFLAGS = -Wall -g -I$(KERNEL_DIR)inc/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -fno-exceptions -fno-rtti -msoft-float -fno-stack-protector -c $(OPT_FLAGS)

#Command line arguments to compile .cc files.
CPFLAGS = -Wall -g -I$(KERNEL_DIR)inc/  -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -fno-exceptions -fno-rtti -msoft-float -fno-stack-protector -Wuninitialized -Woverloaded-virtual -c $(OPT_FLAGS)

#Command line arguments to the linker.
LDFLAGS = -o output/kernel.elf --entry=startORCOS --script=$(LINKERSCRIPT) -L/usr/local/powerpc-eabi/lib/gcc/powerpc-eabi/4.2.3/nof

#Command line arguments for the gcc to assemble .S files.
ASFLAGS = -g -I$(KERNEL_DIR)inc/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -fno-exceptions  -fno-rtti -msoft-float -Xassembler -m405 -c

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Makefile Inclusion
#---------------------------------------------------------------------------------------------------------------------------------------

#include architekture make file
#TODO: autogenerate this by scl
-include $(ARCH_DIR)/RAPTOR/arch.mk

#include kernel make file
-include $(KERNEL_DIR)/kernel.mk

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                     User Defined Make Targets
#---------------------------------------------------------------------------------------------------------------------------------------
	
vmp: all tasks
	#rm /cygdrive/c/orcos/morpheus/ProteusVM/configurations/schedulingvmconf/vms/kernel.elf
	#rm /cygdrive/c/orcos/morpheus/ProteusVM/configurations/schedulingvmconf/vms/kernel.bin
	#rm /cygdrive/c/orcos/morpheus/ProteusVM/configurations/schedulingvmconf/vms/vm1_task1.elf
	#rm /cygdrive/c/orcos/morpheus/ProteusVM/configurations/schedulingvmconf/vms/vm1_task1.bin
	cp output/kernel.elf /cygdrive/c/orcos/morpheus/ProteusVM/configurations/schedulingvmconf/vms/kernel.elf
	cp output/main.bin /cygdrive/c/orcos/morpheus/ProteusVM/configurations/schedulingvmconf/vms/kernel.bin
	cp $(DIR_PREFIX)/tasks/task1/task1.elf /cygdrive/c/orcos/morpheus/ProteusVM/configurations/schedulingvmconf/vms/vm1_task1.elf
	cp $(DIR_PREFIX)/tasks/task1/task1.bin /cygdrive/c/orcos/morpheus/ProteusVM/configurations/schedulingvmconf/vms/vm1_task1.bin

#	cp output/kernel.elf /cygdrive/c/orcos/morpheus/ProteusVM/configurations/schedulingvmconf3/vms/kernel.elf
#	cp output/main.bin /cygdrive/c/orcos/morpheus/ProteusVM/configurations/schedulingvmconf3/vms/kernel.bin
#	cp $(DIR_PREFIX)/tasks/task1/task1.elf /cygdrive/c/orcos/morpheus/ProteusVM/configurations/schedulingvmconf3/vms/vm1_task1.elf
#	cp $(DIR_PREFIX)/tasks/task1/task1.bin /cygdrive/c/orcos/morpheus/ProteusVM/configurations/schedulingvmconf3/vms/vm1_task1.bin
	
bdisftp: all tasks_clean tasks
	sftp -b bdiremote.sftp morpheus@hoernsche.cs.upb.de
	$(BDICOMMAND) "localhost" "2323" "load" "load task1.elf" "load task2.elf" "load kernel.elf"

xmdsftp: all tasks_clean tasks
	sftp -b xmdremote.sftp Administrator@10.211.55.6
	
run:
	@echo Running target
	$(BDICOMMAND) "192.168.1.1" "23" "go"
	
runremote:
	@echo Running target
	$(BDICOMMAND) "localhost" "2323" "go"