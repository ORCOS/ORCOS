
ifeq ($(TARGET),armv7)
-include $(ORCOS_LIB_DIR)/armv7.mk
endif

ifeq ($(TARGET),ppc405)
-include $(ORCOS_LIB_DIR)/ppc405.mk
endif	

ifeq ($(TARGET),sparc)
-include $(ORCOS_LIB_DIR)/sparc.mk
endif	

ifeq ($(TARGET),x86)
-include $(ORCOS_LIB_DIR)/x86.mk
endif	

# Get all C/c++ files to build
OBJS=$(wildcard *.c) $(wildcard *.cc) $(wildcard *.cpp)
OBJS2=$(OBJS:.cc=.o)
OBJS3=$(OBJS2:.c=.o) 
OBJS4=$(OBJS3:.cpp=.o)  

# we build all files in the platform subdirectory
BUILD_OBJS =  $(addprefix $(BUILD_PLATFORM)/, $(OBJS4) )

STACK_USAGE = python $(STACK_USAGE_SCRIPT)
  
SIGN 	= java -jar $(ORCOS_LIB_DIR)/../tools/SCL/dist/sn.jar
  
  
# additional header generation part
# TODO: add CRC support
NEXT_HEADER = 0x0
ADDITIONAL_HEADER = 

ifndef PHASE
PHASE = 0
endif

ifndef INITIAL_PRIORITY
INITIAL_PRIORITY = 1
endif

ifndef PERIOD
PERIOD = 0
endif

ifndef DEADLINE
DEADLINE = 0
endif

ifndef EXECUTIONTIME
EXECUTIONTIME = 0
endif

ifndef STACK_SIZE
STACK_SIZE = 4096
endif

ifndef TASK_HEAP_ALIGN
TASK_HEAP_ALIGN=256
endif

ifndef TASK_START
TASK_START = 0x100000
endif

ifndef TASK_SIZE
TASK_SIZE = 0x100000
endif

ifndef TASK_END
TASK_END = $$(( 0x100000 + $(TASK_SIZE) -1 )) 
endif


ifeq ($(BUILD_CRC32),1)
NEXT_HEADER = 1
ADDITIONAL_HEADER = LONG(0); LONG(CRC_START); LONG(CRC_END); LONG(-1);	
endif

crc:
ifeq ($(BUILD_CRC32),1)
	@echo "Generating Checksum"
	$(SIGN) crc $(BUILD_PLATFORM)/$(OUTPUTFILE)

endif

check_java:
	@if ! java -version > /dev/null 2> /dev/null; then { echo "Java not found! Stopping! Be sure the PATH variable contains a valid path to a java installation."; exit 1;} fi


checktools:
	@if ! [ -e $(ORCOS_LIB_DIR) ]; then echo "ERROR: ORCOS_LIB_DIR does not exist! $(ORCOS_LIB_DIR)"; exit -1; fi
	@echo -ne "Checking for tools: "
	@if ! [ -e $(CC) ]; then { echo "C-Compiler (CC) $(CC) not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(CXX) ]; then { echo "C++-Compiler (CXX) $(CXX) not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(AS) ]; then { echo "Assembler (AS) $(AS) not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(AR) ]; then { echo "Archiver (AR) $(AR) not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(OBJCOPY) ]; then { echo "Object-Copy (OBJCOPY) $(OBJCOPY) not found! Stopping!"; exit 1;} fi
	@if ! [ -e $(GCC_LIB_DIR) ]; then { echo "GCC_LIB_DIR $(GCC_LIB_DIR) not found! Stopping!"; exit 1;} fi
	@echo OK

