#.SUFFIXES:
#.SUFFIXES:      .S .c .o .cof .eep .hex .cc

#kernel.mk contains the logic to compile the operating system. 
#It should only be changed, if the kernel is modified or the build process 
#should be changed in a general way. 
#This file must not be copy and pasted for different purposes.

#To change parameters and settings for a single configuration, 
#the Makefile in the configuration directory should be used.

-include make/scl_make.mk

-include $(ARCH_DIR)/arch.mk

KOBJ 	= $(notdir $(KASRC:.S=.o)) $(ARCH_OBJ)
OBJ = $(addprefix $(OUTPUT_DIR),$(KOBJ))

MODULES = $(addprefix $(MODULES_DIR),$(MODULE_OBJ))

# KOBJ and ARCH_OBJ specify the object-files used for linking.
# For every .cc, .c or .S file there must be a corresponding entry in the KOBJ or ARCH_OBJ list
# KOBJ specifies kernel-objects which are independent of the target archeticeture, 
# while ARCH_OBJ are objects which are specific to a given architecture. 
# It would make sense to exclude the latter list in an own file, as soon as more 
# than one platform is supported.

#TODO: let the scl configuration tool generate the list of objects to create!
#config dir
KOBJ += tasktable.o __cxa_pure_virtual.o

#protocols
#KOBJ += ARP.o SimpleTransportProtocol.o SimpleAddressProtocol.o IPv4AddressProtocol.o UDP.o
KOBJ += ProtocolPool.o TCPTransportProtocol.o IPv4AddressProtocol.o


KOBJ += etharp.o ethernet.o ip.o init.o mem.o memp.o pbuf.o  netif.o udp.o tcp.o sys.o inet.o ip4_addr.o ip4.o ip4_frag.o tcp_in.o tcp_out.o stats.o icmp.o ip6_addr.o ip6.o

KOBJ += icmp6.o ethar.o ethndp.o lwipTMR.o 

KOBJ += WorkerThread.o WorkerTask.o SNServiceDiscovery.o crc32.o

#bluetooth
#KOBJ += HCI.o

#EHCI
KOBJ+=USBEHCIHostController.o

#socket
KOBJ += Socket.o CAB.o  

#db
KOBJ += ArrayDatabase.o LinkedListDatabase.o

#debug
KOBJ += Logger.o Trace.o ETHLogger.o

#filesystem
KOBJ += File.o Directory.o Filemanager.o Resource.o SimpleFileManager.o PartitionManager.o DOSPartition.o FileSystemBase.o FATFileSystem.o SharedMemResource.o

#procfs
KOBJ += SimpleProcfs.o SimpleDebugCollector.o

#hal
KOBJ += PowerManager.o CharacterDeviceDriver.o BlockDeviceDriver.o TimerDevice.o CommDeviceDriver.o USCommDeviceDriver.o Clock.o

#usb
KOBJ += USBDriverLibrary.o USBDeviceDriverFactory.o SMSC95xxUSBDeviceDriver.o MassStorageSCSIUSBDeviceDriver.o

#inc
#__udivdi3.o
KOBJ += stringtools.o memtools.o sprintf.o putc.o libgccmath.o endian.o

#inc/newlib/
KOBJ += newlib_helper.o

#kernel
KOBJ += kwait.o kernelmain.o Kernel.o 

#mem
KOBJ += LinearMemManager.o new.o SequentialFitMemManager.o PagedRamMemManager.o

#process
KOBJ += IdleThread.o Task.o Thread.o PriorityThread.o TaskManager.o RealTimeThread.o Module.o

#scheduler
KOBJ += RoundRobinThreadScheduler.o PriorityThreadScheduler.o RateMonotonicThreadScheduler.o EarliestDeadlineFirstThreadScheduler.o SingleCPUDispatcher.o 

#robustness
KOBJ += TaskErrorHandler.o 

#synchro
KOBJ += Mutex.o 

#syscall
KOBJ += sc_common.o sc_io.o sc_mem.o sc_net.o sc_process.o sc_synchro.o

#Every directory containing source code must be specified in KERNEL_PATH or ARCH_VPATH, 
#depending on whether they are common kernel code or specific to the archectiture.
VPATH = $(addprefix $(KERNEL_DIR), $(KERNEL_VPATH)) $(ARCH_VPATH) make/


