#.SUFFIXES:
#.SUFFIXES:      .S .c .o .cof .eep .hex .cc

# kernel.mk contains the logic to compile the operating system. 
# It should only be changed, if the kernel is modified or the build process 
# should be changed in a general way. 
# This file must not be copy and pasted for different purposes.

# To change parameters and settings for a single configuration, 
# the Makefile in the configuration directory should be used.

-include make/scl_make.mk
-include $(ARCH_DIR)/arch.mk

KOBJ 	= $(notdir $(KASRC:.S=.o)) $(ARCH_OBJ)
OBJ 	= $(addprefix $(OUTPUT_DIR),$(KOBJ))
MODULES = $(addprefix $(MODULES_DIR),$(MODULE_OBJ))

# Overridden by configuration makefile
RELATIVE_SOURCE_PATH=..

#SCL SETTINGS
SCL 	= java -jar $(RELATIVE_SOURCE_PATH)/tools/SCL/dist/scl2.jar SCLConfig.xml  $(RELATIVE_SOURCE_PATH)/tools/SCL/dist/scl2.xsd --check-deps $(RELATIVE_SOURCE_PATH)/source/SCLdependencies.xml  $(RELATIVE_SOURCE_PATH)/tools/SCL/dist/scl2deps.xsd --verbose=3 
SCL_GEN = java -jar $(RELATIVE_SOURCE_PATH)/tools/SCL/dist/scl2.jar --generate-deps $(RELATIVE_SOURCE_PATH)/source/SCLdependencies.xml  $(RELATIVE_SOURCE_PATH)/tools/SCL/dist/scl2deps.xsd --verbose=3 

CPPLINT_FILTER=-readability/multiline_string,-whitespace/line_length,-whitespace/indent,-whitespace/comments,-readability/todo,-runtime/references,-runtime/printf,-build/include,-runtime/threadsafe_fn

# CPP_CHECK
CPP_CHECK_INCLUDES=-I$(KERNEL_DIR)inc/ -I./make/ -I$(KERNEL_DIR)comm/lwip/include/ -I$(KERNEL_DIR)comm/lwip/ -I. -I$(KERNEL_DIR) -I$(KERNEL_LIB_DIR)/../../ $(ARCH_INCLUDES) 
# addition static code checks. Deactivated by default. To be overriden by configuration Makefiles
CHECKSTYLE=0
CPPCHECK=0

# KOBJ and ARCH_OBJ specify the object-files used for linking.
# For every .cc, .c or .S file there must be a corresponding entry in the KOBJ or ARCH_OBJ list
# KOBJ specifies kernel-objects which are independent of the target archeticeture, 
# while ARCH_OBJ are objects which are specific to a given architecture. 
# It would make sense to exclude the latter list in an own file, as soon as more 
# than one platform is supported.

# the scl configuration tool generates the configurable list of objects to create
# the following list only contains the base classes or non configurable components

#config dir
KOBJ += tasktable.o __cxa_pure_virtual.o
#db
KOBJ += ArrayList.o LinkedList.o
#debug
KOBJ += Logger.o Trace.o dwarf.o
#filesystem
KOBJ += File.o Directory.o Resource.o SimpleFileManager.o SharedMemResource.o FileSystemBase.o Partition.o SysFs.o
#hal
KOBJ += PowerManager.o CharacterDevice.o BlockDeviceDriver.o TimerDevice.o CommDeviceDriver.o  Clock.o BufferDevice.o
# KOBJ += USCommDeviceDriver.o
#inc
KOBJ += stringtools.o memtools.o sprintf.o putc.o libgccmath.o endian.o crc32.o
#kernel
KOBJ += kwait.o kernelmain.o Kernel.o KernelServiceThread.o
#mem
KOBJ +=  new.o 
# scheduler
KOBJ += RoundRobinThreadScheduler.o PriorityThreadScheduler.o RateMonotonicThreadScheduler.o EarliestDeadlineFirstThreadScheduler.o
#process
KOBJ += IdleThread.o Task.o Thread.o TaskManager.o  Module.o
#robustness
KOBJ += TaskErrorHandler.o 
#synchro
KOBJ += Mutex.o 
#syscall
KOBJ += sc_common.o sc_io.o sc_mem.o sc_net.o sc_process.o sc_synchro.o sc_table.o

# sorted list of object files to create
BUILD_OBJ = $(sort $(OBJ))

