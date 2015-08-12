#.SUFFIXES:
#.SUFFIXES:      .S .c .o .cof .eep .hex .cc

-include armv7.mk

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Compile/Link Settings
#---------------------------------------------------------------------------------------------------------------------------------------
OUTPUT_DIR = bin/armv7/


OPT_FLAGS = -O2 -mcpu=cortex-a8  -mfpu=neon -mno-unaligned-access -fno-builtin -fno-section-anchors -fno-if-conversion2 
USER_LIB_OPT_FLAGS = -O2 -mcpu=cortex-a8 -mno-unaligned-access -mfpu=neon 

#Command line arguments to compile .c files.
CFLAGS = -Wall -g -Iinc/ -Ilibc/include/ -Ilibc/time/ -Iinc/armv7/ -std=c99 -ffunction-sections -flto -fdata-sections -fno-exceptions -fno-unwind-tables -fno-stack-protector -Wno-write-strings -c $(CPU_FLAGS) 

#Command line arguments to compile .cc files.
CPFLAGS = -Wall -g -Iinc/  -Ilibc/include/ -Ilibc/time/ -Iinc/armv7/ -ffunction-sections -flto -fdata-sections -fno-exceptions -fno-unwind-tables -fno-rtti -fno-stack-protector -c -Wno-write-strings -Wuninitialized -Woverloaded-virtual $(CPU_FLAGS) 

#Command line arguments for the gcc to assemble .S files.
ASFLAGS = -c -g -Iinc/ -fno-exceptions -fno-rtti -fno-stack-protector -mcpu=cortex-a8 -mfpu=neon -flto

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Rules
#---------------------------------------------------------------------------------------------------------------------------------------

KOBJ += io.o mem.o signal.o net.o static.o threads.o Mutex.o pthread.o syscall.o testandset.o string.o time.o stdlib.o random.o strtod.o 
OBJ = $(addprefix $(OUTPUT_DIR),$(KOBJ))

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
	cd $(OUTPUT_DIR) && rm -f $(KOBJ)

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
	@$(AR) rc $@ $(USERLIB_OBJS)  $(OBJ)
	@echo 
	
#Make liborcos.a
bin/armv7/libgloss.a: $(OUTPUT_DIR)libgloss.o
	@echo Archiving the syscall warpper library into libgloss.a
	@$(AR) rc $@ libgloss.o
	@echo 

lib/liborcos_newlib.a: $(USERNEWLIB_OBJS)
	@echo "------------------------------------"
	@echo "Building the User Library for newlib"
	@echo "------------------------------------"
	@make -s $(USERNEWLIB_OBJS)
	@echo Archiving the user library into liborcos_newlib.a
	@$(AR) rc $@ $(USERNEWLIB_OBJS)
	@echo


