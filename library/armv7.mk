
BUILD_PLATFORM = armv7

# The platform flags: ARM + arm mode
PLATFORM_FLAGS = 0x01

# for windows we must specify the ld directory with absolute path 
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
SED		= sed

KERNEL_LIB_DIR = $(ORCOS_LIB_DIR)/bin/armv7


#no -mthumb until thread entry and static.o task_main are in the same mode
AFLAGS =
CFLAGS = -c -I$(ORCOS_LIB_DIR)/inc/ -I$(ORCOS_LIB_DIR)/libc/include/ -I$(ORCOS_LIB_DIR)/libc/time/ -I$(ORCOS_LIB_DIR)/inc/$(BUILD_PLATFORM) -mno-unaligned-access -fno-exceptions -O2  -Wno-write-strings -ffunction-sections  -mcpu=cortex-a8  -mfpu=neon 
CFLAGS += -fno-builtin -mno-unaligned-access -fno-section-anchors -fno-if-conversion2
CPPFLAGS = $(CFLAGS) -g -fno-rtti