# source locations 
KERNEL_VPATH = db/ debug/ filesystem/ hal/ inc/ inc/newlib/ kernel/ robust/ mem/ process/ scheduler/ syscalls/ 
KERNEL_VPATH+= synchro/ comm/ migration/ comm/servicediscovery/ comm/lwip/core/ comm/lwip/core/ipv4/ comm/lwip/netif/ 
KERNEL_VPATH+= comm/lwip/arch/ comm/lwip/core/ipv6/ comm/hcibluetooth/ arch/shared/ arch/shared/usb arch/shared/power

# Every directory containing source code must be specified in KERNEL_PATH or ARCH_VPATH, 
# depending on whether they are common kernel code or specific to the archectiture.
VPATH = $(addprefix $(KERNEL_DIR), $(KERNEL_VPATH)) $(ARCH_VPATH) make/
#---------------------------------------------------------------------------------------------------------------------------------------
#                                                      Pre-Build Checks
#---------------------------------------------------------------------------------------------------------------------------------------

check_defines:
ifndef MODULES_DIR
	$(error MODULES_DIR is undefined)
endif
ifndef OUTPUT_DIR
	$(error OUTPUT_DIR is undefined)
endif
ifndef ARCH_DIR
	$(error ARCH_DIR is undefined. Try running make all again, scl should have updated the ARCH_DIR variable.)
endif
ifndef GCC_LIB_DIR
	$(error GCC_LIB_DIR is undefined)
endif
ifndef LINKERSCRIPT
	$(error LINKERSCRIPT is undefined)
endif
ifndef KERNEL_LIB_DIR
	$(error KERNEL_LIB_DIR is undefined. Needed for linking tasks.)
endif
ifndef KERNEL_DIR
	$(error KERNEL_DIR is undefined)
endif

check_dirs:
	@if ! [ -e $(OUTPUT_DIR) ]; then mkdir $(OUTPUT_DIR); fi
	@if ! [ -e $(XMD_DIR) ]; then mkdir $(XMD_DIR); fi
	@if ! [ -e $(BDI_DIR) ]; then mkdir $(BDI_DIR); fi	
	@if ! [ -e $(MODULES_DIR) ]; then mkdir $(MODULES_DIR); fi

