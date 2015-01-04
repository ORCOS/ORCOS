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

#ifndef _PPC405_H
#define _PPC405_H

#include "archtypes.h"
#include "powerpc.h"

/*------------------------------------------------------*/
/*   Cache related definition                           */
/*------------------------------------------------------*/


#ifndef PPC_CACHE_BLOCK_SIZE
#define PPC_CACHE_BLOCK_SIZE        32
#endif

//-----------------------------------------------------------------------------
// SPRN values of Special Purpose Registers
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Define architecture independent SPRs
//-----------------------------------------------------------------------------

#define DEC        22
#define SRR0       26
#define SRR1       27
#define SPRG0      272
#define SPRG1      273
#define SPRG2      274
#define SPRG3      275

#define SPR_SRR1   SRR1

//- User Mode Registers -------------------------------------------------------
/// Count Register
#define CTR           0x009
/// Link Register
#define LR            0x008
/// Fixed Point Exception Register
#define XER           0x001

//- Special Purpose Registers General -----------------------------------------
/// SPR General 4
#define SPRG4           0x114
/// SPR General 5
#define SPRG5           0x115
/// SPR General 6
#define SPRG6           0x116
/// SPR General 7
#define SPRG7           0x117
/// SPR General 4 (Read-only)
#define SPRG4_RO        0x104
/// SPR General 5 (Read-only)
#define SPRG5_RO        0x105
/// SPR General 6 (Read-only)
#define SPRG6_RO        0x106
/// SPR General 7 (Read-only)
#define SPRG7_RO        0x107
/// User SPR General 0
#define USPRG0          0x100

//- Memory Management ---------------------------------------------------------
/// Core Configuration Register 0
#define CCR0            0x3b3
/// Data Cache Cachability Register
#define DCCR            0x3fa
/// Data Cache Write-through Register
#define DCWR            0x3ba
/// Instruction Cache Cachability Register
#define ICCR            0x3fb
/// Process ID
#define PID             0x3b1
/// Storage Guarded Register
#define SGR             0x3b9
/// Storage Little Endian Register
#define SLER            0x3bb
/// Storage User-defined 0 Register
#define SU0R            0x3bc
/// Zone Protection Register
#define ZPR             0x3b0

//- Fixed-Point Interrupts and Exceptions -------------------------------------
/// Data Error Address Register
#define DEAR            0x3d5
/// Exception Syndrome Register
#define ESR             0x3d4
/// Exception Vector Prefix Register
#define EVPR            0x3d6
/// Save/Restore Register 2
#define SRR2            0x3de
/// Save/Restore Register 3
#define SRR3            0x3df

//- Debugging -----------------------------------------------------------------
/// Data Address Compare 1
#define DAC1            0x3f6
/// Data Address Compare 2
#define DAC2            0x3f7
/// Debug Control Register 0
#define DBCR0           0x3f2
/// Debug Control Register 1
#define DBCR1           0x3bd
/// Debug Status Register
#define DBSR            0x3f0
/// Data Value Compare 1
#define DVC1            0x3b6
/// Data Value Compare 2
#define DVC2            0x3b7
/// Instruction Address Compare 1
#define IAC1            0x3f4
/// Instruction Address Compare 2
#define IAC2            0x3f5
/// Instruction Address Compare 3
#define IAC3            0x3b4
/// Instruction Address Compare 4
#define IAC4            0x3b4
/// Instruction Cache Debug Data Register
#define ICDBDR          0x3d3

//- Timer Facilities ----------------------------------------------------------
/// Programmable Interval Timer
#define PIT            0x3db
/// Time Base Lower
#define TBL            0x11c
/// Time Base Upper
#define TBU            0x11d
/// Time Base Lower (Read-only)
#define TBL_RO         0x10c
/// Time Base Upper (Read-only)
#define TBU_RO         0x10d
/// Timer Control Register
#define TCR            0x3da
/// Timer Status Register
#define TSR            0x3d8

//- Miscellaneous -------------------------------------------------------------
/// Processor Version Register
#define PVR            0x11f

