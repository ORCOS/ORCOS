
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !! Memory layout with 64 MB RAM !!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc",
	      "elf32-powerpc")

OUTPUT_ARCH(powerpc)
ENTRY(startORCOS)

/*
* In this configuration the OS + the task fit into a 4 MB Page. This could be reduced to way less than 256 Kb if needed.
*/

/* If you change any of these values, make sure you change them
   in the MEMORY part as well!  */
/*ramOrigin = 0x00000000;
ramLength = 0x04000000;*/ /* 64 MB */

/* Offset relative to RAM origin.  */
ramOffset = 0x4000;

/* Kernel heap + kernel stacks < 16 k */
/* this way they can easly be mapped into a 16 k page going from 0x10000 - 0x14000 */
_kernelHeapSize = 0x10000; /* 4 KB */
_kernelStackSize = 0x2000; /* 8 KB */

PROVIDE(_kernelHeapSize = 0x10000);
PROVIDE(_kernelStackSize = 0x2000);
PROVIDE(_kernelStackOffset = _kernelHeapSize + _kernelStackSize);

/*MEMORY
{
  ram (rwx) : ORIGIN = 0x00004000, LENGTH = 0x04000000
}*/


SECTIONS
{
  /* text area mapped into 128 k page going from 0x10000 - 0x30000 */
  . = ramOffset;
  PROVIDE(_text_start = .);
   
  /* first the text section */
  .text      .	  :
  {
    *(.text .text._Z*)
  } 

  PROVIDE (_text_end = .);
  
   . = 0x90000;
  PROVIDE(_data_start = .);
  
   /* read only data */
  .rodata	 .	: 
  	{ 
  	*(.rodata) *(.rodata.str*) *(.rodata._Z*)
  	
  	/* wite the tasktable into memory */
	
  	tasktable = .;
	*(.tasktable); 
   } 
 
  .data . :
  {
    *(.data)
    *(.sdata)
    *(.ctors)
  } 
  
  .sbss  . :
  {
    PROVIDE (__sbss_start = .);
    *(.sbss)
    PROVIDE (__sbss_end = .);
  } 
  
  .bss  .  :
  {
    PROVIDE (__bss_start = .);
    *(.bss)
    PROVIDE (__bss_end = .);
  } 
    
  
  /* directly put heap and stacks behind bss */
  PROVIDE (_heap_start = .);
  . += _kernelHeapSize;
  PROVIDE(_heap_end = .);
  . += _kernelStackSize;
  PROVIDE(__stack = .);
  
 	
 

  /DISCARD/ : {
  	*(.eh_frame) 	/* Discarding ErrorHandling Frame */
  	*(.comment) 	/* Discarding comments */
  	*(.data.rel.local)
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