#LWIP_VPATH = comm/lwip/core/ comm/lwip/core/ipv4/ comm/lwip/netif/ comm/lwip/arch/ comm/lwip/include/
KERNEL_VPATH = db/ debug/ filesystem/ hal/ inc/ inc/newlib/ kernel/ robust/ mem/ process/ scheduler/ syscalls/ synchro/ comm/ migration/ comm/servicediscovery/ comm/lwip/core/ comm/lwip/core/ipv4/ comm/lwip/netif/ comm/lwip/arch/ comm/lwip/core/ipv6/ comm/hcibluetooth/ arch/shared/ arch/shared/usb
#$(LWIP_VPATH) 

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
	$(error KERNEL_LIB_DIR is undefined)
endif
ifndef KERNEL_DIR
	$(error KERNEL_DIR is undefined)
endif


check_dirs:
	@if ! [ -e $(OUTPUT_DIR) ]; then mkdir $(OUTPUT_DIR); fi
	@if ! [ -e $(XMD_DIR) ]; then mkdir $(XMD_DIR); fi
	@if ! [ -e $(BDI_DIR) ]; then mkdir $(BDI_DIR); fi	
	@if ! [ -e $(MODULES_DIR) ]; then mkdir $(MODULES_DIR); fi

all: check_dirs scl check_defines			
# build all dependencies
	@make -s checktools 
	@make -s scl 
	@echo "Please be sure to use relative path names for WINDOWS/CYGIN"
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
	@make -s uImage
	@echo All done .. Images can be found inside the output directory..

uImage: scl tasks $(OUTPUT_DIR)kernel.elf $(OUTPUT_DIR)kernel.bin
	@echo "----------------------------------"
	@echo "      Building the uImage"
	@echo "----------------------------------"
	/bin/bash $(RELATIVE_SOURCE_PATH)/source/uImage.sh $(UIMAGE_ARCH)

