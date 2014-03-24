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

#ifndef _MACHINE_POWERPC_H
#define _MACHINE_POWERPC_H


#define MHZ *1000000
#define	CLOCK_RATE					300 MHZ
#define ms * 300000
#define MILLISECONDS * 300000
#define MICROSECONDS * 300


/*------------------------------------------------------*/
/*   Redefinitions for some special purpose registers   */
/*------------------------------------------------------*/

#define SPR_SRR1	SRR1

/*------------------------------------------------------*/
/*	2.3.1   Machine State Register for 64 bit       */
/*------------------------------------------------------*/

#ifndef PPC_MSR_DEFINED
#define PPC_MSR_DEFINED

/// Exception little-endian mode
#define PPC_MSR_ILE	(1<<16)
/// External interrupt enable
#define PPC_MSR_EE	(1<<15)
/// Privilege level
#define PPC_MSR_PR	(1<<14)
/// Floating-point available
#define PPC_MSR_FP	(1<<13)
/// Machine check enable
#define PPC_MSR_ME	(1<<12)
/// Floating-point exception mode 0
#define PPC_MSR_FE0	(1<<11)
/// Single-step trace enable
#define PPC_MSR_SE	(1<<10)
/// Debug interrupts enable
#define PPC_MSR_DE	(1<<9)
/// Floating-point exception mode 1
#define PPC_MSR_FE1	(1<<8)
/// Exception prefix
#define PPC_MSR_IP	(1<<6)
/// Instruct address translation
#define PPC_MSR_IR	(1<<5)
/// Data address translation
#define PPC_MSR_DR	(1<<4)
/// Recoverable exception
#define PPC_MSR_RI	(1<<1)
/// Little-endian mode enable
#define PPC_MSR_LE	(1<<0)
#endif /* PPC_MSR_DEFINED */

#define PPC_BYTES_PER_WORD		4

#ifndef _ASSEMBLER

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef int greg_t;

#if defined(__GNUC__)

#define ppc_sync()		asm volatile ("sync")
#define ppc_isync()		asm volatile ("isync")

#define ppc_mfmsr()	({ greg_t _; asm volatile ("mfmsr %0" : "=r" (_)); _;})
#define ppc_mtmsr(_)	asm volatile ("mtmsr %0" : : "r" (_))

#define ppc_mftbl()	({ greg_t _; asm volatile ("mftbl %0" : "=r" (_)); _;})
#define ppc_mftbu()	({ greg_t _; asm volatile ("mftbu %0" : "=r" (_)); _;})

#define ppc_mfsr(SR)	({ greg_t rD; asm volatile ("mfsr %0, %1" : "=r" (rD) : "i" (SR)); rD;})
#define ppc_mtsr(SR,rS)	asm volatile ("mtsr %0, %1" : : "i" (SR), "r" (rS))

#define ppc_mfspr(spr)		({ greg_t _; asm volatile ("mfspr %0, %1" : "=r" (_) : "i" (spr)); _; })
#define ppc_mtspr(spr,val)	asm volatile ("mtspr %0, %1" : : "i" (spr), "r" (val))

#define ppc_tlbia()		asm volatile ("tlbia")
#define ppc_tlbie(rB)		asm volatile ("tlbie %0" : : "r" (rB))
#define ppc_tlbsync()		asm volatile ("tlbsync")

#endif /* __GNUC__ */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* _ASSEMBLER */

#endif /* _MACHINE_POWERPC_H */

