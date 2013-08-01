/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2008 University of Paderborn

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _SHARED_POWERPC_ASSEMBLER_H
#define _SHARED_POWERPC_ASSEMBLER_H

#include "powerpc.h"

/*
 * Defines for direct register access
 */

#define r0	0
#define sp	1
#define r1	1
#define toc	2
#define r2	2
#define r3	3
#define r4	4
#define r5	5
#define r6	6
#define r7	7
#define r8	8
#define r9	9
#define r10	10
#define r11	11
#define r12	12
#define r13	13
#define r14	14
#define r15	15
#define r16	16
#define r17	17
#define r18	18
#define r19 19
#define r20	20
#define r21	21
#define r22	22
#define r23	23
#define r24	24
#define r25	25
#define r26	26
#define r27	27
#define r28	28
#define r29	29
#define r30	30
#define r31	31
#define f0	0
#define f1	1
#define f2	2
#define f3	3
#define f4	4
#define f5	5
#define f6	6
#define f7	7
#define f8	8
#define f9	9
#define f10	10
#define f11	11
#define f12	12
#define f13	13
#define f14	14
#define f15	15
#define f16	16
#define f17	17
#define f18	18
#define f19 19
#define f20	20
#define f21	21
#define f22	22
#define f23	23
#define f24	24
#define f25	25
#define f26	26
#define f27	27
#define f28	28
#define f29	29
#define f30	30
#define f31	31

/*
 * How to declare a global label
 */
#define _GLOBAL(n)\
	.globl n;\

/*
 * How to declare an externally defined variable/label
 */
#define _EXTERN(name) .extern FUNC_NAME(name);

/*
 * <group>
 * Macros to begin and end a function written in assembler.  If -mcall-aixdesc
 * or -mcall-nt, create a function descriptor with the given name, and create
 * the real function with one or two leading periods respectively.
 */
#define STARTOF_Fcn(name)  _s_##name
#define ENDOF_Fcn(name)    _e_##name
/*
 * </group>
 */

#define FUNC_NAME(name) name

#define FUNC_START(name) \
	.type name,@function; \
name: \
STARTOF_Fcn(name):

#define FUNC_END(name) \
name##End: \
ENDOF_Fcn(name):


#define ASM_LABEL(l)                    asm(".global " #l "; " #l ":")

#define EXPORT_VAR(type, var)           _GLOBAL(var)
#define IMPORT_VAR(type, var)           _EXTERN(var)
#define EXPORT_FCN(type, fcnname, args) _GLOBAL(fcnname)
#define IMPORT_FCN(type, fcnname, args) _EXTERN(fcnname)
#define FORWARD_FCN(fcn) \
        .type fcn,@function;

/* pseudo instructions -----------------------------------------------------*/
/// no operation
#define nop             ori r0,r0,0

/// load integer constant to register */
#define ldc(reg, val)   lis reg,(val)@h; ori reg,reg,(val)@l
#define ldv(reg, var)   lis reg,(var)@h; ori reg,reg,(var)@l

/// var := val using registers reg1, reg2
#define setVar(var, val, reg1, reg2) \
   ldv(reg1, var); ldc(reg2, val); stw reg2, 0(reg1)


#endif /* _SHARED_POWERPC_ASSEMBLER_H */
