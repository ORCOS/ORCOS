
/* OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc",
	      "elf32-powerpc") */
/* OUTPUT_ARCH(powerpc) */

SECTIONS
{
 . = 0x81100000;
 .CB 0x100000 : AT(0x81100000) {
    LONG(0x230f7ae9);  /* ORCOS Task MAGIC value */
    LONG(0x0);		   /* no next header field, RTparams are mandatory */
    
    /* Task CB types.hh: struct taskTable */
    LONG(0x100000);
    LONG(task_main);
    LONG( 0x100000 + (0x81177000 - 0x81100000) );
    LONG( 0x100000 + (0x8117FFFF - 0x81100000));
    LONG(TASK_DATA_END);
    LONG(thread_exit);
    
    /* Task CB Priority and RealtimeThread Parameters: struct RTThreadAttrs  */
    LONG(0);
    LONG(0);
    LONG(500000);
    LONG(500000);
    LONG(0);   
    
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
 TASK_DATA_END = .;
}

