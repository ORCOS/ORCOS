
SECTIONS
{
 . = TASK_START;
 .CB TASK_VMA : AT(TASK_START) {
    LONG(0x230f7ae9);  /* ORCOS Task MAGIC value */
    LONG(NEXT_HEADER); /* next header field */
    LONG(PLATFORM);	   /* PLATFORM Flags */
    
    /* Task CB types.hh: struct taskTable */
    LONG(TASK_VMA);
    LONG(task_main);
    LONG(thread_exit);    
    LONG(TASK_HEAP );
    LONG(TASK_END);
           
    /* Task CB Priority and RealtimeThread Parameters: struct RTThreadAttrs  */
    LONG(INITIAL_PRIORITY);
    LONG(PHASE);
    LONG(PERIOD);
    LONG(DEADLINE);
    LONG(EXECUTIONTIME);
    /* Stack size of the initial/main thread of this task */
    LONG(STACK_SIZE);      
    
    /* Additional header following */
    ADDITIONAL_HEADER 
   }

CRC_START = .;
.text : {    
    static.o(.text)
 	*(.text*)
 }
 .rodata : {
	*(.rodata*)
 }
 
 .data : {
 	*(.data*)
 	*(.init*)
 }
 CRC_END = .;
 
 .bss : {
 	*(.bss*)
 }
 
 TASK_HEAP = ALIGN(TASK_HEAP_ALIGN);
}

