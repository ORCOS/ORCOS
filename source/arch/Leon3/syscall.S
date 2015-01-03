    .globl syscall
syscall:
    ta 0x80
    nop
    nop
    nop
    nop
    nop
    nop
    retl
    nop

