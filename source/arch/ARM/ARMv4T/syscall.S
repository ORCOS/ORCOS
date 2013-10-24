/*
 * syscall.S
 *
 *  Created on: 21.05.2013
 *      Author: dbaldin
 */




#define _ASSEMBLER

#include "assembler.h"

.code 32

.section .text.syscall,  "ax", "progbits"

EXPORT_FCN(void, syscall, (void))
syscall:
	nop
	// r0-r3 should carry the paramters
	svc	 #0
	nop
	// r0 contains return value
	bx 	 lr
