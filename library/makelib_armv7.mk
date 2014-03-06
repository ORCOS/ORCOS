#.SUFFIXES:
#.SUFFIXES:      .S .c .o .cof .eep .hex .cc


#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Tools Settings
#---------------------------------------------------------------------------------------------------------------------------------------
#GCC SETTINGS
ARM_GCC_DIR=arm-2009q3
GCC_VERSION=4.7.4

# for windows we must specify the ld directory with absolute path 
GCC_LIB_DIR=C:/gcc-arm-none-eabi/lib/gcc/arm-none-eabi/$(GCC_VERSION)/armv7-ar/thumb/
LIBC_DIR=C:/gcc-arm-none-eabi/arm-none-eabi/lib/armv7-ar/thumb/

CC      = /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-gcc
CXX     = /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-c++
AS      = /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-as
AR      = /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-ar
SIZE	= /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-size
OBJDUMP = /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-objdump
LD		= /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-ld
OBJCOPY = /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-objcopy 
STRIP   = /cygdrive/c/gcc-arm-none-eabi/bin/arm-none-eabi-strip

#SH Tools
RM      	= rm -rf
RN      	= mv
CP      	= cp
SHELL 		= sh
COPY 		= cp
SED 		= sed

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Compile/Link Settings
#---------------------------------------------------------------------------------------------------------------------------------------
OUTPUT_DIR = bin/armv7/

OPT_FLAGS = -O2   -mcpu=cortex-a8  -mthumb-interwork -mno-unaligned-access -DARM_THUMB=1
USER_LIB_OPT_FLAGS = -O2 -mcpu=cortex-a8 -mno-unaligned-access 

#Command line arguments to compile .c files.
CFLAGS = -Wall -g -Iinc/ -Iinc/armv7/ -ffunction-sections -fdata-sections -fno-exceptions -fno-unwind-tables -msoft-float -fno-stack-protector -Wno-write-strings -c $(CPU_FLAGS) 

#Command line arguments to compile .cc files.
CPFLAGS = -Wall -g -Iinc/ -Iinc/armv7/ -ffunction-sections -fdata-sections -fno-exceptions -fno-unwind-tables -fno-rtti -msoft-float -fno-stack-protector -c -Wno-write-strings -Wuninitialized -Woverloaded-virtual $(CPU_FLAGS) 

#Command line arguments for the gcc to assemble .S files.
ASFLAGS = -c -g -Iinc/ -fno-exceptions -fno-rtti -msoft-float -fno-stack-protector -mcpu=cortex-a8

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Rules
#---------------------------------------------------------------------------------------------------------------------------------------

OBJ = $(addprefix $(OUTPUT_DIR),$(KOBJ))

KOBJ += io.o mem.o signal.o net.o static.o threads.o Mutex.o pthread.o syscall.o testandset.o string.o

VPATH = ../source/arch/ARM/ARMv4T ../source/arch/ARM/ARMv4T/ARMv7 src

checktools: 
	@echo
	@echo "----------------------------------"
	@echo "     Checking for tools"
	@echo "----------------------------------"
	@if ! [ -e $(CC) ]; then { echo "C-Compiler (CC) $(CC) not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(CXX) ]; then { echo "C++-Compiler (CXX) $(CXX) not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(AS) ]; then { echo "Assembler (AS) $(AS) not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(AR) ]; then { echo "Archiver (AR) $(AR) not found! Stopping!"; exit 1;} fi
	@if ! java -version >> /dev/null; then { echo "Java not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(OBJCOPY) ]; then { echo "Object-Copy (OBJCOPY) $(OBJCOPY) not found! Stopping!"; exit 1;} fi
	@echo All tools installed...


all: checktools		
	@echo "Please be sure to use relative path names for WINDOWS/CYGIN"
	@echo "------------------------------------------"
	@echo " Building ORCOS Syscall Library for ARMv7"
	@echo "------------------------------------------"
	@if ! [ -e $(OUTPUT_DIR) ]; then mkdir $(OUTPUT_DIR); fi 
	@make -f makelib_armv7.mk -s bin/armv7/liborcos.a


clean:
	rm $(OUTPUT_DIR)/*.*

# Assemble: create object files from assembler source files. 
$(OUTPUT_DIR)%.o : %.S 
	@echo makelib.mk[AS] : Assembling $@
	@$(CC) $(ASFLAGS) $< --output $@
		
#rule for compiling c++ files with header			
$(OUTPUT_DIR)%.o : %.cc %.hh 
	@echo makelib.mk[C++]: Compiling  $@
	@$(CXX) $(CPFLAGS) $(OPT_FLAGS)  $< --output $@

#rule for compiling c++ files without header
$(OUTPUT_DIR)%.o : %.cc 
	@echo makelib.mk[C++]: Compiling  $@
	@$(CXX) $(CPFLAGS) $(OPT_FLAGS)  $< --output $@

#rule for compiling c files with header	
$(OUTPUT_DIR)%.o : %.c %.h 
	@echo makelib.mk[C]   : Compiling  $@
	@$(CC) $(CFLAGS) $(OPT_FLAGS)  $< --output $@
	
#rule for compiling c files without header
$(OUTPUT_DIR)%.o : %.c
	@echo makelib.mk[C]   : Compiling  $@
	@$(CC) -c $(CFLAGS) $(OPT_FLAGS)  $< --output $@

$(OUTPUT_DIR)syscall.o : syscall.S
	@echo Syscall  : Compiling  $@
	@$(CC) -c $(CPFLAGS) $(USER_LIB_OPT_FLAGS)  $< --output $@


#Make liborcos.a
bin/armv7/liborcos.a: $(OBJ)
	@echo Archiving the user library into liborcos.a
	@$(AR) qc $@ $(USERLIB_OBJS)  $(OBJ)
	@echo 

lib/liborcos_newlib.a: $(USERNEWLIB_OBJS)
	@echo "------------------------------------"
	@echo "Building the User Library for newlib"
	@echo "------------------------------------"
	@make -s $(USERNEWLIB_OBJS)
	@echo Archiving the user library into liborcos_newlib.a
	@$(AR) rc $@ $(USERNEWLIB_OBJS)
	@echo


