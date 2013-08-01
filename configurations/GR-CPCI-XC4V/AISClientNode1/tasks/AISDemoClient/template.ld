
/*OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc",
	      "elf32-powerpc")
OUTPUT_ARCH(powerpc)*/

PHDRS
{
  text PT_LOAD ;
  rodata PT_LOAD ;
  data PT_LOAD ;
  sdata PT_LOAD ;
  sbss PT_LOAD ;
  bss PT_LOAD;
  protected_text PT_LOAD;
}

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
 	task_main.o(.text)
    static.o(.text)
    io.o(.text)
    mem.o(.text)
    net.o(.text)
    threads.o(.text)
    string.o(.text)
    liborcos.a(.text)
 	
 } :text

 /*. = ALIGN(0x1000000);*/

 .protected_text : {
 	main.o(.text)
 	map.o(.text) 
 } :protected_text

 .rodata : {
 	*(.rodata*)
 } :rodata
 
 .data : {
 	*(.data)
 } :data
 .bss : {
 	*(.bss)
 } :bss
 TASK_DATA_END = .;
}

