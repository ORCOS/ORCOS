
OUTPUT_FORMAT("elf32-powerpc", "elf32-powerpc",
	      "elf32-powerpc")
OUTPUT_ARCH(powerpc)

SECTIONS
{
 .text 0xfff80000 :
 {
  LONG(0);
 }

 .bootjump 0xfffffffc : 
 {
 	*(bootjump)
 }
}

