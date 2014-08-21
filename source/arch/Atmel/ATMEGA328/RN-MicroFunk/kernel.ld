 
ENTRY(RESET)

OUTPUT_FORMAT("elf32-avr","elf32-avr","elf32-avr")
OUTPUT_ARCH(avr:5)
MEMORY
{
  ram   (rwx)   : ORIGIN = 0, LENGTH = 2K
  flash   (rx) : ORIGIN = 0x0, LENGTH = 32K
}


/*
* In this configuration the OS + the task fit into a 4 MB Page. 
* This could be reduced to way less than 256 Kb if needed.
*/

/* If you change any of these values, make sure you change them
   in the MEMORY part as well!  */
ramOrigin = 0x100;
ramLength = 0x7FF; 

/* Offset to RAM origin.  */
ramOffset = 0x100;

_kernelHeapSize = 0x300; 
_kernelStackSize = 0x100;

PROVIDE(_kernelHeapSize = 0x100);
PROVIDE(_kernelStackSize = 0x100);
PROVIDE(_kernelStackOffset = _kernelHeapSize + _kernelStackSize);
PROVIDE(__RAM_END = 0x8ff );
PROVIDE(__RAM_START = 0x100 );

PROVIDE(__data_load_start = 0x100);
PROVIDE(__stack = 0x880);
 
SECTIONS
{

  . = 0x0;
  
  /* text area */
  PROVIDE(_text_start = .);

  .text   0x0 	  : AT(0x0)
  {
  	PROVIDE(_irq_start = .);
	*(.iv)
 	PROVIDE(_irq_end = .);
    *(.text*)
     /* The jump trampolines for the 16-bit limited relocs will reside here.  */
    *(.trampolines)
    *(.trampolines*)
    *(.init*)
    *(.progmem.data*)
  } > flash

  PROVIDE (_text_end = .);
  __KERNELEND = _text_end;

.data	0x100  : AT (ADDR (.text) + SIZEOF (.text))
  {
   	PROVIDE(_data_start = .);
     PROVIDE (__data_start = .) ;
    /* --gc-sections will delete empty .data. This leads to wrong start
       addresses for subsequent sections because -Tdata= from the command
       line will have no effect, see PR13697.  Thus, keep .data  */
    KEEP (*(.data))
    *(.data*)
    tasktable = .;
	*(.tasktable); 
    *(.rodata)  /* We need to include .rodata here if gcc is used */
    *(.rodata*) /* with -fdata-sections.  */
    *(.gnu.linkonce.d*)
    . = ALIGN(2);
     _edata = . ;
     PROVIDE (__data_end = .) ;
     
    PROVIDE (__bss_start = .) ;
    *(.bss)
    *(.bss*)
    *(COMMON)
     PROVIDE (__bss_end = .) ;
     . = ALIGN(4);
     PROVIDE (_heap_start = .);
     PROVIDE(_heap_end = . + _kernelHeapSize);    
  }  > flash
  		
 
  __data_load_start = LOADADDR(.data);
  __data_load_end = __data_load_start + (__data_end -  __data_start );
  

  /DISCARD/ : {
  	*(.eh_frame) 	/* Discarding ErrorHandling Frame */
  	*(.comment) 	/* Discarding comments */
  	*(.data.rel.local)
  	*(.gnu*)
  }
  

  /* DWARF debug sections.
     Symbols in the DWARF debugging sections are relative to the beginning
     of the section so we begin them at 0.  */
  /* DWARF 1 */
  .debug           0 : { *(.debug) } 
  .line            0 : { *(.line) }
  /* GNU DWARF 1 extensions */
  .debug_srcinfo   0 : { *(.debug_srcinfo) }
  .debug_sfnames   0 : { *(.debug_sfnames) }
  /* DWARF 1.1 and DWARF 2 */
  .debug_aranges   0 : { *(.debug_aranges) } 
  .debug_pubnames  0 : { *(.debug_pubnames) } 
  /* DWARF 2 */
  .debug_info      0 : { *(.debug_info) } 
  .debug_abbrev    0 : { *(.debug_abbrev) } 
  .debug_line      0 : { *(.debug_line) } 
  .debug_frame     0 : { *(.debug_frame) } 
  .debug_str       0 : { *(.debug_str) } 
  .debug_loc       0 : { *(.debug_loc) } 
  .debug_macinfo   0 : { *(.debug_macinfo) } 
  /* SGI/MIPS DWARF 2 extensions */
  .debug_weaknames 0 : { *(.debug_weaknames) } 
  .debug_funcnames 0 : { *(.debug_funcnames) } 
  .debug_typenames 0 : { *(.debug_typenames) } 
  .debug_varnames  0 : { *(.debug_varnames) } 
  /* These must appear regardless of  .  */
}
