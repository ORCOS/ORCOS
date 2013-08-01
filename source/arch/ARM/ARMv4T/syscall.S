/*
 * syscall.S
 *
 *  Created on: 21.05.2013
 *      Author: dbaldin
 */




#define _ASSEMBLER

#include "SCLConfig.hh"
#include "ARMv4T.h"
#include "assembler.h"

.section .text.syscall,  "ax", "progbits"

EXPORT_FCN(void, syscall, (void))
syscall:
	push {fp}
	// r0-r3 should carry the paramters
	svc	 #0
	// r0 contains return value
	pop	 {fp}
	bx 	 lr
