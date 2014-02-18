
/* OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc",
	      "elf32-powerpc") */
/* OUTPUT_ARCH(powerpc) */

SECTIONS
{
 . = 0x81300000;
 .CB 0x100000 : AT(0x81300000) {
    LONG(0x230f7ae9);  /* ORCOS Task MAGIC value */
    LONG(0x0);		   /* no next header field, RTparams are mandatory */
    LONG(0x101);	   /* PLATFORM ARM, T BIT set*/
    
    /* Task CB types.hh: struct taskTable */
    LONG(0x100000);
    
    LONG(task_main);
    LONG(thread_exit);
    
    LONG(0x81320000 );
    LONG(0x81400000);
           
    /* Task CB Priority and RealtimeThread Parameters: struct RTThreadAttrs  */
    LONG(1);
    LONG(0);
    LONG(500000);
    LONG(500000);
    LONG(0);
    LONG(4096);   
    
   }
   
.text : {    
    static.o(.text)
 	*(.text*)
 }
 .rodata : {
	*(.rodata*)
 }
 
 .data : {
 	*(.data*)
 }
 .bss : {
 	*(.bss*)
 }
 TASK_DATA_START = ALIGN(0x1000);
}

