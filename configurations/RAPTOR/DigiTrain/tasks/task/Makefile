STACK_USAGE = python $(STACK_USAGE_SCRIPT)

OUTPUT_DIR = output/
TARGETNAME = task
OUTPUTFILE =  $(OUTPUT_DIR)$(TARGETNAME)

TOBJ  = task_main.o ETHLI100.o Train.o LI100MessageQueue.o
OBJS = $(addprefix $(OUTPUT_DIR),$(TOBJ))

VPATH = ../../LI100/

AFLAGS =
CFLAGS = -c -I $(KERNEL_LIB_DIR) -I../../LI100/ -fno-exceptions -fno-rtti -msoft-float
CPPFLAGS = $(CFLAGS) -g  
	
all: 
	@make -s $(OUTPUTFILE)
	@make -s binary 
	@make -s stack_usage
	
binary:
	@$(OBJCOPY) -O binary  $(OUTPUTFILE).elf  $(OUTPUTFILE).bin
	
stack_usage:
ifneq ($(STACK_USAGE_SCRIPT),)
	@$(STACK_USAGE) $(OBJDUMP) $(OUTPUTFILE).elf task_main
endif
		
$(OUTPUTFILE): $(OBJS)  $(OUTPUT_DIR)task.ld
	@echo task.mk: Linking $(OUTPUTFILE).elf
	@$(LD) --script=task.ld $(OBJS) -o  $(OUTPUTFILE).elf -L$(KERNEL_LIB_DIR) -lorcos
	
$(OUTPUT_DIR)task.ld: task.sed
	@echo task.mk: Generating linkerscript task.ld
	@$(SED) -f task.sed template.ld >task.ld
	
$(OUTPUT_DIR)%.o: %.cpp
	@echo task.mk: Compiling $@
	@$(CXX) $(CPPFLAGS) $< -o $@

$(OUTPUT_DIR)%.o: %.c
	@echo task.mk: Compiling $@
	@$(CXX) $(CFLAGS) $< -o $@

$(OUTPUT_DIR)%.o: %.cc
#@echo @$(CXX) $(CPPFLAGS) $< -o $@
	@echo task.mk: Compiling $@
	@$(CXX) $(CPPFLAGS) $< -o $@

$(OUTPUT_DIR)%.o: %.S
	@echo task.mk: Assembling $@
	@$(AS) $(AFLAGS) $< -o $@
	
clean:
	@echo Cleaning up Task files
	$(RM) $(OUTPUT_DIR)* 
	
bdi:
	@echo Loading Task $(OUTPUTFILE) to BDI Target
	@cp $(OUTPUTFILE).elf $(BDI_DIR)/$(OUTPUTFILE).elf
	@python $(BDICOMMAND) "$(BDIADRESS)" "$(BDIPORT)" "load" "load $(OUTPUTFILE).elf"

xmd:
	@echo Copying Task $(OUTPUTFILE).elf to XMD directory
	@cp  $(OUTPUTFILE).elf $(XMD_DIR)/$(TARGETNAME).elf