.SUFFIXES:
.SUFFIXES:      .S .c .o .cof .eep .hex .cc

#kernel.mk contains the logic to compile the operating system. 
#It should only be changed, if the kernel is modified or the build process 
#should be changed in a general way. 
#This file must not be copy and pasted for different purposes.

#To change parameters and settings for a single configuration, 
#the Makefile in the configuration directory should be used.


#Command line arguments to the linker.
#LDFLAGS = -o output/kernel.elf --entry=startORCOS --script=$(LINKERSCRIPT) -L/usr/local/powerpc-eabi/bin/../lib/gcc/powerpc-eabi/${GCC_VERSION}/nof  

#Command line arguments to compile .c files.
#CFLAGS = -g -I$(KERNEL_DIR)inc/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -fno-exceptions -fno-rtti -msoft-float -fno-stack-protector 
#-S = output as assembler files
#CFLAGS += -ftest-coverage -fprofile-arcs

#Command line arguments to compile .cc files.
#CPFLAGS = -g -I$(KERNEL_DIR)inc/  -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -fno-exceptions -fno-rtti -msoft-float -fno-stack-protector -Wuninitialized -Woverloaded-virtual 
#-S
#CPFLAGS += -ftest-coverage -fprofile-arcs

#Command line arguments for the gcc to assemble .S files.
#GCC_ASFLAGS = -g -I$(KERNEL_DIR)inc/ -I. -I$(KERNEL_DIR) $(ARCH_INCLUDES) -fno-exceptions  -fno-rtti -msoft-float -Xassembler -m405

KOBJ 	= $(notdir $(KASRC:.S=.o)) $(ARCH_OBJ)
OBJ = $(addprefix $(OUTPUT_DIR),$(KOBJ))

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
KOBJ += ProtocolPool.o IPv4AddressProtocol.o TCPTransportProtocol.o


KOBJ += etharp.o ethernet.o ip.o init.o mem.o memp.o pbuf.o lwip_ethernetif.o netif.o udp.o tcp.o sys.o inet.o ip4_addr.o ip4.o ip4_frag.o tcp_in.o tcp_out.o stats.o icmp.o ip6_addr.o ip6.o

KOBJ += icmp6.o ethar.o ethndp.o lwipTMR.o

#KOBJ += WorkerThread.o WorkerTask.o SNServiceDiscovery.o
KOBJ += WorkerTask.o SNServiceDiscovery.o

#socket
#KOBJ += Socket.o CAB.o  MigrationManager.o
KOBJ += Socket.o CAB.o 

#db
KOBJ += ArrayDatabase.o LinkedListDatabase.o

#debug
KOBJ += Logger.o Trace.o ETHLogger.o

#filesystem
KOBJ += Directory.o Filemanager.o Resource.o SimpleFileManager.o

#procfs
KOBJ += SimpleProcfs.o SimpleDebugCollector.o

#hal
KOBJ += PowerManager.o CharacterDeviceDriver.o TimerDevice.o CommDeviceDriver.o Clock.o

#inc
#__udivdi3.o
KOBJ += stringtools.o memtools.o sprintf.o putc.o libgccmath.o

#inc/newlib/
#KOBJ += newlib_helper.o

#kernel
KOBJ += error.o kernelmain.o Kernel.o Kernel_initInitialTasks.o

#mem
KOBJ += LinearMemManager.o new.o SequentialFitMemManager.o

#process
KOBJ += IdleThread.o Task.o Thread.o PriorityThread.o TaskManager.o RealTimeThread.o

#scheduler
KOBJ += RoundRobinThreadScheduler.o PriorityThreadScheduler.o RateMonotonicThreadScheduler.o EarliestDeadlineFirstThreadScheduler.o SingleCPUDispatcher.o 

#synchro
KOBJ += Mutex.o 

#syscall
KOBJ += handle_syscalls.o


