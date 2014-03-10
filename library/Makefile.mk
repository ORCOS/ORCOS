
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

prepare:
	@mkdir -p $(BUILD_PLATFORM)
	@cp -f $(ORCOS_LIB_DIR)/task.ld $(BUILD_PLATFORM)/template.ld
	@echo ""
	@echo "Building '$(OUTPUTFILE)'" 
	@echo "Task Start     : $(TASK_START)"
	@echo "Task End       : $(shell printf 0x"%x" $(TASK_END))"
	@echo "Task Size      : $(TASK_SIZE)"	
	@echo "s/TASK_VMA/0x100000/" > $(BUILD_PLATFORM)/task.sed
	@echo "s/TASK_START/$(TASK_START)/" >> armv7/task.sed
	@echo "s/TASK_HEAP_ALIGN/$(TASK_HEAP_ALIGN)/" >> armv7/task.sed
#@echo -ne "s/TASK_HEAP/" >> $(BUILD_PLATFORM)/task.sed
#@echo -ne $$(( $(TASK_START) + $(TASK_HEAP_SIZE) )) >> $(BUILD_PLATFORM)/task.sed	
#@echo "/" >> $(BUILD_PLATFORM)/task.sed		 	
	@echo "s/TASK_END/$(TASK_END)/" >> armv7/task.sed
	@echo "s/INITIAL_PRIORITY/$(INITIAL_PRIORITY)/" >> $(BUILD_PLATFORM)/task.sed
	@echo "s/PHASE/$(PHASE)/" >> $(BUILD_PLATFORM)/task.sed
	@echo "s/PERIOD/$(PERIOD)/" >> $(BUILD_PLATFORM)/task.sed
	@echo "s/DEADLINE/$(DEADLINE)/" >> $(BUILD_PLATFORM)/task.sed
	@echo "s/EXECUTIONTIME/$(EXECUTIONTIME)/" >> $(BUILD_PLATFORM)/task.sed
	@echo "s/DEADLINE/$(STACK_SIZE)/" >> $(BUILD_PLATFORM)/task.sed
	@echo "s/PLATFORM/$(PLATFORM_FLAGS)/" >> $(BUILD_PLATFORM)/task.sed
	@echo "s/STACK_SIZE/$(STACK_SIZE)/" >> $(BUILD_PLATFORM)/task.sed
	@echo "s/NEXT_HEADER/$(NEXT_HEADER)/" >> $(BUILD_PLATFORM)/task.sed
	@echo "s/ADDITIONAL_HEADER/$(ADDITIONAL_HEADER)/" >> $(BUILD_PLATFORM)/task.sed
	@echo
	@$(SED) -f $(BUILD_PLATFORM)/task.sed $(BUILD_PLATFORM)/template.ld > $(BUILD_PLATFORM)/task.ld	
	 
all: prepare $(BUILD_OBJS) $(BUILD_PLATFORM)/$(OUTPUTFILE) crc
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
	@$(LD) --script=$(BUILD_PLATFORM)/task.ld $(BUILD_OBJS) -o $(BUILD_PLATFORM)/$(OUTPUTFILE).elf -L$(KERNEL_LIB_DIR) -L$(GCC_LIB_DIR) -lorcos -lgcc
	@$(OBJCOPY) -O binary $(BUILD_PLATFORM)/$(OUTPUTFILE).elf $(BUILD_PLATFORM)/$(OUTPUTFILE)
	@$(OBJDUMP) -h $(BUILD_PLATFORM)/$(OUTPUTFILE).elf > $(BUILD_PLATFORM)/task.sections
	@echo " DONE "
	@$(SIZE) $(BUILD_PLATFORM)/$(OUTPUTFILE).elf

$(BUILD_PLATFORM)/%.o: %.cpp 
	@echo Building [$@]
	@$(CXX) $(CPPFLAGS) $< -o $@

$(BUILD_PLATFORM)/%.o: %.c 
	@echo Building [$@]
	@$(CXX) $(CFLAGS) $< -o $@

$(BUILD_PLATFORM)/%.o: %.cc 
	@echo Building [$@]
	@$(CXX) $(CPPFLAGS) $< -o $@

$(BUILD_PLATFORM)/%.o: %.S 
	@echo Building [$@]
	@$(AS) $(AFLAGS) $< -o $@
	
clean:
	@echo Cleaning up Task files
	@$(RM) -R $(BUILD_PLATFORM)
	@$(RM) $(BUILD_OBJS)
	