prepare:
	@mkdir -p $(BUILD_PLATFORM)
	@cp -f $(ORCOS_LIB_DIR)/task.ld $(BUILD_PLATFORM)/template.ld
	@echo "Building '$(BUILD_PLATFORM)/$(OUTPUTFILE)'" 
	@echo "Task Start     : $(TASK_START)"
	@echo "Task End       : $(shell printf 0x"%x" $(TASK_END))"
	@echo "Task Size      : $(TASK_SIZE)"	
	@echo "s/TASK_VMA/0x100000/" 					> $(BUILD_PLATFORM)/task.sed
	@echo "s/TASK_START/$(TASK_START)/" 			>> $(BUILD_PLATFORM)/task.sed
	@echo "s/TASK_HEAP_ALIGN/$(TASK_HEAP_ALIGN)/" 	>> $(BUILD_PLATFORM)/task.sed 	
	@echo "s/TASK_END/$(TASK_END)/" 				>> $(BUILD_PLATFORM)/task.sed
	@echo "s/INITIAL_PRIORITY/$(INITIAL_PRIORITY)/" >> $(BUILD_PLATFORM)/task.sed
	@echo "s/PHASE/$(PHASE)/" 						>> $(BUILD_PLATFORM)/task.sed
	@echo "s/PERIOD/$(PERIOD)/" 					>> $(BUILD_PLATFORM)/task.sed
	@echo "s/DEADLINE/$(DEADLINE)/" 				>> $(BUILD_PLATFORM)/task.sed
	@echo "s/EXECUTIONTIME/$(EXECUTIONTIME)/" 		>> $(BUILD_PLATFORM)/task.sed
	@echo "s/DEADLINE/$(STACK_SIZE)/" 				>> $(BUILD_PLATFORM)/task.sed
	@echo "s/PLATFORM/$(PLATFORM_FLAGS)/" 			>> $(BUILD_PLATFORM)/task.sed
	@echo "s/STACK_SIZE/$(STACK_SIZE)/" 			>> $(BUILD_PLATFORM)/task.sed
	@echo "s/NEXT_HEADER/$(NEXT_HEADER)/" 			>> $(BUILD_PLATFORM)/task.sed
	@echo "s/ADDITIONAL_HEADER/$(ADDITIONAL_HEADER)/" >> $(BUILD_PLATFORM)/task.sed
	@echo
	@$(SED) -f $(BUILD_PLATFORM)/task.sed $(BUILD_PLATFORM)/template.ld > $(BUILD_PLATFORM)/task.ld	
	 
	 
build: $(BUILD_OBJS) $(BUILD_PLATFORM)/$(OUTPUTFILE) crc
	 
all: check_java
	@make -s checktools
	@make -s prepare
	@make -s build
ifdef POST_BUILD
	$(POST_BUILD)
endif 
ifndef KEEP_TEMP_FILES
	@$(RM) $(BUILD_PLATFORM)/*.o
	@$(RM) $(BUILD_PLATFORM)/task.sed
	@$(RM) $(BUILD_PLATFORM)/template.ld
	@$(RM) $(BUILD_PLATFORM)/task.ld
	@$(RM) $(BUILD_PLATFORM)/task.sections
endif
	
binary:
	@$(OBJCOPY) -O binary $(OUTPUTFILE).elf $(OUTPUTFILE).bin
	
stack_usage:
ifneq ($(STACK_USAGE_SCRIPT),)
	@$(STACK_USAGE) $(OBJDUMP) $(OUTPUTFILE).elf task_main
endif
		
$(BUILD_PLATFORM)/$(OUTPUTFILE): $(BUILD_OBJS) $(BUILD_PLATFORM)/task.ld
	@echo -ne "Linking  [$@]"
	@$(CC) -Wl,-Map=$(BUILD_PLATFORM)/$(OUTPUTFILE).map  -Wl,--script=$(BUILD_PLATFORM)/task.ld $(BUILD_OBJS) -o $(BUILD_PLATFORM)/$(OUTPUTFILE).elf -nostartfiles  -L$(KERNEL_LIB_DIR) -L$(GCC_LIB_DIR)  -Wl,--start-group -lorcos -lc -lm -lgcc -Wl,--end-group 
	@$(OBJCOPY) -O binary $(BUILD_PLATFORM)/$(OUTPUTFILE).elf $(BUILD_PLATFORM)/$(OUTPUTFILE)
	@$(OBJDUMP) -h $(BUILD_PLATFORM)/$(OUTPUTFILE).elf > $(BUILD_PLATFORM)/task.sections
	@echo " DONE "
	@$(SIZE) $(BUILD_PLATFORM)/$(OUTPUTFILE).elf

$(BUILD_PLATFORM)/%.o: %.cpp 
	@echo [C++] Building $@
	@$(CXX) $(CPPFLAGS) $< -o $@

$(BUILD_PLATFORM)/%.o: %.c 
	@echo [ C ] Building $@
	@$(CC) $(CFLAGS) $< -o $@
	
$(BUILD_PLATFORM)/%.o: %.cc 
	@echo [C++] Building $@
	@$(CXX) $(CPPFLAGS) $< -o $@

$(BUILD_PLATFORM)/%.o: %.S 
	@echo [ASM] Building $@
	@$(AS) $(AFLAGS) $< -o $@
	
clean:
	@echo Cleaning up Task files
	@$(RM) -R $(BUILD_PLATFORM)
	@$(RM) $(BUILD_OBJS)
	