#Every directory containing source code must be specified in KERNEL_PATH or ARCH_VPATH, 
#depending on whether they are common kernel code or specific to the archectiture.
VPATH = $(addprefix $(KERNEL_DIR), $(KERNEL_VPATH)) $(ARCH_VPATH)
	
#KERNEL_VPATH = db/ debug/ filesystem/ hal/ inc/ inc/newlib/ kernel/ mem/ process/ scheduler/ syscalls/ synchro/ comm/
KERNEL_VPATH = db/ debug/ filesystem/ hal/ inc/ inc/newlib/ kernel/ mem/ process/ scheduler/ syscalls/ synchro/ comm/ migration/ comm/servicediscovery/ comm/lwip/core/ comm/lwip/core/ipv4/ comm/lwip/netif/ comm/lwip/arch/ comm/lwip/core/ipv6/


all: scl lib $(OUTPUT_DIR)kernel.elf binary size

clean: 
	@echo kernel.mk: cleaning up...
	$(RM) $(OUTPUT_DIR)* 
	$(RM) SCLConfig.hh tasks.sh tasks_clean.sh logger_config.hh tasktable.S
	$(RM) $(KERNEL_DIR)syscalls/syscall.o

# Assemble: create object files from assembler source files. 
# As we use c-style preprocesser macros and includes we have to
# use gcc to assemble.
$(OUTPUT_DIR)%.o : %.S SCLConfig.hh
	@echo
#---------------------------------------------------------------------------
	@echo kernel.mk: Assembling $<
#-----------------------------------
# run the preprocessor
#-----------------------------------
	@$(CC) -S $(ASFLAGS) $< > $@.S
#-----------------------------------
# afterburn
#-----------------------------------
	@python afterburn.py $@.S  $@ab.S output/ab_tab.S
#-----------------------------------
# create object file
#-----------------------------------
	@$(CC) -c $(ASFLAGS) $@ab.S --output $@
#---------------------------------------------------------------------------				
		
SCLConfig.hh tasktable.S: SCLConfig.xml
	@echo
	@echo kernel.mk: Reading config file $<
	$(SCL) 

#rule for compiling c++ files with header			
$(OUTPUT_DIR)%.o : %.cc %.hh SCLConfig.hh
	@echo 
	@echo kernel.mk[1]: Compiling $<
#----------------------------------- 
# run the preprocessor
#-----------------------------------
	@$(CXX) -S $(CPFLAGS) -Os $< -o $@.S
	@echo "Assembler: $< --> $@.S"
#-----------------------------------
# afterburn
#-----------------------------------
	@python afterburn.py $@.S  $@ab.S output/ab_tab.S
	@echo "Previrt: $@.S --> $@ab.S"
#-----------------------------------
# create object file
#-----------------------------------
	@$(CXX) -c $(CPFLAGS) -Os $@ab.S --output $@
	@echo "Compile: $@ab.S --> $@"
#---------------------------------------------------------------------------				


#rule for compiling c++ files without header
$(OUTPUT_DIR)%.o : %.cc SCLConfig.hh
	@echo
#---------------------------------------------------------------------------
	@echo kernel.mk[2]: Compiling $<
#-----------------------------------
# run the preprocessor
#-----------------------------------
	@$(CXX) -S $(CPFLAGS) -Os $< -o $@.S
#-----------------------------------
# afterburn
#-----------------------------------
	@python afterburn.py $@.S  $@ab.S output/ab_tab.S
#-----------------------------------
# create object file
#-----------------------------------
	@$(CXX) -c $(CPFLAGS) -Os $@ab.S --output $@
#---------------------------------------------------------------------------				


#rule for compiling c files with header	
$(OUTPUT_DIR)%.o : %.c %.h SCLConfig.hh
	@echo
#---------------------------------------------------------------------------
	@echo kernel.mk[3]: Compiling $<
#-----------------------------------
# run the preprocessor
#-----------------------------------
	@$(CC) -S $(CPFLAGS) -Os $< -o $@.S
	@echo "Assembler: $< --> $@.S"
