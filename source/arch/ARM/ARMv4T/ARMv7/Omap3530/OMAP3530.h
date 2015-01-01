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

#ifndef OMAP3530_H_
#define OMAP3530_H_

#define MHZ            *1000000
#define kHZ            *1000

/*
 * PRCM Interrupt Configuration Registers
 */

#define PRM_IRQENABLE_MPU    0x4830681C    // register for enabling prm interrupts#define PRM_IRQSTATUS_MPU    0x48306818    // register with prm interrupt status
#define PRM_CLKSRC_CTRL      0x48307270    // control register for device source clock#define PRM_CLKSEL           0x48306D40    // selection of system clock frequency
#define CM_PER               0x48005000    // peripheral domain modules
#define CLKSEL_OFFSET        0x00000040    // clock selection offset
#define CM_CLKSEL_PER        CM_PER + CLKSEL_OFFSET    // peripheral clock selection#define CM_CLKSEL_WKUP       0x48004C40    //     wakeup module clock selection#define CM_CLKSEL_CORE       0x48004A40

#define CM_FCLKEN1_CORE      0x48004A00
#define CM_ICLKEN1_CORE      0x48004A10

/*
 * INTC MPU Interrupt Controller Module
 */
#define GPT1_IRQ            37
#define GPT2_IRQ            38
#define GPT10_IRQ           46

#define UART1_IRQ           72
#define UART2_IRQ           73
#define UART3_IRQ           74
#define EHCI_IRQ            77
#define MPU_INTC_BASE_ADDR        0x48200000
#define MPU_INTCPS                MPU_INTC_BASE_ADDR + 0x100
#define MPU_INTCPS_ILR(m)         MPU_INTCPS + (0x4 * (m))
#define MPU_INTCPS_CONTROL        MPU_INTC_BASE_ADDR + 0x48
#define MPU_INTCPS_SIR_IRQ        MPU_INTC_BASE_ADDR + 0x40
#define MPU_INTCPS_SYSCONFIG      MPU_INTC_BASE_ADDR + 0x10

#define MPU_INTCPS_MIR_SET(n)     MPU_INTC_BASE_ADDR + 0x08C + (0x20 * (n))
#define MPU_INTCPS_MIR_CLEAR(n)   MPU_INTC_BASE_ADDR + 0x088 + (0x20 * (n))

/*
 * Timer Module
 */

#define GPT1_BASE_ADDR    0x48318000
#define GPT2_BASE_ADDR    0x49032000
#define GPT3_BASE_ADDR    0x49034000
#define GPT5_BASE_ADDR    0x49038000
#define GPT10_BASE_ADDR   0x48086000

#define TPIR_OFFSET        0x048
#define TNIR_OFFSET        0x04C
#define TLDR_OFFSET        0x02C
#define TIER_OFFSET        0x01C
#define TISR_OFFSET        0x018
#define TCLR_OFFSET        0x024
#define TCRR_OFFSET        0x028
#define TMAR_OFFSET        0x038
#define TOCR_OFFSET        0x054
#define TOWR_OFFSET        0x058

#define TIOCP_CFG_OFFSET 0x10

#define GPT1_TIER    GPT1_BASE_ADDR + TIER_OFFSET
#define GPT1_TISR    GPT1_BASE_ADDR + TISR_OFFSET
#define GPT1_TCLR    GPT1_BASE_ADDR + TCLR_OFFSET
#define GPT1_TPIR    GPT1_BASE_ADDR + TPIR_OFFSET
#define GPT1_TNIR    GPT1_BASE_ADDR + TNIR_OFFSET
#define GPT1_TLDR    GPT1_BASE_ADDR + TLDR_OFFSET
#define GPT1_TCRR    GPT1_BASE_ADDR + TCRR_OFFSET
#define GPT1_TOCR    GPT1_BASE_ADDR + TOCR_OFFSET
#define GPT1_TOWR    GPT1_BASE_ADDR + TOWR_OFFSET
#define GPT1_TMAR    GPT1_BASE_ADDR + TMAR_OFFSET

#define GPT2_TIER    GPT2_BASE_ADDR + TIER_OFFSET
#define GPT2_TISR    GPT2_BASE_ADDR + TISR_OFFSET
#define GPT2_TCLR    GPT2_BASE_ADDR + TCLR_OFFSET
#define GPT2_TPIR    GPT2_BASE_ADDR + TPIR_OFFSET
#define GPT2_TNIR    GPT2_BASE_ADDR + TNIR_OFFSET
#define GPT2_TLDR    GPT2_BASE_ADDR + TLDR_OFFSET
#define GPT2_TCRR    GPT2_BASE_ADDR + TCRR_OFFSET
#define GPT2_TMAR    GPT2_BASE_ADDR + TMAR_OFFSET
#define GPT2_TOCR    GPT2_BASE_ADDR + TOCR_OFFSET
#define GPT2_TOWR    GPT2_BASE_ADDR + TOWR_OFFSET
#define GPT2_TIOCP_CFG    GPT2_BASE_ADDR + TIOCP_CFG_OFFSET

#define GPT10_TIER    GPT10_BASE_ADDR + TIER_OFFSET
#define GPT10_TISR    GPT10_BASE_ADDR + TISR_OFFSET
#define GPT10_TCLR    GPT10_BASE_ADDR + TCLR_OFFSET
#define GPT10_TPIR    GPT10_BASE_ADDR + TPIR_OFFSET
#define GPT10_TNIR    GPT10_BASE_ADDR + TNIR_OFFSET
#define GPT10_TLDR    GPT10_BASE_ADDR + TLDR_OFFSET
#define GPT10_TCRR    GPT10_BASE_ADDR + TCRR_OFFSET
#define GPT10_TMAR    GPT10_BASE_ADDR + TMAR_OFFSET
#define GPT10_TOCR    GPT10_BASE_ADDR + TOCR_OFFSET
#define GPT10_TOWR    GPT10_BASE_ADDR + TOWR_OFFSET

#define GPT3_TIER    GPT3_BASE_ADDR + TIER_OFFSET
#define GPT3_TISR    GPT3_BASE_ADDR + TISR_OFFSET
#define GPT3_TCLR    GPT3_BASE_ADDR + TCLR_OFFSET
#define GPT3_TPIR    GPT3_BASE_ADDR + TPIR_OFFSET
#define GPT3_TNIR    GPT3_BASE_ADDR + TNIR_OFFSET
#define GPT3_TLDR    GPT3_BASE_ADDR + TLDR_OFFSET
#define GPT3_TCRR    GPT3_BASE_ADDR + TCRR_OFFSET
#define GPT3_TOCR    GPT3_BASE_ADDR + TOCR_OFFSET
#define GPT3_TOWR    GPT3_BASE_ADDR + TOWR_OFFSET

#endif /* OMAP3530_H_ */
