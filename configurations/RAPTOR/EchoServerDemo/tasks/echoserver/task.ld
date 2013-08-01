
/* OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc",
	      "elf32-powerpc")
OUTPUT_ARCH(powerpc) */

SECTIONS
{
 . = 0x140000;
 .text 0x140000 : AT(0x140000) {
    LONG(0x140000);
    LONG(task_main);
    LONG(0x148000);
    LONG(0x14FFFF);
    LONG(TASK_DATA_END);
    LONG(thread_exit);
    LONG(0);
    LONG(2000000);
    LONG(500000);
    LONG(1000000);
    LONG(0);
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

