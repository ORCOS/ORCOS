
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !! Memory layout with 128 MB RAM !!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm",
	      "elf32-littlearm") 
 
ENTRY(startORCOS)

/*
* In this configuration the OS + the task fit into a 4 MB Page. 
* This could be reduced to way less than 256 Kb if needed.
*/

/* If you change any of these values, make sure you change them
   in the MEMORY part as well!  */
ramOrigin = 0x80000000;
ramLength = 0x08000000; /* 128 MB */

/* Offset to RAM origin.  */
ramOffset = 0x81000000;

/* Kernel heap + kernel stacks < 16 k */
/* this way they can easily be mapped into a 16 k page going from 0x10000 - 0x14000 */
_kernelHeapSize = 0x40000; /* 65*4 KB */
_kernelStackSize = 0x2000; /* 8 KB */

__PtSize = 0x4000; /* 16 KB */
PROVIDE(__PtSize = 0x4000); /* 16 KB */
__MaxNumPts = 20;  /* 20 * 16 KB = 320 KB */

PROVIDE(_kernelHeapSize = 0x10000);
PROVIDE(_kernelStackSize = 0x2000);
PROVIDE(_kernelStackOffset = _kernelHeapSize + _kernelStackSize);
PROVIDE(__RAM_END = ramOrigin + ramLength );
PROVIDE(__RAM_START = ramOrigin );

MEMORY
{
  ram (rwx) : ORIGIN = 0x80000000, LENGTH = 0x08000000
}


SECTIONS
{

  . = ramOffset;
  __LOADADDRESS = .;
    
   __exidx_start = .;
   __exidx_end = .;
   
  /* irq section */ 
  .irq	.	:
  {
  	PROVIDE(_irq_start = .);
	*(.irq)
 	PROVIDE(_irq_end = .);
  }

  /* swi section */ 
  .swi	.	:
  {
  	PROVIDE(_swi_start = .);
	*(.swi)
 	PROVIDE(_swi_end = .);
  }
  
  /* data_abort section */ 
  .data_abort	.	:
  {
  	PROVIDE(_data_abort_start = .);
	*(.data_abort)
 	PROVIDE(_data_abort_end = .);
  }
  
  /* text area */
  /*. = ramOffset;*/
  PROVIDE(_text_start = .);
   
  /* first the text section */
  .text      .	  :
  {
    *(.text)
    *(.text.*)
  } 

  PROVIDE (_text_end = .);
  
  /* read only data */
  .rodata ALIGN(4)	: 
  	{
  	PROVIDE(_data_start = .);
  	 
  	*(.rodata.*) *(.rodata.str*) *(.rodata._Z*)
  	
  	/* write the tasktable into memory */
	
  	tasktable = .;
	*(.tasktable); 
   } 
  		
  .data	 ALIGN(4) :
  {
  	PROVIDE(_data_start_data = .);
    *(.data)
    *(.data.*)
    *(.sdata)
    *(.ctors)
  } 
  
  .sbss  . :
  {
    PROVIDE (__sbss_start = .);
    *(.sbss)
    PROVIDE (__sbss_end = .);
  } 
  
  . = ALIGN(4);
  PROVIDE (__bss_start = .);
  
  .bss  .  :
  {
    *(.bss)        
    *(.bss.*)
  }  
  . = ALIGN(4);
  PROVIDE (__bss_end = .);
  
  /* directly put heap and stacks behind bss */
  PROVIDE (_heap_start = .);
  . += _kernelHeapSize;
  PROVIDE(_heap_end = .);
  . += _kernelStackSize;
  PROVIDE(__stack = .);	
 

  /* put page table in memory (16k alignment) */
  . = ALIGN(0x4000);
  
  PROVIDE (__PageTableSec_start = .);
  . += (__PtSize * __MaxNumPts);
  PROVIDE (__PageTableSec_end = .);

  __KERNELEND = .;

  /DISCARD/ : {
  	*(.eh_frame) 	/* Discarding ErrorHandling Frame */
  	*(.comment) 	/* Discarding comments */
  	*(.data.rel.local)
  	*(.ARM.exidx* .gnu.linkonce.armexidx.*)
  	*(.ARM.ex*)
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
