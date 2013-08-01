#STACK_USAGE_SCRIPT = $(KERNEL_DIR)../tools/PPCStackUsage/stack_usage.py
STACK_USAGE = python $(STACK_USAGE_SCRIPT)

OUTPUTFILE = task1_qemu

OBJS 	= task_main.o

AFLAGS =
CFLAGS = -c -I $(KERNEL_LIB_DIR) -fno-exceptions -fno-rtti -msoft-float
CPPFLAGS = $(CFLAGS) -g -fno-exceptions -fno-rtti -msoft-float -Wuninitialized -Woverloaded-virtual

all: $(OUTPUTFILE).elf binary stack_usage

$(OUTPUTFILE).bin: $(OUTPUTFILE).elf
	@$(OBJCOPY) -O binary $(OUTPUTFILE).elf $(OUTPUTFILE).bin

binary: $(OUTPUTFILE).bin

stack_usage:
ifneq ($(STACK_USAGE_SCRIPT),)
	@$(STACK_USAGE) $(OBJDUMP) $(OUTPUTFILE).elf task_main
endif

$(OUTPUTFILE).elf: $(OBJS) task.ld
	@echo task.mk: Linking $(OUTPUTFILE).elf
	@$(LD) --script=task.ld $(OBJS) -o $(OUTPUTFILE).elf -L $(KERNEL_LIB_DIR) -lorcos

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
