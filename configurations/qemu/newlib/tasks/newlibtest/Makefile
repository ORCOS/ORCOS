#STACK_USAGE_SCRIPT = $(KERNEL_DIR)../tools/PPCStackUsage/stack_usage.py
STACK_USAGE = python $(STACK_USAGE_SCRIPT)

OUTPUTFILE = newlibtest

OBJS 	= main.o

AFLAGS =
CFLAGS = -c -msoft-float -meabi -Os
CPPFLAGS = $(CFLAGS) -g -Wuninitialized -Woverloaded-virtual

all: $(OUTPUTFILE).elf binary stack_usage

$(OUTPUTFILE).bin: $(OUTPUTFILE).elf
	@$(OBJCOPY) -O binary $(OUTPUTFILE).elf $(OUTPUTFILE).bin

binary: $(OUTPUTFILE).bin

stack_usage:
ifneq ($(STACK_USAGE_SCRIPT),)
	@$(STACK_USAGE) $(OBJDUMP) $(OUTPUTFILE).elf main
endif

$(OUTPUTFILE).elf: $(OBJS) task.ld
	@echo task.mk: Linking $(OUTPUTFILE).elf
#	@$(LD) --script=task.ld -o $(OUTPUTFILE).elf /usr/local/powerpc-eabi/powerpc-eabi/lib/crt0.o $(OBJS) -L /usr/local/powerpc-eabi/powerpc-eabi/lib/nof -lc -lm -lstdc++ $(KERNEL_LIB_DIR)/liborcos_newlib.a #-L $(KERNEL_LIB_DIR) -lorcos_newlib
	#@$(LD) -XLinker --script=task.ld $(OBJS) $(GCC_LIB_DIR)/../../../../../powerpc-eabi/lib/crt0.o  ../../$(KERNEL_LIB_DIR)/libgloss.o ../../$(KERNEL_LIB_DIR)/liborcos_newlib.a -o $(OUTPUTFILE).elf -L../../$(KERNEL_LIB_DIR) -L$(GCC_LIB_DIR)/../../../../../powerpc-eabi/lib/ -L$(GCC_LIB_DIR) -lorcos -lc -lm
	$(CXX) -o $(OUTPUTFILE).elf -msoft-float -meabi $(GCC_LIB_DIR)/../../../../../powerpc-eabi/lib/crt0.o $(OBJS) -L$(GCC_LIB_DIR)/../../../../../powerpc-eabi/lib/nof/ -L$(GCC_LIB_DIR) -lc -lm ../../$(KERNEL_LIB_DIR)/static.o ../../$(KERNEL_LIB_DIR)/libgloss.o ../../output/syscall.o -Xlinker -Ttask.ld

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