#-----------------------------------
# afterburn
#-----------------------------------
	@python afterburn.py $@.S  $@ab.S output/ab_tab.S
	@echo "Previrt: $@.S --> $@ab.S"
#-----------------------------------
# create object file
#-----------------------------------
	@$(CC) -c $(CPFLAGS) -Os $@ab.S --output $@
	@echo "Compile: $@ab.S --> $@"
#---------------------------------------------------------------------------	

	
#rule for compiling c files without header
$(OUTPUT_DIR)%.o : %.c SCLConfig.hh
	@echo
#---------------------------------------------------------------------------
	@echo kernel.mk[4]: Compiling $<
#-----------------------------------
# run the preprocessor
#-----------------------------------
	@$(CC) -S $(CPFLAGS) -Os $< -o $@.S
	@echo "Assembler: $< --> $@.S"
#-----------------------------------
# afterburn
#-----------------------------------
	@python afterburn.py $@.S  $@ab.S output/ab_tab.S
	@echo "Previrt: $@.S --> $@ab.S"
#-----------------------------------
# create object file
#-----------------------------------
	@$(CC) -c $(CPFLAGS) -Os $@ab.S --output $@
	@echo "Compile: $@ab.S --> $@"
#---------------------------------------------------------------------------	

	
$(KERNEL_DIR)syscalls/syscall.o : syscall.cc SCLConfig.hh
	@echo Compiling the lib
	$(CC) -c $(CPFLAGS) -Os $< --output $@

# Create static library of the OS so the linker only links the 
# really used classes into the binary
$(OUTPUT_DIR)liborcos.a : $(OBJ)
	@echo
	@echo Creating static library
	$(RM) $(OUTPUT_DIR)liborcos.a
	$(AR) qc $(OUTPUT_DIR)liborcos.a $(OBJ)

#Make the syscall.o for the tasks to include.
lib:	$(KERNEL_DIR)syscalls/syscall.o
		
$(OUTPUT_DIR)kernel.elf: output/_startup.o output/tasktable.o $(OUTPUT_DIR)liborcos.a 
	@echo
	# write final 0x0 entry into afterburn table
	echo "	.long 0x0" >> output/ab_tab.S
	# create afterburn table object file
	$(CC) -c output/ab_tab.S --output output/ab_tab.o
	@echo kernel.mk: Linking $@
	$(LD) $(LDFLAGS) -L$(OUTPUT_DIR) output/_startup.o output/tasktable.o -lorcos -lgcc   output/ab_tab.o
	#no libc (-lc) linking since this increases the memory footprint by about 660 bytes
	
scl: SCLConfig.hh
	touch output/ab_tab.S
	echo   '	.section	.afterburn , "aw", "progbits"' >> output/ab_tab.S
	
size: $(OUTPUT_DIR)kernel.elf
	$(SIZE) $(OUTPUT_DIR)kernel.elf
	
binary:
	$(OBJCOPY) -O binary $(OUTPUT_DIR)kernel.elf $(OUTPUT_DIR)main.bin
	
#tasks: scl lib
#	sh tasks.sh $(MAKE)
	
tasks: scl lib
	@echo "----------------------------------"
	@echo "     Building Tasks"
	@echo "----------------------------------"
	@for i in $(TASKS); do echo Creating $$i; make -s -C $$i CC=$(CC) CXX=$(CXX) AS=$(AS) LD=$(LD) OBJCOPY=$(OBJCOPY) OBJDUMP=$(OBJDUMP) SED=$(SED) KERNEL_LIB_DIR=$(KERNEL_LIB_DIR) KERNEL_DIR=$(KERNEL_DIR) STACK_USAGE_SCRIPT=$(STACK_USAGE_SCRIPT) CFLAGS_TASKS=$(CFLAGS_TASKS) LDFLAGS_TASKS="$(LDFLAGS_TASKS)" TASK_DIR="$$i" ; echo ; done
	
	
tasks_clean: scl
	sh tasks_clean.sh $(MAKE)


