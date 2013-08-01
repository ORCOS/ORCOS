#Configuration Makefile

#Relative path to the directory containing the source directory
DIR_PREFIX = ../../..

# Current working directory
CWD = $(shell pwd)


#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Tools Settings
#---------------------------------------------------------------------------------------------------------------------------------------
#The tools used
TOOL_DIR = /home/gavinvaz
#GCC_ID = powerpc-405-linux-gnu
GCC_ID = powerpc-eabi
#GCC_VER = 4.0.2
#GCC_VER = 4.2.3
GCC_VER = 4.4.0
#GCC_DIR = $(TOOL_DIR)/$(GCC_ID)
#GCC_DIR = /usr/local/$(GCC_ID)
GCC_DIR = /home/gavinvaz/crossgcc-4.4.0-powerpc-eabi_2009-08-12
GCC_EXE_PREFIX = $(GCC_DIR)/bin/$(GCC_ID)-

CC      = $(GCC_EXE_PREFIX)gcc
CXX     = $(GCC_EXE_PREFIX)c++
AS      = $(GCC_EXE_PREFIX)as
AR      = $(GCC_EXE_PREFIX)ar
RM      = rm -f
RN      = mv
CP      = cp
SHELL 	= sh
REMOVE 	= rm -f
COPY 	= cp
SED 	= sed
SIZE	= $(GCC_EXE_PREFIX)size
OBJDUMP = $(GCC_EXE_PREFIX)objdump
LD		= $(GCC_EXE_PREFIX)ld
OBJCOPY = $(GCC_EXE_PREFIX)objcopy 
STRIP   = $(GCC_EXE_PREFIX)strip
GDB     = $(GCC_EXE_PREFIX)gdb

# BDI SETTINGS
BDICOMMAND = "$(KERNEL_DIR)../tools/BDICommand/BDICommand.py"
BDIADRESS  = "192.168.1.1"
BDIPORT	   = "23"

#SCL SETTINGS
SCL 	= java -jar $(DIR_PREFIX)/tools/SCL/dist/scl2.jar SCLConfig.xml  $(DIR_PREFIX)/tools/SCL/dist/scl2.xsd --check-deps $(DIR_PREFIX)/source/SCLdependencies.xml  $(DIR_PREFIX)/tools/SCL/dist/scl2deps.xsd 

BINMERGE = $(TOOL_DIR)/binmerge/binmerge
#DEBUGGER = $(TOOL_DIR)/gcc-4.1.0-glibc-2.3.6/powerpc-405-linux-gnu/bin/powerpc-405-linux-gnu-gdb
QEMU_PPC = $(TOOL_DIR)/qemu-0.9.1/ppcemb-softmmu/qemu-system-ppcemb
QEMU_GDB_PORT = 8888
#DDD		 = /usr/bin/ddd

#GCC_LIBDIR = $(TOOL_DIR)/gcc-4.1.0-glibc-2.3.6/powerpc-405-linux-gnu/lib/gcc/powerpc-405-linux-gnu/4.1.0/nof
GCC_LIBDIR = $(GCC_DIR)/lib/gcc/$(GCC_ID)/$(GCC_VER)/nof

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                       Directory Settings
#---------------------------------------------------------------------------------------------------------------------------------------

#TODO Generate by scl
# all task directories for this configuration. used for building see kernel.mk target: tasks, tasks_clean
TASKS = $(CWD)/$(DIR_PREFIX)/tasks/task1_qemu/

#FIXME needed for 'merge' target.
TASK_DIR1 = $(TASKS)
TASK_ADDRESS1 = 0x140000

#This is the architecture directory we want to use
ARCH_DIR = $(KERNEL_DIR)arch/PPC40x/

#Only needed if xmd or bdi are used
BDI_DIR = /cygdrive/c/BDI
XMD_DIR = /cygdrive/c/xmd

#Output Directory. This is where the objects and the final binary go
OUTPUT_DIR = ./output/

#This is where the kernel source can be found
KERNEL_DIR = $(CWD)/$(DIR_PREFIX)/source/
KERNEL_LIB_DIR = $(CWD)/$(DIR_PREFIX)/source/lib/

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Compile/Link Settings
#---------------------------------------------------------------------------------------------------------------------------------------

# The Linker Script used for this configuration
LINKERSCRIPT = kernel.ld   

#Optimization flag
OPT_FLAGS = -Os

#Command line arguments to compile .c files.
CFLAGS = -Wall -g -I$(KERNEL_DIR)inc/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -fno-exceptions -fno-rtti -msoft-float -c $(OPT_FLAGS)

#Command line arguments to compile .cc files.
CPFLAGS = -Wall -g -I$(KERNEL_DIR)inc/  -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -fno-exceptions -fno-rtti -msoft-float -Wuninitialized -Woverloaded-virtual -c $(OPT_FLAGS)

#Command line arguments to the linker.
LDFLAGS = -o output/kernel.elf --entry=startORCOS --script=$(LINKERSCRIPT) #-L$(GCC_LIBDIR) -lgcc

#Command line arguments for the gcc to assemble .S files.
ASFLAGS = -g -I$(KERNEL_DIR)inc/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -fno-exceptions  -fno-rtti -msoft-float -Xassembler -m405 -c

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Makefile Inclusion
#---------------------------------------------------------------------------------------------------------------------------------------

#include architekture make file
#TODO: autogenerate this by scl
-include $(ARCH_DIR)/RAPTOR/arch.mk

