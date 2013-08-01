#.SUFFIXES:
#.SUFFIXES:      .S .c .o .cof .eep .hex .cc



#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Tools Settings
#---------------------------------------------------------------------------------------------------------------------------------------
#The tools used
#GCC_ID = powerpc-405-linux-gnu
GCC_ID = powerpc-eabi
#GCC_DIR = /home/coconut/$(GCC_ID)
GCC_DIR = /usr/local/$(GCC_ID)
GCC_EXE_PREFIX = $(GCC_DIR)/bin/$(GCC_ID)-

AS      = $(GCC_EXE_PREFIX)as
RM      = rm -f
RN      = mv
CP      = cp
SHELL 	= sh
REMOVE 	= rm -f
COPY 	= cp
LD      = $(GCC_EXE_PREFIX)ld
OBJCOPY = $(GCC_EXE_PREFIX)objcopy
SIZE    = $(GCC_EXE_PREFIX)size
#---------------------------------------------------------------------------------------------------------------------------------------
#                                                       Directory Settings
#---------------------------------------------------------------------------------------------------------------------------------------

OUTPUT_DIR = ./

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Compile/Link Settings
#---------------------------------------------------------------------------------------------------------------------------------------

# The Linker Script used for this configuration
LINKERSCRIPT = ./linkerscript.ld   

#Optimization flag
OPT_FLAGS = -Os

#Command line arguments to compile .c files.
CFLAGS = -g -I. -fno-exceptions -fno-rtti -msoft-float -fno-stack-protector -c $(OPT_FLAGS)

#Command line arguments to compile .cc files.
CPFLAGS = -g -I. -fno-exceptions -fno-rtti -msoft-float -fno-stack-protector -Wuninitialized -Woverloaded-virtual -c $(OPT_FLAGS)

#Command line arguments to the linker.
LDFLAGS = -o bootrom.elf --script=$(LINKERSCRIPT) --verbose

#Command line arguments for the gcc to assemble .S files.
ASFLAGS = -I.


#To change parameters and settings for a single configuration, 
#the Makefile in the configuration directory should be used.

KOBJ += bootrom.o

OBJ = $(addprefix $(OUTPUT_DIR),$(KOBJ))


#Every directory containing source code must be specified in KERNEL_PATH or ARCH_VPATH, 
#depending on whether they are common kernel code or specific to the archectiture.
VPATH = $(addprefix $(KERNEL_DIR), $(KERNEL_VPATH)) $(ARCH_VPATH)

KERNEL_VPATH = .

all: $(OUTPUT_DIR)bootrom.elf binary size

clean: 
	@echo Makefile: cleaning up...
	$(RM) $(OUTPUT_DIR)bootrom.o
	$(RM) $(OUTPUT_DIR)bootrom.elf

# Assemble: create object files from assembler source files. 
$(OUTPUT_DIR)%.o : %.S 
	@echo
	@echo Makefile: Assembling $<
	$(AS) $(ASFLAGS) $(OUTPUT_DIR)$< -o $(OUTPUT_DIR)$@
		

#final linking rule		
$(OUTPUT_DIR)bootrom.elf: $(OBJ)
	@echo
	@echo Makefile: Linking $@
	$(LD) $(LDFLAGS) -L. $(OBJ) -o bootrom.elf 

	
size: $(OUTPUT_DIR)bootrom.elf
	$(SIZE) $(OUTPUT_DIR)bootrom.elf
	
binary:
	$(OBJCOPY) -O binary $(OUTPUT_DIR)bootrom.elf $(OUTPUT_DIR)ppc405_rom.bin

