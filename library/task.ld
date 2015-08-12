
PROVIDE(_taskHeapEnd = TASK_END);

SECTIONS
{
 . = TASK_START;
 .CB TASK_VMA : AT(TASK_START) {
    LONG(0x230f7ae9);     /* ORCOS Task MAGIC value */
    LONG(NEXT_HEADER);    /* next header field */
    LONG(PLATFORM);       /* PLATFORM Flags */

    /* Task CB types.hh: struct taskTable */
    LONG(TASK_VMA);
    LONG(task_main);
    LONG(thread_exit);
    LONG(TASK_HEAP);
    LONG(TASK_END);

    /* Task CB Priority and RealtimeThread Parameters: struct RTThreadAttrs  */
    QUAD(PHASE);
    LONG(INITIAL_PRIORITY);
    LONG(PERIOD);
    LONG(DEADLINE);
    LONG(EXECUTIONTIME);
    /* Stack size of the initial/main thread of this task */
    LONG(STACK_SIZE);
    LONG(0);

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
     *(.bss*)
  }
  .sbss     ALIGN(4) :
  {
     PROVIDE(__sbss_start = .);
     *(.sbss*)
     PROVIDE(__sbss_end = .);
  }
  .init     ALIGN(4) :
  {
     PROVIDE(__init_start = .);
     *(.init*)
     PROVIDE(__init_end = .);
  }
  .fini     ALIGN(4) :
  {
     PROVIDE(__fini_start = .);
     *(.fini*)
     PROVIDE(__fini_end = .);
  }
 CRC_END = .;

 TASK_HEAP = ALIGN(TASK_HEAP_ALIGN);
 PROVIDE(_taskHeapStart = TASK_HEAP);
}

