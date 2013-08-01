
OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc",
	      "elf32-powerpc")
OUTPUT_ARCH(powerpc)

SECTIONS
{
 . = TASK_START;
 .text TASK_VMA : AT(TASK_START) {
    LONG(TASK_START);
    LONG(task_main);
    LONG(TASK_HEAP);
    LONG(TASK_END);
    LONG(TASK_DATA_END);
    LONG(thread_exit);
    LONG(INITIAL_PRIORITY);
    LONG(PHASE);
    LONG(PERIOD);
    LONG(DEADLINE);
    LONG(EXECUTIONTIME);
    static.o(.text)
 	*(.text)
 }
 .rodata : {
	*(.rodata)
 }
 .data : {
 	*(.data)
 }
 .bss : {
 	*(.bss)
 }
 TASK_DATA_END = .;
}

