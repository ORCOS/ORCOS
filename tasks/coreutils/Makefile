STACK_USAGE = python $(STACK_USAGE_SCRIPT)

OUTPUTFILE = cp

OBJS 	=  cp.o 
LIBOBJS =  $(KERNEL_LIB_DIR)/static.o 
# all provided defines are relative to the main makefile (usually two directories lower)
# exception is GCC_LIB_DIR which is usually absolute

#Provided variables:
 #CC, CXX, AS, LD,				: Compiler and linker 
 #OBJCOPY, OBJDUMP				: binutils
 #KERNEL_LIB_DIR				: The directory of the user ORCOS API sources (e.g. orcos.hh)
 #KERNEL_DIR					: The directory of the kernel sources
 #GCC_LIB_DIR					: Directory of libgcc
 #STACK_USAGE_SCRIPT		 	: Stack analyzer script (ppc only)
 #CFLAGS_TASKS= 				: Compile flags for tasks
 #LDFLAGS_TASKS				    : Linker flags for tasks 
 #TASK_DIR   					: This directory

CC      = /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-gcc
CXX     = /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-c++
AS      = /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-as
AR      = /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-ar
SIZE	= /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-size
OBJDUMP = /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-objdump
LD		= /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-ld
OBJCOPY = /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-objcopy 
STRIP   = /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-strip

KERNEL_LIB_DIR = C:/git/orcos_github/library/bin/armv7
KERNEL_INC_DIR = C:/git/orcos_github/library/inc

GCC_VERSION=4.7.4
GCC_LIB_DIR=C:/gcc-arm-none-eabi/lib/gcc/arm-none-eabi/$(GCC_VERSION)/

AFLAGS =
CFLAGS = -c -I$(KERNEL_INC_DIR) -fno-exceptions -fno-rtti -O2 -mno-unaligned-access -mthumb -ffunction-sections
CPPFLAGS = $(CFLAGS) -g  
	
all: $(OBJS) 
	
binary:
	@$(OBJCOPY) -O binary bin/$(OUTPUTFILE).elf bin/$(OUTPUTFILE)
	
stack_usage:
ifneq ($(STACK_USAGE_SCRIPT),)
	@$(STACK_USAGE) $(OBJDUMP) $(OUTPUTFILE).elf task_main
endif
		
task.elf: $(OBJS) task.ld
	@echo task.mk: Linking $(OUTPUTFILE).elf
	$(LD) $(OBJS) --gc-sections --script=task.ld  -o bin/$(OUTPUTFILE).elf -Map $(OUTPUTFILE).map -L$(KERNEL_LIB_DIR) -L$(GCC_LIB_DIR) -lorcos  -lgcc 
	
task.ld: 
	@echo task.mk: Generating linkerscript task.ld
	@$(SED) -f task.sed template.ld >task.ld
	
%.o: %.cpp
	@echo task.mk: Compiling $@
	@$(CXX) $(CPPFLAGS) $< -o bin/$@

%.o: %.c
	@echo task.mk: Compiling $@
	@$(CXX) $(CFLAGS) $< -o bin/$@

cp.o: cp.cc
	@echo @$(CXX) $(CPPFLAGS) $< -o $@
	@echo task.mk: Compiling $@
	@$(CXX) $(CPPFLAGS) $< -o bin/$@
	@$(LD) bin/$@ --script=task.ld  -o bin/cp.elf -Map cp.map -L$(KERNEL_LIB_DIR) -L$(GCC_LIB_DIR) -lorcos  -lgcc
	@$(OBJCOPY) -O binary bin/cp.elf bin/cp

%.o: %.S
	@echo task.mk: Assembling $@
	@$(AS) $(AFLAGS) $< -o $@
	
clean:
	@echo Cleaning up Task files
	@$(RM) bin/*
	