#include kernel make file
-include $(KERNEL_DIR)/kernel.mk

#---------------------------------------------------------------------------------------------------------------------------------------
#                                                     User Defined Make Targets
#---------------------------------------------------------------------------------------------------------------------------------------


bdisftp: all tasks_clean tasks
	sftp -b bdiremote.sftp morpheus@hoernsche.cs.upb.de
	$(BDICOMMAND) "localhost" "2323" "load" "load task1.elf" "load task2.elf" "load kernel.elf"

xmdsftp: all tasks_clean tasks
	sftp -b xmdremote.sftp Administrator@10.211.55.6

run:
	@echo Running target
	@python $(BDICOMMAND) $(BDIADRESS) $(BDIPORT) "go"

runremote:
	@echo Running target
	$(BDICOMMAND) "localhost" "2323" "go"

$(TASK_DIR1)task1_qemu.bin: tasks

$(OUTPUT_DIR)kernel_with_jump.bin: $(OUTPUT_DIR)../jump.bin $(OUTPUT_DIR)main.bin
	@echo Merge jump.bin and the kernel main.bin
	$(BINMERGE)  "$(OUTPUT_DIR)../jump.bin" "$(OUTPUT_DIR)main.bin" $@ "0x0" "0x4000"

$(OUTPUT_DIR)orcos_kernel_and_tasks.bin: $(OUTPUT_DIR)kernel_with_jump.bin $(TASK_DIR1)task1_qemu.bin Makefile
	@echo Merge kernel_with_jump.bin with the tasks
	$(BINMERGE)  "$(OUTPUT_DIR)kernel_with_jump.bin" "$(TASK_DIR1)task1_qemu.bin" $@ "0x0" "$(TASK_ADDRESS1)"

merge: $(OUTPUT_DIR)empty.bin $(OUTPUT_DIR)orcos_kernel_and_tasks.bin

$(OUTPUT_DIR)empty.bin:
	@echo Creating empty bin file for qemu
	echo > "$@"

gdbinit: Makefile
	@echo Generating gdbinit file...
	echo "add-symbol-file $(TASK_DIR1)task1_qemu.elf $(TASK_ADDRESS1)" > gdbinit

ORCOS_qemu_test.launch: ORCOS_qemu_test.launch.in Makefile
	@echo Generating launcher to debug ORCOS
	$(SED) 's;@GDB@;'"$(GDB)"';g' $< | $(SED) 's;@GDBINIT@;'"$(CWD)"'/gdbinit;g' > "$@"

qemu_test.launch: qemu_test.launch.in Makefile
	@echo Generating launcher to start qemu
	$(SED) 's;@QEMU@;'"$(QEMU_PPC)"';g' $< | $(SED) 's;@QEMU_ARGS@;'"-S -s -p $(QEMU_GDB_PORT) -L $(CWD)/$(DIR_PREFIX)/tools/BootRomFile -nographic -M ref405ep -cpu 401100c4 -kernel $(CWD)/$(OUTPUT_DIR)orcos_kernel_and_tasks.bin $(CWD)/$(OUTPUT_DIR)empty.bin"';g' > "$@"

qemu.sh: Makefile
	@echo Generating shell script to start qemu
	@echo "#!/bin/sh" > $@
	@echo "exec $(QEMU_PPC) -S -s -p $(QEMU_GDB_PORT) -L $(CWD)/$(DIR_PREFIX)/tools/BootRomFile -nographic -M ref405ep -cpu 401100c4 -kernel $(CWD)/$(OUTPUT_DIR)orcos_kernel_and_tasks.bin $(CWD)/$(OUTPUT_DIR)empty.bin" >> $@
	@chmod 755 $@

gdb.sh: Makefile
	@echo Generating shell script to start gdb
	@echo "#!/bin/sh" > $@
	@echo 'echo "INFO: --------------------------------------------------------------"' >> $@
	@echo 'echo "INFO: connect to target with: \"target extended-remote localhost:'$(QEMU_GDB_PORT)'\""' >> $@
	@echo 'echo "INFO: --------------------------------------------------------------"' >> $@
	@echo "exec $(GDB) -x $(CWD)/gdbinit $(OUTPUT_DIR)/kernel.elf" >> $@
	@chmod 755 $@

preqemu: merge gdbinit ORCOS_qemu_test.launch qemu_test.launch qemu.sh gdb.sh

#debug:
#	echo Debug ORCOS kernel and tasks with DDD on QEMU PowerPC405 system emulation
#	echo "target remote localhost:8888" >gdb_commands
#	echo "add-symbol-file $(OUTPUT_DIR)kernel.elf 0x4000" >>gdb_commands
#	echo "add-symbol-file $(TASK_DIR1)task1_qemu.elf 0x40000" >>gdb_commands
#	echo "b task_main" >>gdb_commands
#	echo "c" >>gdb_commands
#	echo "$(QEMU_PPC) -S -s -p 8888 -L /home/coconut/orcos/tools/BootRomFile -nographic -M ref405ep -cpu 41810000 -kernel $(OUTPUT_DIR)orcos_kernel_and_tasks.bin ./output/empty.bin &" >start.sh 
#	echo "$(SLEEP) 1 ">>start.sh
#	echo "$(DDD) --debugger $(DEBUGGER) --directory=$(DIR_PREFIX) -x gdb_commands $(OUTPUT_DIR)kernel.elf" >>start.sh
#	sh start.sh
