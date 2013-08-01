STACK_USAGE = python $(STACK_USAGE_SCRIPT)

OUTPUTFILE = task2

OBJS 	= task_main.o

AFLAGS =
CFLAGS = -c -I $(KERNEL_LIB_DIR) -fno-exceptions -fno-rtti 
CPPFLAGS = $(CFLAGS) -g  
	
all: task2.elf binary stack_usage
	
binary:
	@$(OBJCOPY) -O binary $(OUTPUTFILE).elf $(OUTPUTFILE).bin
	
stack_usage:
ifneq ($(STACK_USAGE_SCRIPT),)
	@$(STACK_USAGE) $(OBJDUMP) $(OUTPUTFILE).elf task_main
endif
		
task2.elf: $(OBJS) task.ld
	@echo task.mk: Linking $(OUTPUTFILE).elf
	@$(LD) --script=task.ld $(OBJS) -o $(OUTPUTFILE).elf -L$(KERNEL_LIB_DIR) -L/opt/arm-none-eabi/lib/gcc/arm-none-eabi/4.4.1 -lorcos -lgcc
	
task.ld: task.sed
	@echo task.mk: Generating linkerscript task.ld
	@$(SED) -f task.sed template.ld >task.ld
	
%.o: %.cpp
	@echo task.mk: Compiling $@
	@$(CXX) $(CPPFLAGS) $< -o $@

%.o: %.c
	@echo task.mk: Compiling $@
	@$(CXX) $(CFLAGS) $< -o $@

%.o: %.cc
	@echo @$(CXX) $(CPPFLAGS) $< -o $@
	@echo task.mk: Compiling $@
	@$(CXX) $(CPPFLAGS) $< -o $@

%.o: %.S
	@echo task.mk: Assembling $@
	@$(AS) $(AFLAGS) $< -o $@
	
clean:
	@echo Cleaning up Task files
	@$(RM) $(OBJS) task.ld $(OUTPUTFILE).elf $(OUTPUTFILE).bin task.sed
	
bdi:
	@echo Loading Task $(OUTPUTFILE) to BDI Target
	@cp $(OUTPUTFILE).elf $(BDI_DIR)/$(OUTPUTFILE).elf
	#@python $(BDICOMMAND) "$(BDIADRESS)" "$(BDIPORT)" "load" "load $(OUTPUTFILE).elf"

xmd:
	@echo Copying Task $(OUTPUTFILE) to XMD directory
	@cp $(OUTPUTFILE).elf $(XMD_DIR)/$(OUTPUTFILE).elf