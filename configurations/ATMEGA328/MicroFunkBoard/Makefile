SHELL:=/bin/bash
#Configuration Makefile

#CWD = $(shell pwd)
CWD = .
override RELATIVE_SOURCE_PATH = ../../..

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Tools Settings
#---------------------------------------------------------------------------------------------------------------------------------------
#GCC SETTINGS
GCC_VERSION=4.7.2
#GCC_DIR=/usr/local/CrossPack-AVR/bin/
GCC_DIR=/cygdrive/d/toolchains/avr-gcc-4.7.2-mingw32/bin/

# for windows we must specify the ld directory with absolute path 
GCC_LIB_DIR=d:/toolchains/avr-gcc-4.7.2-mingw32/lib/gcc/avr/4.7.2/tiny-stack
#C:/gcc-arm-none-eabi/lib/gcc/arm-none-eabi/$(GCC_VERSION)/armv7-ar/thumb/
LIBC_DIR=. 
#C:/gcc-arm-none-eabi/arm-none-eabi/lib/armv7-ar/thumb/

CC      = $(GCC_DIR)avr-gcc
CXX     = $(GCC_DIR)avr-c++
AS      = $(GCC_DIR)avr-as
AR      = $(GCC_DIR)avr-ar
SIZE	= $(GCC_DIR)avr-size
OBJDUMP = $(GCC_DIR)avr-objdump
LD		= $(GCC_DIR)avr-ld
OBJCOPY = $(GCC_DIR)avr-objcopy 
STRIP   = $(GCC_DIR)avr-strip

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
KERNEL_LIB_DIR = $(CWD)/$(RELATIVE_SOURCE_PATH)/library/bin/atmega328/


#Output Directory. This is where the objects and the final binary go
OUTPUT_DIR = ./output/
MODULES_DIR = ./modules/

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Compile/Link Settings
#---------------------------------------------------------------------------------------------------------------------------------------

# The Linker Script used for this configuration
#LINKERSCRIPT = $(ARCH_DIR)/kernel.ld   

# if thumb mode is activated set this define to 1!
#CPU_FLAGS =  -I/cygdrive/d/toolchains/avr-gcc-4.7.2-mingw32/avr/include
 
#Optimization flag
#OPT_FLAGS = -Os -mmcu=atmega328
# -mthumb -mcpu=cortex-a8  -mthumb-interwork -mno-unaligned-access -DARM_THUMB=1
# User Library Optimization Flag. Keep as is
#USER_LIB_OPT_FLAGS = -Os -mmcu=atmega328
# -mno-unaligned-access

#Command line arguments to compile .c files.
#CFLAGS = -Wall -g -I$(KERNEL_DIR)inc/ -I./make/ -I$(KERNEL_DIR)comm/lwip/include/ -I$(KERNEL_DIR)comm/lwip/ -I. -I$(KERNEL_DIR) -I$(KERNEL_LIB_DIR)/../../ $(ARCH_INCLUDES) -ffunction-sections -flto -fno-exceptions -fno-unwind-tables -fno-stack-protector -Wno-write-strings -c $(CPU_FLAGS) 
#CFLAGS +=  -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wstrict-prototypes -Wmissing-prototypes


#Command line arguments to compile .cc files.
#CPFLAGS = -Wall -g -I$(KERNEL_DIR)inc/ -I./make/ -I$(KERNEL_DIR)comm/lwip/include/ -I$(KERNEL_DIR)comm/lwip/ -I. -I$(KERNEL_DIR) -I$(KERNEL_LIB_DIR)/../../ $(ARCH_INCLUDES) -ffunction-sections  -flto -fno-exceptions -fno-unwind-tables -fno-rtti -fno-stack-protector -c -Wno-write-strings -Wuninitialized -Woverloaded-virtual $(CPU_FLAGS) 
#CPFLAGS +=  -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align

#Command line arguments to the linker.
#LDFLAGS = -o output/kernel.elf -L$(LIBC_DIR) -L$(GCC_LIB_DIR) -Map output/kernel.map -lgcc  --gc-sections  --script=$(LINKERSCRIPT)
#
#Command line arguments for the gcc to assemble .S files.
#ASFLAGS = -c -g -I$(KERNEL_DIR)inc/ -I. -I./make/ -I$(KERNEL_DIR) $(ARCH_INCLUDES) -flto  -fno-exceptions -fno-rtti -fno-stack-protector -mmcu=atmega328
#ASFLAGS += -I/usr/local/CrossPack-AVR/avr/include
#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Makefile Inclusion
#---------------------------------------------------------------------------------------------------------------------------------------

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

#add dwarf debugging support (increases binray size by multiple KB due to method string table beeing added)
export ADD_DWARF := 1

# -fno-if-conversion2 enforces to not use THUMB ITT blocks..
# it seams like they are not correctly executed after context restore ...
# although the PSR is correct .. (ITSTATE)


USER_OPT_FLAGS = -Os  -mmcu=atmega328

# -ffunction-sections 
#Command line arguments to compile .c files.

USER_CFLAGS  =  -I/cygdrive/d/toolchains/avr-gcc-4.7.2-mingw32/avr/include

#Command line arguments to compile .cc files.
#USER_CPFLAGS =  -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align -Wconversion

USER_LDFLAGS = 
#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Makefile Inclusion
#---------------------------------------------------------------------------------------------------------------------------------------

ARCH_OBJ += task_main.o

#include kernel make file
-include $(KERNEL_DIR)/kernel.mk

