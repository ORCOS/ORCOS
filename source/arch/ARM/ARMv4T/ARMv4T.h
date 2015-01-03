/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2010 University of Paderborn

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

#ifndef SOURCE_ARCH_ARM_ARMV4T_ARMV4T_H_
#define SOURCE_ARCH_ARM_ARMV4T_ARMV4T_H_

/* Program Status Register (CPSR)
 *
 * +---+---+---+---+---+-------+---+-----+---------+---------+---+---+---+---+---+--------+
 * | N | Z | C | V | Q |IT[1:0]| J | DNM | GE[3:0] | IT[7:2] | E | A | I | F | T | M[4:0] |
 * +---+---+---+---+---+-------+---+-----+---------+---------+---+---+---+---+---+--------+
 *   31  30  29  28  27  26:25   24 23:20    19:16     15:10   9   8   7   6   5    4:0
 */

#define CPSR_DISINT           0xC0    /* mask to disable all interrupts (IRQ and FIQ) */

#define CPSR_DISINTALL        0x1C0    /* mask to disable all interrupts (imprecise abort, IRQ and FIQ) */

/* Control Register (CP15, C1)
 *
 * +----+----+----+----+----+----+----+---------+----+----+----+---------+----+----+----+
 * |Res.| TE |AFE |TRE |NMFI|Res.| EE |   Res.  | V  | I  | Z  |   Res.  | C  | A  | M  |
 * +----+----+----+----+----+----+----+---------+----+----+----+---------+----+----+----+
 *   31   30   29   28   27   26   25    24:14    13   12   11     10:3    2    1    0
 */

#define CP15C1_HIGHINTVEC   0x2000    /* mask to configure high interrupt vectors */

#define CP15C1_IC_DIS       0x1000    /* mask to disable instruction cache */
#define CP15C1_DC_DIS       0x4       /* mask to disable data cache */

#define INTVEC_LOWBOUND     0xFFFF0000
#define INTVEC_HIGHBOUND    0xFFFF001C

/* Defines for switching operating modes*/
#define user_mode       16    // b10000 User#define fiq_mode        17    // b10001 FIQ#define irq_mode        18    // b10010 IRQ#define svc_mode        19    // b10011 Supervisor#define abort_mode      23    // b10111 Abort#define undef_mode      27    // b11011 Undefined#define system_mode     31    // b11111 System#define mon_mode        22    // b10110 Secure Monitor
#ifndef ARM_SUPPORT_NEON
#define ARM_SUPPORT_NEON 0
#endif

#if !ARM_SUPPORT_NEON
#define PROCESSOR_CONTEXT_SIZE     68
#else
#define PROCESSOR_CONTEXT_SIZE     324
#endif

#endif // SOURCE_ARCH_ARM_ARMV4T_ARMV4T_H_
