
BUILD_PLATFORM = ppc405

# The platform flags: PPC
PLATFORM_FLAGS = 0x002

# for windows we must specify the ld directory with absolute path 
GCC_LIB_DIR=D:/toolchains/powerpc-eabi/lib/gcc/powerpc-eabi/4.8.0/
LIBC_DIR=D:/toolchains/powerpc-eabi/powerpc-eabi/lib/

GCC_PATH	= /cygdrive/d/toolchains/powerpc-eabi
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
SED			= sed

KERNEL_LIB_DIR = $(ORCOS_LIB_DIR)/bin/ppc405


AFLAGS =
CFLAGS = -c -I$(ORCOS_LIB_DIR)/inc -I$(ORCOS_LIB_DIR)/inc/$(BUILD_PLATFORM) -fno-exceptions  -msoft-float -fno-stack-protector -fno-rtti -O2 -Wno-write-strings -ffunction-sections
CPPFLAGS = $(CFLAGS) -g
