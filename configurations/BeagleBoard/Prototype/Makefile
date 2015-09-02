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

#USER_OPT_FLAGS = -O0

# -ffunction-sections 
#Command line arguments to compile .c files.

#USER_CFLAGS  =  -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes -Wmissing-prototypes -Wconversion 

#Command line arguments to compile .cc files.
#USER_CPFLAGS =  -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wconversion

USER_LDFLAGS = 


#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Makefile Inclusion
#---------------------------------------------------------------------------------------------------------------------------------------

#include kernel make file
-include $(KERNEL_DIR)/kernel.mk

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                     User Defined Make Targets
#---------------------------------------------------------------------------------------------------------------------------------------
	
