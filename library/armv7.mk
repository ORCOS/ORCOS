
BUILD_PLATFORM = armv7

# The platform flags: ARM + thumb mode
PLATFORM_FLAGS = 0x101

CC      = /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-gcc
CXX     = /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-c++
AS      = /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-as
AR      = /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-ar
SIZE	= /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-size
OBJDUMP = /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-objdump
LD		= /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-ld
OBJCOPY = /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-objcopy 
STRIP   = /cygdrive/c/yagarto-20121222/bin/arm-none-eabi-strip
SED		= sed

KERNEL_LIB_DIR = $(ORCOS_LIB_DIR)/bin/armv7

GCC_VERSION=4.7.2
GCC_LIB_DIR=C:/yagarto-20121222/lib/gcc/arm-none-eabi/$(GCC_VERSION)/thumb/

AFLAGS =
CFLAGS = -c -I$(ORCOS_LIB_DIR)/inc -I$(ORCOS_LIB_DIR)/inc/$(BUILD_PLATFORM) -fno-exceptions -fno-rtti -O2 -mthumb -Wno-write-strings -ffunction-sections
CPPFLAGS = $(CFLAGS) -g