//-----------------------------------------------------------------------------
// Abbreviations for quickly reading from or writing to SPRs
//-----------------------------------------------------------------------------
#define ppc_mfccr0()       ppc_mfspr(CCR0)
#define ppc_mfsprg4()      ppc_mfspr(SPRG4)
#define ppc_mfsprg5()      ppc_mfspr(SPRG5)
#define ppc_mfsprg6()      ppc_mfspr(SPRG6)
#define ppc_mfsprg7()      ppc_mfspr(SPRG7)
#define ppc_mfpid()        ppc_mfspr(PID)
#define ppc_mfzpr()        ppc_mfspr(ZPR)
#define ppc_mfdccr()       ppc_mfspr(DCCR)
#define ppc_mfdcwr()       ppc_mfspr(DCWR)
#define ppc_mficcr()       ppc_mfspr(ICCR)
#define ppc_mfsgr()        ppc_mfspr(SGR)
#define ppc_mfsler()       ppc_mfspr(SLER)
#define ppc_mfsu0r()       ppc_mfspr(SU0R)
#define ppc_mfevpr()       ppc_mfspr(EVPR)
#define ppc_mfesr()        ppc_mfspr(ESR)
#define ppc_mfdear()       ppc_mfspr(DEAR)
#define ppc_mfsrr2()       ppc_mfspr(SRR2)
#define ppc_mfsrr3()       ppc_mfspr(SRR3)
#define ppc_mfdbsr()       ppc_mfspr(DBSR)
#define ppc_mfdbcr0()      ppc_mfspr(DBCR0)
#define ppc_mfdbcr1()      ppc_mfspr(DBCR1)
#define ppc_mfdac1()       ppc_mfspr(DAC1)
#define ppc_mfdac2()       ppc_mfspr(DAC2)
#define ppc_mfdvc1()       ppc_mfspr(DVC1)
#define ppc_mfdvc2()       ppc_mfspr(DVC2)
#define ppc_mfiac1()       ppc_mfspr(IAC1)
#define ppc_mfiac2()       ppc_mfspr(IAC2)
#define ppc_mfiac3()       ppc_mfspr(IAC3)
#define ppc_mfiac4()       ppc_mfspr(IAC4)
#define ppc_mficdbr()      ppc_mfspr(ICDBR)
#define ppc_mftcr()        ppc_mfspr(TCR)
#define ppc_mftsr()        ppc_mfspr(TSR)
#define ppc_mfpit()        ppc_mfspr(PIT)

#define ppc_mtccr0(_)      ppc_mtspr(CCR0,_)
#define ppc_mtsprg4(_)     ppc_mtspr(SPRG4,_)
#define ppc_mtsprg5(_)     ppc_mtspr(SPRG5,_)
#define ppc_mtsprg6(_)     ppc_mtspr(SPRG6,_)
#define ppc_mtsprg7(_)     ppc_mtspr(SPRG7,_)
#define ppc_mtpid(_)       ppc_mtspr(PID,_)
#define ppc_mtzpr(_)       ppc_mtspr(ZPR,_)
#define ppc_mtdccr(_)      ppc_mtspr(DCCR,_)
#define ppc_mtdcwr(_)      ppc_mtspr(DCWR,_)
#define ppc_mticcr(_)      ppc_mtspr(ICCR,_)
#define ppc_mtsgr(_)       ppc_mtspr(SGR,_)
#define ppc_mtsler(_)      ppc_mtspr(SLER,_)
#define ppc_mtsu0r(_)      ppc_mtspr(SU0R,_)
#define ppc_mtevpr(_)      ppc_mtspr(EVPR,_)
#define ppc_mtesr(_)       ppc_mtspr(ESR,_)
#define ppc_mtdear(_)      ppc_mtspr(DEAR,_)
#define ppc_mtsrr2(_)      ppc_mtspr(SRR2,_)
#define ppc_mtsrr3(_)      ppc_mtspr(SRR3,_)
#define ppc_mtdbsr(_)      ppc_mtspr(DBSR,_)
#define ppc_mtdbcr0(_)     ppc_mtspr(DBCR0,_)
#define ppc_mtdbcr1(_)     ppc_mtspr(DBCR1,_)
#define ppc_mtdac1(_)      ppc_mtspr(DAC1,_)
#define ppc_mtdac2(_)      ppc_mtspr(DAC2,_)
#define ppc_mtdvc1(_)      ppc_mtspr(DVC1,_)
#define ppc_mtdvc2(_)      ppc_mtspr(DVC2,_)
#define ppc_mtiac1(_)      ppc_mtspr(IAC1,_)
#define ppc_mtiac2(_)      ppc_mtspr(IAC2,_)
#define ppc_mtiac3(_)      ppc_mtspr(IAC3,_)
#define ppc_mtiac4(_)      ppc_mtspr(IAC4,_)
#define ppc_mticdbr(_)     ppc_mtspr(ICDBR,_)
#define ppc_mttcr(_)       ppc_mtspr(TCR,_)
#define ppc_mttsr(_)       ppc_mtspr(TSR,_)
#define ppc_mtpit(_)       ppc_mtspr(PIT,_)

//-----------------------------------------------------------------------------
// Initial Register Values
//-----------------------------------------------------------------------------

/// Initial MSR value (user mode)
#define USR_MSR_VAL         PPC_MSR_EE | PPC_MSR_ME
/// Initial MSR value (system mode)
#define SYS_MSR_VAL         PPC_MSR_ME | PPC_MSR_DE
/// Initial SRR1 value (user mode)
#define USR_SRR1_VAL        USR_MSR_VAL
/// Initial SRR1 value (system mode)
#define SYS_SRR1_VAL        SYS_MSR_VAL

