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

/*
 * Defines for sparc V8 architecture
 */

#ifndef SPARC_HH_
#define SPARC_HH_


#define 					MHZ *1000000
#define	CLOCK_RATE			60 MHZ

#define set_reg_val(reg, val)\
				asm volatile ( \
					"st %1, [%0]" \
					: \
					:"r"(reg), "r"(val) \
					: "%l0", "%l1" \
					) \

#define get_reg_val(reg)({ int ret; asm volatile ( "ld [%1], %0" : "=r" (ret): "r" (reg)); ret;})

/* Processor State Register (PSR)
 *
 * +--------+--------+--------+------------+--+--+--------+--+--+--+----------+
 * |  impl  |   ver  |   icc  |  reserverd |EC|EF|   PIL  |S |PS|ET|    CWP   |
 * +--------+--------+--------+------------+--+--+--------+--+--+--+----------+
 *   31:28    27:24    23:20      19:14     13 12   11:8   7  6  5     4:0
 */

#define PSR_IMPL	0xF0000000		/* identifies the implementation of the architecture */
#define PSR_VER		0xF000000		/* version of implementation */
#define PSR_ICC		0xF00000		/* integer condition codes */
#define PSR_EC		0x2000		/* coprocessor enabled */
#define	PSR_EF		0x1000		/* FPU enabled */
#define PSR_PIL		0xF00		/* processor interrupt level */
#define PSR_S		0x80		/* sets the processor mode (supervisor / user) */
#define PSR_PS		0x40		/* determines the previous supervisor state */
#define PSR_ET		0x20		/* determines whether traps are enabled */
#define PSR_CWP		0x1F		/* current window pointer */


/* Trap Base Register (TBR)
 *
 * +----------------------------------------+----------------+--------+
 * |                  TBA                   |       tt       |  zero  |
 * +----------------------------------------+----------------+--------+
 *                   31:12                         11:4          3:0
 */

#define TBR_TBA		0xFFFFF000		/* trap table base address */
#define TBR_TT		0x00000FF0		/* determines trap type */

/*
 * Hardware Traps
 */

#define TRAP_IAEX	0x01	/* instruction access exception */
#define TRAP_II		0x02	/* illegal instruction */
#define TRAP_PI		0x03	/* privileged instruction */
#define TRAP_FPD	0x04	/* floating point disabled */
#define TRAP_WOF	0x05	/* window overflow */
#define TRAP_WUF	0x06	/* window underflow */
#define TRAP_MNA	0x07	/* memory address not aligned */
#define TRAP_FPE	0x08	/* floating point exception */
#define TRAP_DAEX	0x09	/* data access exception */
#define TRAP_TOF	0x0A	/* tag overflow */
#define TRAP_WD		0x0B	/* watchpoint detected */
#define TRAP_RAE	0x20	/* register access error */
#define TRAP_IAERR	0x21	/* instruction access error */
#define TRAP_CPD	0x24	/* coprocessor disabled */
#define TRAP_UFL	0x25	/* unimplemented flush instruction */
#define TRAP_CPE	0x28	/* coprocessor exception */
#define TRAP_DIVZ	0x2A	/* division by zero */
#define TRAP_DST	0x2B	/* data store error */
#define TRAP_DAERR	0x29	/* data access error */
#define TRAP_DMMU	0x2C	/* data access mmu miss */
#define TRAP_IMMU	0x3C	/* instruction access mmu miss */

/*
 * Interrupts
 */

#define TRAP_IRQ1	0x11	/* interrupt level 1 */
#define TRAP_IRQ2	0x12	/* interrupt level 2 */
#define TRAP_IRQ3	0x13	/* interrupt level 3 */
#define TRAP_IRQ4	0x14	/* interrupt level 4 */
#define TRAP_IRQ5	0x15	/* interrupt level 5 */
#define TRAP_IRQ6	0x16	/* interrupt level 6 */
#define TRAP_IRQ7	0x17	/* interrupt level 7 */
#define TRAP_IRQ8	0x18	/* interrupt level 8 */
#define TRAP_IRQ9	0x19	/* interrupt level 9 */
#define TRAP_IRQ10	0x1A	/* interrupt level 10 */
#define TRAP_IRQ11	0x1B	/* interrupt level 11 */
#define TRAP_IRQ12	0x1C	/* interrupt level 12 */
#define TRAP_IRQ13	0x1D	/* interrupt level 13 */
#define TRAP_IRQ14	0x1E	/* interrupt level 14 */
#define TRAP_IRQ15	0x1F	/* interrupt level 15 */

#endif /* SPARC_HH_ */
