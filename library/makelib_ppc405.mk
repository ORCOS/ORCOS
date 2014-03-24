#.SUFFIXES:
#.SUFFIXES:      .S .c .o .cof .eep .hex .cc

-include ppc405.mk


#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Compile/Link Settings
#---------------------------------------------------------------------------------------------------------------------------------------
OUTPUT_DIR = bin/ppc405/
ORCOS_LIB_DIR = .

OPT_FLAGS = -O2  
USER_LIB_OPT_FLAGS = -O2 

#Command line arguments to compile .cc files.
CPFLAGS = $(CFLAGS)

#Command line arguments for the gcc to assemble .S files.
ASFLAGS = -c -g -Iinc/ -fno-exceptions  -msoft-float -fno-stack-protector

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Rules
#---------------------------------------------------------------------------------------------------------------------------------------

OBJ = $(addprefix $(OUTPUT_DIR),$(KOBJ))

KOBJ += io.o mem.o signal.o net.o static.o threads.o Mutex.o pthread.o syscall.o testandset.o string.o

VPATH = ../source/arch/PPC40x src

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
	@echo " Building ORCOS Syscall Library for PPC405"
	@echo "------------------------------------------"
	@if ! [ -e $(OUTPUT_DIR) ]; then mkdir $(OUTPUT_DIR); fi 
	@make -f makelib_ppc405.mk -s bin/ppc405/liborcos.a


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
bin/ppc405/liborcos.a: $(OBJ)
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


