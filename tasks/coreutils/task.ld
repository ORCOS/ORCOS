
/*OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)*/

MEMORY
{
  ram (rwx) : ORIGIN = 0x100000, LENGTH = 0x08000000
}

/* Offset to RAM origin.  */
ramOffset = 0x100000;

TASK_DATA_END  = 0x1fffff; /* heap for static data */

SECTIONS
{
 . = ramOffset;
 .CB 0x100000 : {
    LONG(0x230f7ae9);    /* ORCOS Task MAGIC value */
    LONG(0x0);		     /* no next header field, RTparams are mandatory */
    LONG(0x101);	     /* PLATFORM ARM, T BIT set*/
      
    /* Task CB types.hh: struct taskTable */
    
    /* Executable Area definitions */
    
    LONG(0x100000);      					/* start address */
    
    /* Symbols inside the executable area*/
    
    LONG(task_main);    					/* entry address */
    LONG(thread_exit);   					/* return address for thread exit */
       
    /* Data Area Definitions */
    
    LONG(TASK_DATA_START);      			/* data start address */
    LONG(TASK_DATA_END); /* task end address */
  
    
    /* Task CB Priority and RealtimeThread Parameters: struct RTThreadAttrs  */
  	LONG(0);
    LONG(0);
    LONG(0);
    LONG(0);
    LONG(0);
    LONG(4096);   
    
    /*next Header*/
    LONG(0x0);
    LONG(0x0); /* Task CRC*/
   }
   
 .text ADDR(.CB) + SIZEOF(.CB) : {
  static.o(.text)
    *(.text*)
	
 	}
 
 .rodata ADDR(.text) + SIZEOF(.text)  : {
	*(.rodata*)
 }
 
 .data ADDR(.rodata) + SIZEOF(.rodata)  : {
 	*(.data*)
 }
 
 .bss ADDR(.data) + SIZEOF(.data)  : {
 	*(.bss*)
 }
TASK_DATA_START = ALIGN(ADDR(.bss) + SIZEOF(.bss),0x1000);

}

