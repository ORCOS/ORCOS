
/* OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc",
	      "elf32-powerpc") */
/* OUTPUT_ARCH(powerpc) */

SECTIONS
{
 . = TASK_START;
 .CB TASK_VMA : AT(TASK_START) {
    LONG(0x230f7ae9);  /* ORCOS Task MAGIC value */
    LONG(0x0);		   /* no next header field, RTparams are mandatory */
    
    /* Task CB types.hh: struct taskTable */
    LONG(TASK_VMA);
    LONG(task_main);
    LONG( TASK_VMA + (TASK_HEAP - TASK_START) );
    LONG( TASK_VMA + (TASK_END - TASK_START));
    LONG(TASK_DATA_END);
    LONG(thread_exit);
    
    /* Task CB Priority and RealtimeThread Parameters: struct RTThreadAttrs  */
    LONG(INITIAL_PRIORITY);
    LONG(PHASE);
    LONG(PERIOD);
    LONG(DEADLINE);
    LONG(EXECUTIONTIME);
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
 TASK_DATA_END = .;
}

