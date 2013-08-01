
/* !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 * !! Memory layout with 128 MB RAM !!
 * !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm",
	      "elf32-littlearm") 
 
ENTRY(module_init)
MODULE_VMA = 0x50000;

MEMORY
{
  ram (rwx) : ORIGIN = 0x80000000, LENGTH = 0x08000000
}


SECTIONS
{

  . = MODULE_PHY_MEM_ADDR;
   
  /* first the text section */
  .text MODULE_VMA : AT(MODULE_PHY_MEM_ADDR) 
  {
    *(.module_mcb)
    *(.text.*)    
  } 
  
  /* read only data */
  .rodata ALIGN(4)	: 
  	{	 
  	*(.rodata) *(.rodata.str*) *(.rodata._Z*)
   } 	
  		
  .data	 ALIGN(4) :
  {
    *(.data)
    *(.sdata)
    *(.ctors)
  } 
  
  .sbss  ALIGN(4) :
  {
    *(.sbss)
  } 
  
  .bss  ALIGN(4)  :
  {
    *(.bss)        
  }  

  /DISCARD/ : {
  	*(.eh_frame) 	/* Discarding ErrorHandling Frame */
  	*(.comment) 	/* Discarding comments */
  	*(.data.rel.local)
  	*(.ARM.exidx* .gnu.linkonce.armexidx.*)
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
