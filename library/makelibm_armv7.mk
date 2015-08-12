#.SUFFIXES:
#.SUFFIXES:      .S .c .o .cof .eep .hex .cc

-include armv7.mk

C_FILES = $(shell find libm/common libm/complex libm/math libm/machine/arm libm/mathfp -type f -name '*.[c|S]')
LIBC_FILES = $(filter-out engine.c, $(notdir $(C_FILES) ))
LIBC_DIRS  = $(dir $(C_FILES))
#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Compile/Link Settings
#---------------------------------------------------------------------------------------------------------------------------------------
OUTPUT_DIR = bin/armv7/

PLUGIN_DIR=$(shell $(CC) -print-prog-name=liblto_plugin.so)

OPT_FLAGS = -O2  -mcpu=cortex-a8 -mthumb -mno-unaligned-access -DARM_THUMB=1 -flto -fno-builtin -fno-section-anchors -mfpu=neon

#Command line arguments to compile .c files.
CFLAGS =  -DINTERNAL_NEWLIB -Wall -g -Ilibc/include -Ilibm/common -Iinc/armv7/ -Iinc/ -ffunction-sections -fdata-sections -fno-exceptions -fno-unwind-tables -fno-stack-protector -Wno-write-strings -c $(CPU_FLAGS) 
#Command line arguments for the gcc to assemble .S files.
ASFLAGS = -DINTERNAL_NEWLIB -c -g -Iinc/ -fno-exceptions -flto -fno-rtti -msoft-float -fno-stack-protector -mcpu=cortex-a8

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Rules
#---------------------------------------------------------------------------------------------------------------------------------------

KOBJ = $(LIBC_FILES:.c=.o)
OBJ = $(addprefix $(OUTPUT_DIR),$(KOBJ))

VPATH = ../source/arch/ARM/ARMv4T ../source/arch/ARM/ARMv4T/ARMv7 $(LIBC_DIRS)

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
	@make -f makelibm_armv7.mk -s bin/armv7/libm.a


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



#Make liborcos.a
bin/armv7/libm.a: $(OBJ)
	@echo Archiving the library into libm.a
	@$(AR) qc $@ $(OBJ) --plugin $(PLUGIN_DIR)
	@echo 



