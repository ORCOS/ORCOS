
BUILD_PLATFORM = armv7

# The platform flags: ARM + thumb mode
PLATFORM_FLAGS = 0x101

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


AFLAGS =
CFLAGS = -c -I$(ORCOS_LIB_DIR)/inc -I$(ORCOS_LIB_DIR)/inc/$(BUILD_PLATFORM) -mno-unaligned-access -fno-exceptions -fno-rtti -O2 -mthumb -Wno-write-strings -ffunction-sections -march=armv7
CFLAGS += -fno-builtin -flto -mno-unaligned-access -fno-section-anchors -fno-if-conversion2 
CPPFLAGS = $(CFLAGS) -g