//-----------------------------------------------------------------------------
// Additional Machine State Register Bits
//-----------------------------------------------------------------------------

/// Auxiliary processor available.
#define PPC_MSR_AP              (1 << 25)
/// APU exception enable.
#define PPC_MSR_APE             (1 << 19)
/// Wait state enable.
#define PPC_MSR_WE              (1 << 18)
/// Critical interrupt enable.
#define PPC_MSR_CE              (1 << 17)
/// Interrupt enable (critical and external).
#define PPC_MSR_IE              PPC_MSR_CE | PPC_MSR_EE
//#define PPC_MSR_IE                (1<<14) | (1<<16)


//-----------------------------------------------------------------------------
// Timer Control Register Bits
//-----------------------------------------------------------------------------

/// Watchdog interrupt enable.
#define PPC_TCR_WIE        (1 << 27)
/// PIT interrupt enable.
#define PPC_TCR_PIE        (1 << 26)
/// FIT interrupt enable.
#define PPC_TCR_FIE        (1 << 23)
/// Auto reload enable.
#define PPC_TCR_ARE        (1 << 22)

//-----------------------------------------------------------------------------
// Timer Status Register Bits
//-----------------------------------------------------------------------------

/// Enable next watchdog.
#define PPC_TSR_ENW        (1 << 31)
/// Watchdog interrupt status.
#define PPC_TSR_WIS        (1 << 30)
/// PIT interrupt status.
#define PPC_TSR_PIS        (1 << 27)
/// FIT interrupt status.
#define PPC_TSR_FIS        (1 << 26)

//-----------------------------------------------------------------------------
// Core Configuration Register Bits
//-----------------------------------------------------------------------------

/// Load word as line.
#define PPC_CCR0_LWL        (1 << 25)
/// Load without allocate.
#define PPC_CCR0_LWOA       (1 << 24)
/// Store without allocate.
#define PPC_CCR0_SWOA       (1 << 23)
/// DCU PLB priority bit 1.
#define PPC_CCR0_DPP1       (1 << 22)
/// ICU PLB priority bit 0
#define PPC_CCR0_IPP0       (1 << 21)
/// ICU PLB priority bit 1.
#define PPC_CCR0_IPP1       (1 << 20)
/// Enable U0 exception.
#define PPC_CCR0_U0XE       (1 << 17)
/// Load debug enable.
#define PPC_CCR0_LDBE       (1 << 16)
/// ICU prefetching for cachable regions.
#define PPC_CCR0_PFC        (1 << 11)
/// ICU prefetching for non-cachable regions.
#define PPC_CCR0_PFNC       (1 << 10)
/// Non-cachable ICU request size.
#define PPC_CCR0_NCRS       (1 << 9)
/// Fetch without allocate.
#define PPC_CCR0_FWOA       (1 << 8)
/// Cache information select.
#define PPC_CCR0_CIS        (1 << 4)
/// Cache way select.
#define PPC_CCR0_CWS        (1)

//-----------------------------------------------------------------------------
// Miscellaneous instructions
//-----------------------------------------------------------------------------

///  \brief System call.
///
///  Generates a system call exception at (EVPR || 0x0c00).
///
#define ppc_sc()        asm volatile ("sc")

///  \brief Return from critical interrupt.
///
///  Loads program counter with SRR2, machine state register with SRR3.
///
#define ppc_rfci()        asm volatile ("rfci")

///  \brief Invalidate instruction cache congruence class.
///
#define ppc_iccci()        asm volatile ("iccci 0, 0")

///  \brief Invalidate data cache congruence class.
///
#define ppc_dccci(rA)        asm volatile ("dccci %0, 0" : : "r" (rA))

///  \brief Write high portion of TLB entry.
///
#define ppc_tlbwehi(rS,rA)    asm volatile ("tlbwehi %0, %1" : : "r" (rS), "r" (rA))

///  \brief Write low portion of TLB entry.
///
#define ppc_tlbwelo(rS,rA)    asm volatile ("tlbwelo %0, %1" : : "r" (rS), "r" (rA))

///  \brief Read high portion of TLB entry.
///
#define ppc_tlbrehi(rA)        ({greg_t rT; asm volatile ("tlbrehi %0, %1" : "=r" (rT) : "r" (rA)); rT;})

///  \brief Read low portion of TLB entry.
///
#define ppc_tlbrelo(rA)        ({greg_t rT; asm volatile ("tlbrelo %0, %1" : "=r" (rT) : "r" (rA)); rT;})

///  \brief Search TLB for a valid entry.
///
#define ppc_tlbsx(rA,rB)    ({greg_t rT; asm volatile ("tlbsx. %0, %1, %2" : "=r" (rT) : "b" (rA), "r" (rB)); rT;})

#endif /* _PPC405_H */
