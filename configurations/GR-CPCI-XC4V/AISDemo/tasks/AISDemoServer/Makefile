STACK_USAGE = python $(STACK_USAGE_SCRIPT)

#GCC Version
GCC_VERSION = 4.3.3

#Location of compiler binaries
COMP_DIR = /opt/xgcc-sparc-elf-$(GCC_VERSION)

OUTPUTFILE = AISDemoServer

OBJS 	= task_main.o map.o turbo.o encoder.o

AFLAGS =
CFLAGS = -c -I $(KERNEL_LIB_DIR) -fno-exceptions -fno-rtti -msoft-float 
CPPFLAGS = $(CFLAGS) -g  -msoft-float 
	
all: task.elf binary stack_usage
	
binary:
	@$(OBJCOPY) -O binary $(OUTPUTFILE).elf $(OUTPUTFILE).bin
	
stack_usage:
ifneq ($(STACK_USAGE_SCRIPT),)
	@$(STACK_USAGE) $(OBJDUMP) $(OUTPUTFILE).elf task_main
endif
		
task.elf: $(OBJS) task.ld
	@echo task.mk: Linking $(OUTPUTFILE).elf
	@echo $(COMP_DIR)/sparc-elf/lib/soft/v8
	@$(LD) --script=task.ld $(OBJS) -o $(OUTPUTFILE).elf  -L $(KERNEL_LIB_DIR) -L $(COMP_DIR)/sparc-elf/lib/soft/v8 -L /opt/xgcc-sparc-elf-4.3.3/lib/gcc/sparc-elf/4.3.3/soft/v8 $(LDFLAGS_TASKS) -lorcos -lm -lgcc -lc
	#@$(LD) --script=task.ld $(OBJS) -o $(OUTPUTFILE).elf -L $(KERNEL_LIB_DIR) $(LDFLAGS_TASKS) -lorcos 
	
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
	@echo task.mk: Compiling $@
	@$(CXX) $(CPPFLAGS) $< -o $@

%.o: %.S
	@echo task.mk: Assembling $@
	@$(AS) $(AFLAGS) $< -o $@
	
clean:
	@echo Cleaning up Task files
	@$(RM) $(OBJS) task.ld $(OUTPUTFILE).elf $(OUTPUTFILE).bin
	
bdilocal:
	@echo Loading Task $(OUTPUTFILE) to BDI Target
	@rm -f $(BDI_DIR)/$(OUTPUTFILE).elf
	@cp $(OUTPUTFILE).elf $(BDI_DIR)/$(OUTPUTFILE).elf
	@python $(BDICOMMAND) "$(BDIADRESS)" "$(BDIPORT)" "load" "load $(OUTPUTFILE).elf"

xmdlocal:
	@echo Copying Task $(OUTPUTFILE) to XMD directory
	@rm -f $(XMD_DIR)/$(OUTPUTFILE).elf
	@cp $(OUTPUTFILE).elf $(XMD_DIR)/$(OUTPUTFILE).elf