
OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)

MEMORY
{
  ram (rwx) : ORIGIN = 0x100000, LENGTH = 0x08000000
}

/* Offset to RAM origin.  */
ramOffset = 0x100000;

SECTIONS
{
 . = ramOffset;
  
 .CB 0x100000 : {
    LONG(0x230f7ae9);  /* ORCOS Task MAGIC value */
    LONG(0x0);		   /* no next header field, RTparams are mandatory */
    LONG(0x101);	   /* PLATFORM ARM, T BIT set*/
      
    /* Task CB types.hh: struct taskTable */
    LONG(0x100000);   /* start address */
    LONG(task_main);  /* entry address */
    LONG(0x177000);  /* heap start address */
    LONG(0x18f000);  /* heap end address */
    LONG(TASK_DATA_END);
    LONG(thread_exit);
    
    /* Task CB Priority and RealtimeThread Parameters: struct RTThreadAttrs  */
    LONG(0);
    LONG(0);
    LONG(0);
    LONG(0);
    LONG(0);   
   
   }
   
.text . : {
	static.o(*.text*)
	*(.text*)	
 }
 
 .rodata . : {
	*(.rodata*)
 }
 
 .data . : {
 	*(.data*)
 }
 
 .bss . : {
 	*(.bss*)
 }

 TASK_DATA_END = .;
}