checktools:
	@echo
	@if ! [ -e $(ARCH_DIR) ]; then echo "ERROR: ARCH_DIR does not exist!"; exit -1; fi
	@if ! [ -e $(ARCH_DIR)/arch.mk ]; then echo "ERROR: no arch.mk found in $(ARCH_DIR)!"; exit -1; fi
	@if ! [ -e $(KERNEL_LIB_DIR) ]; then echo "ERROR: KERNEL_LIB_DIR does not exist!"; exit -1; fi
	@if ! [ -e $(KERNEL_DIR) ]; then echo "ERROR: KERNEL_DIR does not exist!"; exit -1; fi
	@echo "----------------------------------"
	@echo "     Checking for tools"
	@echo "----------------------------------"
	@if ! [ -e $(CC) ]; then { echo "C-Compiler (CC) $(CC) not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(CXX) ]; then { echo "C++-Compiler (CXX) $(CXX) not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(AS) ]; then { echo "Assembler (AS) $(AS) not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(AR) ]; then { echo "Archiver (AR) $(AR) not found! Stopping!"; exit 1;} fi
	@if ! java -version >> /dev/null; then { echo "Java not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(OBJCOPY) ]; then { echo "Object-Copy (OBJCOPY) $(OBJCOPY) not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(GCC_LIB_DIR) ]; then { echo "GCC_LIB_DIR $(GCC_LIB_DIR) not found! Stopping!"; exit 1;} fi
	@echo All tools installed...
	@echo 
	
clean: tasks_clean
	@echo "----------------------------------"
	@echo "     Cleaning Kernel"
	@echo "----------------------------------"
	@echo kernel.mk: cleaning up...
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
	$(RM) make/SCLConfig.hh make/logger_config.hh make/tasktable.S make/scl_make.mk
ifdef KERNEL_DIR
	$(RM) $(KERNEL_DIR)lib/*.o $(KERNEL_DIR)lib/*.a
endif
	@for i in $(TASKS); do rm -f $$i/task.sed; done

# Assemble: create object files from assembler source files. 
$(OUTPUT_DIR)%.o : %.S SCLConfig.hh
	@echo kernel.mk[AS] : Assembling $@
	@$(CC) $(ASFLAGS) $< --output $@
		
#rule for creating the configuration header file SCLConfig.hh and the tasktable by unsing the scl tool	
SCLConfig.hh tasktable.S: SCLConfig.xml
	@echo ----------------- SCLTool -------------------
	@echo kernel.mk: Reading config file $<
	@ if $(SCL); then echo Configuration valid! Files created for compilation.; else exit 1; fi 
	@echo ---------------------------------------------
	@echo

#rule for compiling c++ files with header			
$(OUTPUT_DIR)%.o : %.cc %.hh SCLConfig.hh
	@echo kernel.mk[C++]: Compiling  $@
	@$(CXX) $(CPFLAGS) $(OPT_FLAGS)  $< --output $@

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
	@echo kernel.mk[C++]: Compiling  $@
	@$(CXX) $(CPFLAGS) $(OPT_FLAGS)  $< --output $@

#rule for compiling c files with header	
$(OUTPUT_DIR)%.o : %.c %.h SCLConfig.hh
	@echo kernel.mk[C]   : Compiling  $@
	@$(CC) $(CFLAGS) $(OPT_FLAGS)  $< --output $@
	
#rule for compiling c files without header
$(OUTPUT_DIR)%.o : %.c SCLConfig.hh
	@echo kernel.mk[C]   : Compiling  $@
	@$(CC) -c $(CFLAGS) $(OPT_FLAGS)  $< --output $@

$(OUTPUT_DIR)syscall.o : syscall.S SCLConfig.hh
	@echo Syscall  : Compiling  $@
	@$(CC) -c $(CPFLAGS) $(USER_LIB_OPT_FLAGS)  $< --output $@

# Create static library of the OS so the linker only links the 
# really used classes into the binary
$(OUTPUT_DIR)liborcoskernel.a : $(OBJ)
	@echo
	@echo Creating static library liborcoskernel.a
	@$(RM) $(OUTPUT_DIR)liborcoskernel.a
	@$(AR) qc $(OUTPUT_DIR)liborcoskernel.a $(OBJ)

#rule for compiling the user orcos library
$(KERNEL_DIR)lib/%.o : $(KERNEL_DIR)lib/%.cc SCLConfig.hh
	@echo Compiling the user library: $@
# be sure to use optimization for the library otherwise syscalls wont work!
	@$(CXX) $(CPFLAGS) $(USER_LIB_OPT_FLAGS) $< --output $@

#rule for compiling the user orcos library
$(KERNEL_DIR)lib/%.o : $(KERNEL_DIR)lib/%.c SCLConfig.hh
	@echo Compiling the user library: $@
# be sure to use optimization for the library otherwise syscalls wont work!
	@$(CC) $(CFLAGS) $(USER_LIB_OPT_FLAGS) $< --output $@

USERLIB_OBJS_ = static.o threads.o mem.o io.o string.o net.o Mutex.o pthread.o signal.o
USERLIB_OBJS = $(addprefix $(KERNEL_DIR)lib/, $(USERLIB_OBJS_))
USERLIB_OBJS += $(OUTPUT_DIR)syscall.o $(OUTPUT_DIR)testandset.o 
USERNEWLIB_OBJS_ = static.o libgloss.o
USERNEWLIB_OBJS = $(addprefix $(KERNEL_DIR)lib/, $(USERNEWLIB_OBJS_))
USERNEWLIB_OBJS += $(OUTPUT_DIR)syscall.o

#Make liborcos.a
$(KERNEL_DIR)lib/liborcos.a: $(USERLIB_OBJS)
	@echo "----------------------------------"
	@echo "     Building the User Library"
	@echo "----------------------------------"
	@make -s $(USERLIB_OBJS)
	@echo Archiving the user library into liborcos.a
	@$(AR) qc $@ $(USERLIB_OBJS)
	@echo 

$(KERNEL_DIR)lib/liborcos_newlib.a: $(USERNEWLIB_OBJS)
	@echo "------------------------------------"
	@echo "Building the User Library for newlib"
	@echo "------------------------------------"
	@make -s $(USERNEWLIB_OBJS)
	@echo Archiving the user library into liborcos_newlib.a
	@$(AR) rc $@ $(USERNEWLIB_OBJS)
	@echo

modules: $(MODULES)
	@echo Modules build..

lib:
	@if ! [ -e $(OUTPUT_DIR) ]; then mkdir $(OUTPUT_DIR); fi 
	@make -s scl $(KERNEL_DIR)lib/liborcos.a $(KERNEL_DIR)lib/liborcos_newlib.a

#final linking rule		
$(OUTPUT_DIR)kernel.elf: output/_startup.o output/tasktable.o $(OUTPUT_DIR)liborcoskernel.a 
	@echo
	@echo kernel.mk[LD] : Linking $@
	$(LD) -L$(OUTPUT_DIR) output/_startup.o output/tasktable.o -lorcoskernel  $(LDFLAGS) 
	$(OBJDUMP) -h $(OUTPUT_DIR)kernel.elf > $(OUTPUT_DIR)kernel.sections
#no libc (-lc) linking since this increases the memory footprint by about 660 bytes

#rule for creation of the configuration header file	
scl: SCLConfig.hh
	
size: $(OUTPUT_DIR)kernel.elf
	@$(SIZE) $(OUTPUT_DIR)kernel.elf
	
$(OUTPUT_DIR)kernel.bin: $(OUTPUT_DIR)kernel.elf
	@$(OBJCOPY) -O binary $(OUTPUT_DIR)kernel.elf $(OUTPUT_DIR)kernel.bin

binary: $(OUTPUT_DIR)kernel.bin

tasks: scl lib
	@echo "----------------------------------"
	@echo "     Building Tasks"
	@echo "----------------------------------"
	@for i in $(TASKS); do echo Creating $$i; make -s -C $$i CC=$(CC) CXX=$(CXX) AS=$(AS) LD=$(LD) OBJCOPY=$(OBJCOPY) OBJDUMP=$(OBJDUMP) SED=$(SED) KERNEL_LIB_DIR=$(KERNEL_LIB_DIR) KERNEL_DIR=$(KERNEL_DIR) GCC_LIB_DIR=$(GCC_LIB_DIR) STACK_USAGE_SCRIPT=$(STACK_USAGE_SCRIPT) CFLAGS_TASKS=$(CFLAGS_TASKS) LDFLAGS_TASKS="$(LDFLAGS_TASKS)" TASK_DIR="$$i" ; echo ; done
	
tasks_clean: 
	@echo "----------------------------------"
	@echo "     Cleaning Tasks"
	@echo "----------------------------------"
	@for i in $(TASKS); do echo Cleaning $$i; make -s -C $$i TASK_DIR="$$i" clean; done


bdilocal: all tasks_clean tasks
	@echo "----------------------------------"
	@echo "     Loading to Target BDI"
	@echo "----------------------------------"
	@python $(BDICOMMAND) $(BDIADRESS) $(BDIPORT) "reset"
	@sleep 1
# first copy and load the tasks
	@for i in $(TASKS); do make -s -C $$i bdilocal BDICOMMAND=$(BDICOMMAND) BDIADRESS=$(BDIADRESS) BDIPORT=$(BDIPORT) BDI_DIR=$(BDI_DIR); done
# now copy and load the kernel
	@echo Loading Kernel to BDI Target
	$(RM) $(BDI_DIR)/kernel.elf
	$(CP) $(OUTPUT_DIR)/kernel.elf $(BDI_DIR)/kernel.elf
	@python $(BDICOMMAND) $(BDIADRESS) $(BDIPORT) "load" "load kernel.elf"
	
bdi: all tasks
	@echo "----------------------------------"
	@echo "     Copying to Target BDI"
	@echo "----------------------------------"
# first copy and load the tasks
	@echo $(TASKS)
	@for i in $(TASKS); do cp $$i/*.elf $(BDI_DIR)/; done
# now copy and load the kernel
	@echo Copying Kernel to BDI directory
	$(CP) $(OUTPUT_DIR)/kernel.elf $(BDI_DIR)/
	#$(CP) $(OUTPUT_DIR)/kernel.bin $(BDI_DIR)/
	$(CP) $(BDI_CONFIG) $(BDI_DIR)/
	$(CP) $(BDI_REGDEF) $(BDI_DIR)/	


xmdlocal: all tasks_clean tasks
	@echo "----------------------------------"
	@echo "     Copying to Target XMD"
	@echo "----------------------------------"
# first copy and load the tasks
	@for i in $(TASKS); do make -s -C $$i xmdlocal XMD_DIR=$(XMD_DIR); done
# now copy and load the kernel
	@echo Copying Kernel to XMD directory
	$(RM) $(XMD_DIR)/kernel.elf
	$(CP) $(OUTPUT_DIR)/kernel.elf $(XMD_DIR)/kernel.elf
	$(CP) xmd.ini $(XMD_DIR)/xmd.ini	
	
xmd: all tasks
	@echo "----------------------------------"
	@echo "     Copying to Target XMD"
	@echo "----------------------------------"
# first copy and load the tasks
	@for i in $(TASKS); do make -s -C $$i xmd XMD_DIR=$(CWD)/$(XMD_DIR); done
# now copy and load the kernel
	@echo Copying Kernel to XMD directory
	$(CP) $(OUTPUT_DIR)/kernel.elf $(XMD_DIR)/
#$(CP) $(OUTPUT_DIR)/kernel.bin $(XMD_DIR)/
	$(CP) xmd.ini $(XMD_DIR)/