checktools:
	@echo
	@if ! [ -e $(ARCH_DIR) ]; then echo "ERROR: ARCH_DIR does not exist!"; exit -1; fi
	@if ! [ -e $(ARCH_DIR)/arch.mk ]; then echo "ERROR: no arch.mk found in $(ARCH_DIR)!"; exit -1; fi
	@if ! [ -e $(KERNEL_LIB_DIR) ]; then echo "ERROR: KERNEL_LIB_DIR does not exist! $(KERNEL_LIB_DIR)"; exit -1; fi
	@if ! [ -e $(KERNEL_DIR) ]; then echo "ERROR: KERNEL_DIR does not exist!"; exit -1; fi
	@echo "----------------------------------"
	@echo "     Checking for tools"
	@echo "----------------------------------"
	@if ! [ -e $(CC) ]; then { echo "C-Compiler (CC) $(CC) not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(CXX) ]; then { echo "C++-Compiler (CXX) $(CXX) not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(AS) ]; then { echo "Assembler (AS) $(AS) not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(AR) ]; then { echo "Archiver (AR) $(AR) not found! Stopping!"; exit 1;} fi
	@if ! java -version > /dev/null 2> /dev/null; then { echo "Java not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(OBJCOPY) ]; then { echo "Object-Copy (OBJCOPY) $(OBJCOPY) not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(GCC_LIB_DIR) ]; then { echo "GCC_LIB_DIR $(GCC_LIB_DIR) not found! Stopping!"; exit 1;} fi
	@echo All tools installed...
	@echo 
	
#---------------------------------------------------------------------------------------------------------------------------------------
#                                                     Target: all
#---------------------------------------------------------------------------------------------------------------------------------------

all: check_dirs	
# First be sure the auto geneated files are up to date
	@make -s scl
# Check for all defines to be set and tools	
	@make -s check_defines
	@make -s checktools 	
	@echo "Please be sure to use relative path names for WINDOWS/CYGIN"
	@# Now call this file again so all auto generated files are correctly included	
	@make -s -k -f $(RELATIVE_SOURCE_PATH)/source/kernel.mk build
	
build:
	@echo "----------------------------------"
	@echo "     Building Modules"
	@echo "----------------------------------"
	@make -s modules
	@echo "----------------------------------"
	@echo "     Building the Kernel"
	@echo "----------------------------------"
	@make -s  $(OUTPUT_DIR)kernel.elf
	@make -s binary 
	@make -s size
#	@make -s uImage
#	@echo All done .. Images can be found inside the output directory..


#---------------------------------------------------------------------------------------------------------------------------------------
#                        Target: clean
#---------------------------------------------------------------------------------------------------------------------------------------

clean: tasks_clean
	@echo "----------------------------------"
	@echo "     Cleaning Kernel"
	@echo "----------------------------------"
	@echo kernel.mk: Cleaning up...
ifdef MODULES_DIR
	@echo "Removing modules"
	@$(RM) $(MODULES_DIR)/*
endif
ifdef OUTPUT_DIR
	@echo "Removing kernel objects"
	@$(RM) $(OUTPUT_DIR)/*
endif
ifdef XMD_DIR
	@$(RM) $(XMD_DIR)/*
endif
ifdef BDI_DIR
	@$(RM) $(BDI_DIR)/* 
endif
	@$(RM) make/SCLConfig.hh make/logger_config.hh make/tasktable.S make/scl_make.mk


#---------------------------------------------------------------------------------------------------------------------------------------
#                        Target: uImage : Create UBOOT compatible boot image 
#---------------------------------------------------------------------------------------------------------------------------------------

uImage: SCLConfig.hh tasks $(OUTPUT_DIR)kernel.elf $(OUTPUT_DIR)kernel.bin
	@echo "----------------------------------"
	@echo "      Building the uImage"
	@echo "----------------------------------"
	@sh $(RELATIVE_SOURCE_PATH)/source/uImage.sh $(UIMAGE_ARCH)

#---------------------------------------------------------------------------------------------------------------------------------------
#                         				Compile rules
#---------------------------------------------------------------------------------------------------------------------------------------

# Assemble: create object files from assembler source files. 
$(OUTPUT_DIR)%.o : %.S SCLConfig.hh
	@echo "kernel.mk[ASM]: Assembling $@"
	@$(CC) $(ASFLAGS) $< --output $@
		
#rule for creating the configuration header file SCLConfig.hh and the tasktable by unsing the scl tool	
SCLConfig.hh tasktable.S: SCLConfig.xml $(RELATIVE_SOURCE_PATH)/source/SCLdependencies.xml
	@echo ----------------- SCLTool -------------------
	@echo kernel.mk: Reading config file $<
	@ if $(SCL); then echo Configuration valid! Files created for compilation.; else exit 1; fi 
	@echo ---------------------------------------------
	@echo

scl_gen:
	@echo ----------------- SCLTool -------------------
	@echo kernel.mk: Generating SCL Dependency file $<	
	@ if $(SCL_GEN); then echo SCL Dependency file created!; else exit 1; fi 
	@echo ---------------------------------------------
	@echo

scl: SCLConfig.hh 


#rule for compiling c++ files with header			
$(OUTPUT_DIR)%.o : %.cc %.hh SCLConfig.hh
	@echo "kernel.mk[C++]: Compiling  $@"
	@$(CXX) $(CPFLAGS) $(OPT_FLAGS)  $< --output $@	
	if [ $(CHECKSTYLE) -eq 1 ]; then python2.7 $(KERNEL_DIR)cpplint.py --extensions=cc,hh,c,h --filter=$(CPPLINT_FILTER) $< $(patsubst %.cc, %.hh, $<); fi
	if [ $(CPPCHECK) -eq 1 ]; then cppcheck --enable=warning,style,performance --quiet --force --inline-suppr $(CPP_CHECK_INCLUDES)  $<; fi

$(MODULES_DIR)%.o : %.cc %.hh SCLConfig.hh $(OUTPUT_DIR)syscall.o
	@echo mkmodules[C++]: Compiling $@	 
	if ! [ `less $< | grep ORCOS_MODULE_INIT` ]; then { echo "ERROR: $< is no valid ORCOS module"; exit 3; }; fi
	@$(CXX) $(CPFLAGS) $(OPT_FLAGS) -std=c++0x -DMODULE_IN_USERSPACE=1 $< --output $@
	@echo mkmodules[C++]: Linking   $@
	$(SED) -f $@.sed $(KERNEL_DIR)/arch/module.ld > $@.ld
	$(LD) $@ $(OUTPUT_DIR)syscall.o --script=$@.ld -o $@.elf
	$(SIZE) $@.elf
	$(OBJDUMP) -h $@.elf > $@.sections
	

#rule for compiling c++ files without header
$(OUTPUT_DIR)%.o : %.cc SCLConfig.hh
	@echo "kernel.mk[C++]: Compiling  $@"
	@$(CXX) $(CPFLAGS) $(OPT_FLAGS)  $< --output $@
	if [ $(CHECKSTYLE) -eq 1 ]; then python2.7 $(KERNEL_DIR)cpplint.py --extensions=cc,hh,c,h --filter=$(CPPLINT_FILTER) $<; fi
	if [ $(CPPCHECK) -eq 1 ]; then cppcheck --enable=warning,style,performance --quiet --force --inline-suppr $(CPP_CHECK_INCLUDES)  $<; fi

#rule for compiling c files with header	
$(OUTPUT_DIR)%.o : %.c %.h SCLConfig.hh
	@echo "kernel.mk[C  ]: Compiling  $@"
	@$(CC) $(CFLAGS) $(OPT_FLAGS) $< --output $@	
	if [ $(CHECKSTYLE) -eq 1 ]; then python2.7 $(KERNEL_DIR)cpplint.py --extensions=cc,hh,c,h --filter=$(CPPLINT_FILTER) $< $(patsubst %.c, %.h, $<); fi
	if [ $(CPPCHECK) -eq 1 ]; then cppcheck --enable=warning,style,performance --quiet --force --inline-suppr $(CPP_CHECK_INCLUDES)  $<; fi
	
#rule for compiling c files without header
$(OUTPUT_DIR)%.o : %.c SCLConfig.hh
	@echo "kernel.mk[C  ]: Compiling  $@"
	@$(CC) -c $(CFLAGS) $(OPT_FLAGS) $< --output $@	
	if [ $(CHECKSTYLE) -eq 1 ]; then python2.7 $(KERNEL_DIR)cpplint.py --extensions=cc,hh,c,h --filter=$(CPPLINT_FILTER) $<; fi
	if [ $(CPPCHECK) -eq 1 ]; then cppcheck --enable=warning,style,performance --quiet --force --inline-suppr $(CPP_CHECK_INCLUDES)  $<; fi

# Create static library of the OS so the linker only links the 
# really used classes into the binary
PLUGIN_DIR=$(shell $(CC) -print-prog-name=liblto_plugin.so)
$(OUTPUT_DIR)liborcoskernel.a : $(BUILD_OBJ)
	@echo
	@echo Creating static library liborcoskernel.a	
	@$(RM) $(OUTPUT_DIR)liborcoskernel.a	
	@$(AR) qc $(OUTPUT_DIR)liborcoskernel.a $(BUILD_OBJ) --plugin $(PLUGIN_DIR)


modules: $(MODULES)
	@echo Modules build..

#---------------------------------------------------------------------------------------------------------------------------------------
#                         				Kernel Linking 
#---------------------------------------------------------------------------------------------------------------------------------------
SIGN 	= java -jar $(RELATIVE_SOURCE_PATH)/tools/SCL/dist/sn.jar

#final linking rule		
$(OUTPUT_DIR)kernel.elf: output/tasktable.o $(OUTPUT_DIR)liborcoskernel.a 
	@echo
	@echo kernel.mk[LD] : Linking $@	
	@echo "#include <debug/dwarf.hh>" > $(OUTPUT_DIR)kernel_strtable.c
	@echo "DebugStrTableT __attribute__((used)) debug_strtable[] = { { 0, 0xffffffff, 0 } }; unsigned int debug_strtable_entries = 0;" >> $(OUTPUT_DIR)kernel_strtable.c
	@$(CC) -c $(CFLAGS) $(OPT_FLAGS)   $(OUTPUT_DIR)kernel_strtable.c --output ./output/kernel_strtable.o
	$(CC) -L$(OUTPUT_DIR) output/tasktable.o $(OUTPUT_DIR)liborcoskernel.a $(OUTPUT_DIR)kernel_strtable.o $(LDFLAGS)
	
	#@$(CC) -c $(CFLAGS) $(OPT_FLAGS) -fno-lto $(RELATIVE_SOURCE_PATH)/source/debug/dwarf.cc --output ./output/dwarf.o
	#$(CC) -L$(OUTPUT_DIR) output/tasktable.o $(OUTPUT_DIR)liborcoskernel.a ./output/dwarf.o $(OUTPUT_DIR)kernel_strtable.o $(LDFLAGS)
	$(OBJDUMP) -h $(OUTPUT_DIR)kernel.elf > $(OUTPUT_DIR)kernel.sections
	$(SIGN) stringtable $(OUTPUT_DIR)kernel.map
	#@cp $(OUTPUT_DIR)kernel.map $(OUTPUT_DIR)kernel.map_before
	@$(CC) -c $(CFLAGS) $(OPT_FLAGS)   $(OUTPUT_DIR)kernel_strtable.c --output ./output/kernel_strtable.o
	$(CC) -L$(OUTPUT_DIR) output/tasktable.o $(OUTPUT_DIR)liborcoskernel.a $(OUTPUT_DIR)kernel_strtable.o $(LDFLAGS)
	#$(CC) -L$(OUTPUT_DIR) output/tasktable.o $(OUTPUT_DIR)liborcoskernel.a  ./output/dwarf.o $(OUTPUT_DIR)kernel_strtable.o $(LDFLAGS)
	$(SIGN) stringtable $(OUTPUT_DIR)kernel.map
	#@cp $(OUTPUT_DIR)kernel.map $(OUTPUT_DIR)kernel.map_before2
	@$(CC) -c $(CFLAGS) $(OPT_FLAGS)   $(OUTPUT_DIR)kernel_strtable.c --output ./output/kernel_strtable.o
	$(CC) -L$(OUTPUT_DIR) output/tasktable.o $(OUTPUT_DIR)liborcoskernel.a  $(OUTPUT_DIR)kernel_strtable.o $(LDFLAGS)	
	$(SIZE) $(OUTPUT_DIR)liborcoskernel.a  > $(OUTPUT_DIR)sizes.txt
		

	
#no libc (-lc) linking since this increases the memory footprint by about 660 bytes
# in order to use --gc-sections we need to be sure to always include interrupt vectors a.s.o

#---------------------------------------------------------------------------------------------------------------------------------------
#                         				Other rules 
#---------------------------------------------------------------------------------------------------------------------------------------

tasktable:
	$(SIGN) stringtable $(OUTPUT_DIR)kernel.map

size: $(OUTPUT_DIR)kernel.elf
	@$(SIZE) $(OUTPUT_DIR)kernel.elf
	
$(OUTPUT_DIR)kernel.bin: $(OUTPUT_DIR)kernel.elf
	@$(OBJCOPY) -O binary $(OUTPUT_DIR)kernel.elf $(OUTPUT_DIR)kernel.bin

binary: $(OUTPUT_DIR)kernel.bin


#---------------------------------------------------------------------------------------------------------------------------------------
#                         				Initial Tasks Make rules 
#
# Tasks build here will be contained inside the uImage boot image and started directly after startup
#---------------------------------------------------------------------------------------------------------------------------------------

-include make/tasks.in

tasks: scl 
	@echo "----------------------------------"
	@echo "     Building Tasks"
	@echo "----------------------------------"
	@n=0 ; \
	starts=$(TASKS_START); \
	sizes=$(TASKS_SIZE); \
	periods=$(TASKS_PERIOD); \
	priorities=$(TASKS_PRIORITY); \
	phases=$(TASKS_PHASE); \
	deadlines=$(TASKS_DEADLINE); \
	exectime=$(TASKS_EXECTIME); \
	for x in $(TASKS); do\
		echo Creating $$x; \
		make -s -C $$x OUTPUTFILE=task.image TARGET=$(TARGET) TASK_START=$${starts[$$n]} TASK_SIZE=$${sizes[$$n]} PHASE=$${phases[$$n]} INITIAL_PRIORITY=$${priorities[$$n]} PERIOD=$${periods[$$n]} DEADLINE=$${deadlines[$$n]} EXECUTIONTIME=$${exectime[$$n]} all; \
		$(CP) $$x/$(TARGET)/task.image tasks/task$$n.image; \
		$(CP) $$x/$(TARGET)/task.image.elf tasks/task$$n.image.elf; \
		let "n+=1" ; \
	done;
	
	
tasks_clean: 
	@echo "----------------------------------"
	@echo "     Cleaning Tasks"
	@echo "----------------------------------"
	@for i in $(TASKS); do echo Cleaning $$i; make -s -C $$i TASK_DIR="$$i" clean; done
	@rm -f tasks/*

