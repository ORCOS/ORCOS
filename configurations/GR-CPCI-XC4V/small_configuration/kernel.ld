/* 
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !! Memory layout with 32 MB RAM !!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

OUTPUT_FORMAT("elf32-sparc", "elf32-sparc", "elf32-sparc")
OUTPUT_ARCH(sparc)

ENTRY(startORCOS)

/* 
 * If you change any of these values, make sure you change them
 * in the MEMORY part as well!  
 */
ramOrigin = 0x40000000;
ramLength = 0x02000000; /* 32 MB */

/* 
 * Offset relative to RAM origin.  
 */
ramOffset = 0x4000;


/* 
 * Default size of Kernel stack and Heap 
 */
_kernelHeapSize = 0x8000;  /*  8 KB */
_kernelStackSize = 0x4000; /* 16 KB */

PROVIDE(_kernelHeapSize = 0x8000);
PROVIDE(_kernelStackSize = 0x8000);
PROVIDE(_kernelStackOffset = _kernelHeapSize + _kernelStackSize);

MEMORY
{
  ram (rwx) : ORIGIN = 0x40004000, LENGTH = 0x02000000
}

SECTIONS
{
  . = ALIGN(8);
  . = ramOffset + ramOrigin;
  PROVIDE(_text_start = .);
  
  .text      	  :
  {
    *(.text .text._Z* .gnu.linkonce.t*)
  } 
  
  PROVIDE (_text_end = .);
  . = ALIGN(0x40000);
  PROVIDE(_data_start = .);
  
  .init			: { *(.init)				} > ram
 
  .fini			: { *(.fini)				} > ram
  .rodata ALIGN(0x40000) : { *(.rodata) *(.rodata.str*) *(.rodata._ZTV*) *(.gnu.linkonce.r*)  
  . = ALIGN(4);
  	tasktable = .;
	*(.tasktable); 
   } > ram
 
  .gcc_except_table	: { *(.gcc_except_table)		} > ram
  _etext = .; 
 
  .data    :
  {
    *(.data)
  } > ram

  .data.rel.local : { *(.data.rel.local) } > ram
  .data.rel       : { *(.data.rel) } > ram

  .ctors	  : { *(.ctors) } > ram

 /* We want the small data sections together, so single-instruction offsets
     can access them all, and initialized data all before uninitialized, so
     we can shorten the on-disk segment size.  */
 
  .sdata	  : { *(.sdata) } > ram 
  _edata  =  .; 
  PROVIDE (edata = .);
  .sbss      :
  {
    PROVIDE (__sbss_start = .);
    *(.sbss)
    PROVIDE (__sbss_end = .);
  } > ram
  .bss       :
  {
    PROVIDE (__bss_start = .);
    *(.bss)
    PROVIDE (__bss_end = .);
  } > ram
  _end = . ;
  
  /* These are needed for ELF backends which have not yet been
     converted to the new style linker.  */
  .stab : { *(.stab) } > ram 
  PROVIDE (end = .);
  
  
 /* directly put heap and stacks behind bss */
  PROVIDE (_heap_start = .);
  . += _kernelHeapSize;
  PROVIDE(_heap_end = .);
  . += _kernelStackSize;
  PROVIDE(__stack = .);
 
  /DISCARD/ : {
  	*(.eh_frame) 	/* Discarding ErrorHandling Frame */
  	*(.comment) /* Discarding comments */
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
