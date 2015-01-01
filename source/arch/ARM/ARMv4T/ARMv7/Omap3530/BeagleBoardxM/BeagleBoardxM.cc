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

#include "BeagleBoardxM.hh"
#include "kernel/Kernel.hh"
#include "inc/memio.h"
#include "OmapIOMux.hh"

extern Kernel* theOS;
extern Board_ClockCfdCl* theClock;
/*
 * Concatenate preprocessor tokens A and B without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define PPCAT_NX(A, B) A ## B

/*
 * Concatenate preprocessor tokens A and B after macro-expanding them.
 */
#define PPCAT(A, B) PPCAT_NX(A, B)

/*
 * Turn A into a string literal without expanding macro definitions
 * (however, if invoked from a macro, macro arguments are expanded).
 */
#define STRINGIZE_NX(A) #A

/*
 * Turn A into a string literal after macro-expanding it.
 */
#define STRINGIZE(A) STRINGIZE_NX(A)

#define REG_PRM_CLKSEL 0x48306D40

#define CM_CLKSEL1_PLL 0x48004D40
#define CM_CLKSEL2_PLL 0x48004D44
#define CM_CLKSEL3_PLL 0x48004D48
#define CM_CLKSEL4_PLL 0x48004D4c
#define CM_CLKSEL5_PLL 0x48004D50
#define CM_FCLKEN_PER  0x48005000

#define SETBITS(a, UP, LOW, val) a = ((a & ~(( (1 << (UP - LOW + 1)) -1) << LOW)) | ((val & ((1 << (UP - LOW + 1)) -1)) << LOW) )
#define writew(b, addr) (void)((*(volatile unsigned short *) (addr)) = (b))

/*****************************************************************************
 * ARCHITECTURE MEMORY MAPPING
 *******************************************************************************/

static t_mapping OMAP3630Mappings[4] = {
{ 0x0       , 0x0       , 0xFFFFF , hatProtectionExecute | hatProtectionRead, 0 },                          /* OMAP3630 ROM vectors */
{ 0x40200000, 0x40200000, 0xFFFF  , hatProtectionExecute | hatProtectionRead | hatProtectionWrite, 0 },     /* SRAM IRQ vectors */
{ 0x48000000, 0x48000000, 0xFFFFFF, hatProtectionRead | hatProtectionWrite, hatCacheInhibit },              /* MMIO */
{ 0x49000000, 0x49000000, 0xFFFFF , hatProtectionRead | hatProtectionWrite, hatCacheInhibit }               /* L4 peripheries */
};

/* The important architecture kernel mapping structure */
t_archmappings arch_kernelmappings =
{   .count = 4,
    .mappings = OMAP3630Mappings
};


/*****************************************************************************
 * METHODS
 *******************************************************************************/



BeagleBoardxM::BeagleBoardxM() {
}

/*****************************************************************************
 * Method: BeagleBoardxM::initialize()
 *
 * @description
 *******************************************************************************/
void BeagleBoardxM::initialize() {
    // Initialize interface and function clock tree
    /*
     * Tables for 36XX/37XX devices
     *
     */
    //    mpu_36x_dpll_param:
    //    .word 300, 12, 0, 1
    //iva_36x_dpll_param:
    /* 26MHz */
    //    .word 10, 0, 0, 1
    //core_36x_dpll_param:
    /* 26MHz */
    // .word 200, 12, 0, 1
    //    per_36x_dpll_param:
    /*    SYSCLK    M       N      M2      M3      M4     M5      M6      m2DIV */
    //    .word 26000,    432,   12,     9,      16,     9,     4,      3,      1
    /*unint4 m = 432*2 ;
     //unint4 m = 432 ;
     unint4 n = 12;
     unint4 m2 = 9 ;
     unint4 m3 = 16 ;
     unint4 m4 = 9 ;
     unint4 m5 = 4 ;
     unint4 m6 = 3 ;

     unint4 core_m = 332;
     //unint4 core_m = 200;
     unint4 core_n = 12;*/

    // set to 26mhz sys_clk
    OUTW(REG_PRM_CLKSEL, 3);
    //uboot: 0x48306d40: 00000003

    // core clock is set by dpll3
    // also sets 54mhz to DPLL4_M3
    // 96 Mhz to CM_96M
    //unint4 cm_clksel1_val = core_n << 8 | core_m << 16 | 1 << 27;
    //OUTW(CM_CLKSEL1_PLL,cm_clksel1_val);

    // L4 @ 100 mhz , L3 @ 200 mhz
    OUTW(CM_CLKSEL1_PLL, 0x09900c00);

    // L4 @ 82.5 mhz , L3 @ 165 mhz
    //OUTW(CM_CLKSEL1_PLL,0x094a0c00);

    // uboot: 0x48004d40: 09900c00
    // core_n = 12 = 0xc
    // core_m = 400 = 0x190

    // set core L3 and L4 divider
    // CM_CLKSEL_CORE
    //OUTW(0x48004a40,2 | 2 << 2 | 1 << 12);
    //OUTW(0x48004a40,0x1f0a);

    // CM_CLKSEL_CORE
    //OUTW(CM_CLKSEL_CORE, 0x0000130a);
    /* L3 CLK is  CORE_CLK / 2, L4 CLK is CORE_CLK / 2, GPTIMER10 + 11 use SYS_CLK*/
    OUTW(CM_CLKSEL_CORE, 0x000013ca);
    OUTW(CM_CLKSEL_PER , 0xFF );

    // divide sys clock by 2! = sys_clock = 13 mhz
    //PRM_CLKSRC_CTRL
    OUTW(0x48307270, 0x80);

    // no division
    //OUTW(0x48307270,0x40);

    // controls dpll4
    //unint4 sd_val = 8; // ((26 * m / (n+1)) / 250) +1;
    //unint4 cm_clksel2_val = n | m << 8 | 0x4 << 21 | sd_val << 24;
    //OUTW(CM_CLKSEL2_PLL,cm_clksel2_val);

    OUTW(CM_CLKSEL2_PLL, 0x0483600c);
    // uboot: 0x48004d44: 0483600c
    // n = 12 = 0xc
    // m = 864 = 0x360

    // sets the divider for 96 Mhz clock: 96 Mhz = DPLL4 clock / this value
    //unint4 cm_slksel3_val = m2;
    //OUTW(CM_CLKSEL3_PLL,cm_slksel3_val);

    OUTW(CM_CLKSEL3_PLL, 0x9);
    // uboot m2 = 9

    // CM_CLKEN_PLL
    OUTW(0x48004d00, 0x370037);   // lock DPLL3

    // sets DPLL5
    //OUTW(CM_CLKSEL4_PLL,0x01bb0b);
    //OUTW(CM_CLKSEL4_PLL,(120 << 8) | 25);
    OUTW(CM_CLKSEL4_PLL, 0x0000780c);

    // 120 MHZ = DPPL5 / regival
    //OUTW(CM_CLKSEL5_PLL,0x8);
    OUTW(CM_CLKSEL5_PLL, 0x1);

    //CM_AUTOIDLE2_PLL
    OUTW(0x48004d34, 0x0);          // disable auto stop mode of dpll5
    //OUTW(0x48004d04,0x7);          // lock DPLL5

    // CM_CLKEN2_PLL
    OUTW(0x48004d04, 0x17);

    //unint4 cm_clksel_dss_val = m4 | m3 << 8;
    //OUTW(0x48004e40,cm_clksel_dss_val);
    // CM_CLKSEL_DSS
    OUTW(0x48004e40, 0x00001009);
    // uboot 0x48004e40: 00001009

    //OUTW(0x48005140,m6<<24);
    // CM_CLKSEL1_EMU
    OUTW(0x48005140, 0x03020a50);
    // uboot : 0x48005140: 03020a50

    // set clsel PLL IVA2
    OUTW(0x48004040, 0x81400);
    // uboot : 0x48004040: 00081400

    // lock iva2
    //OUTW(0x48004004,0x00000037);
    OUTW(0x48004004, 0x00000017);
    // uboot: 0x48004004: 00000017

    unint4 mpu_m = 600;      // 600 MHZ for ARM core
    unint4 mpu_n = 12;
    unint4 mpu_clk_src = 2;

    // mpu dpll = ((core_clk / (mpu_clk_src)) * mpu_m) / mpu_n
    unint4 cm_clksel1_pll_mpu_val = mpu_n | mpu_m << 8 | mpu_clk_src << 19;

    // set mpu (dpll1) divider
    // CM_CLKSEL1_PLL_MPU
    OUTW(0x48004940, cm_clksel1_pll_mpu_val);
    //OUTW(0x48004940,0x0012580c);

    //CM_CLKSEL2_PLL_MPU
    OUTW(0x48004944, 1);

    //CM_AUTOIDLE_PLL_MPU
    OUTW(0x48004934, 0);

    // lock mpu
    // CM_CLKEN_PLL_MPU
    OUTW(0x48004904, 0x00000037);

    // independently of configuration: enable gpio 5+6 functional clock and uart3
    //CM_FCLKEN_PER

    OUTW(CM_FCLKEN_PER, 0x30FF8);
    OUTW(0x48005010, 0x30FF8);

// created first so we can very early write to the serial console
// to e.g. write error messages!0
#ifdef HAS_Board_UARTCfd
    INIT_Board_UARTCfd
    UARTCfd = new NEW_Board_UARTCfd;

#if __EARLY_SERIAL_SUPPORT__
    theOS->setStdOutputDevice(UARTCfd);
#endif

    LOG(ARCH, INFO, "");
    printf_p("\rORCOS booting..\r");
    LOG(ARCH, INFO, "BeagleBoardXM Initializing...");
    LOG(ARCH, INFO, "BeagleBoardxM: Board UART: [" STRINGIZE(Board_UARTCfdCl) "]");
#endif

    unint4 cm_idlest_wkup = INW(0x48004c20);
    if ((cm_idlest_wkup & 0)) {
        LOG(ARCH, ERROR, "GPTIMER1 cannot be accessed..");
    }
    if ((cm_idlest_wkup & (1 << 2))) {
        LOG(ARCH, ERROR, "32k Sync Timer cannot be accessed..");
    }
    if ((cm_idlest_wkup & (1 << 3))) {
        LOG(ARCH, ERROR, "GPIO1 cannot be accessed..");
    }
    if ((cm_idlest_wkup & (1 << 5))) {
        LOG(ARCH, ERROR, "WDTIMER2 cannot be accessed..");
    }

    // enable clocks of most components uart1-3 i2c1-3 a.s.o
    OUTW(CM_FCLKEN1_CORE, 0x03fffe29);
    OUTW(CM_ICLKEN1_CORE, 0x3ffffffb);

    OUTW(0x48004A14, 0x0000001f);
    // CM_FCLKEN_WKUP
    /* enable wakeup domain function clocks for GPIO1 and Timer1*/
    OUTW(0x48004c00, 0x000000e9);

    //CM_ICLKEN_WKUP
    /* enable wakeup domain interface clocks for GPIO1 and Timer1*/
    OUTW(0x48004c10, 0x0000003f);

    //CM_CLKSEL_WKUP
    OUTW(0x48004c40, 0x15);
    OUTW(0x48004c30, 0x0);


    //     unint4 cm_core = INW(0x48004A40);
    //    SETBITS(cm_core,13,12,2);
    //    OUTW(0x48004A40,cm_core);

    // enable i2c pullup
    OUTW(0x48000000 + 0x2000 + 0x448, ~(0x00000001));

    LOG(ARCH, DEBUG, "BeagleBoardxM: CM_FCLKEN1_CORE  =%x", INW(CM_FCLKEN1_CORE));
    LOG(ARCH, DEBUG, "BeagleBoardxM: CM_CLKSEL_CORE   =%x", INW(0x48004A40));
    LOG(ARCH, DEBUG, "BeagleBoardxM: CM_CLKSEL1_PLL   =%x", INW(0x48004D40));
    LOG(ARCH, DEBUG, "BeagleBoardxM: CM_IDLEST2_CKGEN =%x", INW(0x48004d24));

    unint sys_clk = INW(REG_PRM_CLKSEL);
    LOG(ARCH, DEBUG, "BeagleBoardxM: PRM_CLKSEL       =%d", sys_clk);

    // TODO read sysclock / 2 bit
    sys_clock = 26000;

    if (sys_clk == 4)
        sys_clock = 38400;
    else if (sys_clk == 3)
        sys_clock = 26000;
    else if (sys_clk == 2)
        sys_clock = 19200;
    else if (sys_clk == 1)
        sys_clock = 13000;
    else if (sys_clk == 0)
        sys_clock = 12000;

    sys_clock = sys_clock >> 1;
    LOG(ARCH, INFO, "BeagleBoardxM: System Clock      = %d kHz", sys_clock);

    unint m_dpll3 = (INW(CM_CLKSEL1_PLL) >> 16) & 0x7ff;  // dpll3 multiplier
    unint n_dpll3 = (INW(CM_CLKSEL1_PLL) >> 8) & 0x7f;  // dpll3 divisor
    unint f_clkout = (sys_clock * m_dpll3) / (n_dpll3 + 1);
    LOG(ARCH, INFO,  "BeagleBoardxM: f_CLKOUT          = %d kHz ", f_clkout);
    LOG(ARCH, DEBUG, "BeagleBoardxM:                     [m = %d, n = %d]", m_dpll3, n_dpll3);

    unint m2_dpll3 = INW(CM_CLKSEL1_PLL) >> 27;
    unint core_clock = f_clkout / m2_dpll3;   // dpll3 output clock
    LOG(ARCH, INFO,  "BeagleBoardxM: CORE_CLOCK        = %d kHz ", core_clock);
    LOG(ARCH, DEBUG, "BeagleBoardxM:                     [m2 = %d]", m2_dpll3);

    //unint mpu_dpll_clock = ((core_clock / mpu_clk_src) * mpu_m) / mpu_n;
    unint mpu_dpll_clock = ((sys_clock * mpu_m * 2) / ((mpu_n + 1) * m2_dpll3));
    LOG(ARCH, INFO, "BeagleBoardxM: MPU_DPLL_CLOCK    = %d kHz [%0x]", mpu_dpll_clock, cm_clksel1_pll_mpu_val);
    LOG(ARCH, INFO, "BeagleBoardxM: ARM Core at       = %d kHz", mpu_dpll_clock / 2);

    unint4 m_dpll4 = (INW(CM_CLKSEL2_PLL) >> 8) & 0x7ff;
    unint4 n_dpll4 = INW(CM_CLKSEL2_PLL) & 0x7f;
    unint dpll4_clock = (sys_clock * m_dpll4) / (n_dpll4 + 1);
    LOG(ARCH, INFO,  "BeagleBoardxM: DPLL4_CLOCK       = %d kHz ", dpll4_clock);
    LOG(ARCH, DEBUG, "BeagleBoardxM:                     [m = %d, n = %d]", m_dpll4, n_dpll4);
    LOG(ARCH, INFO,  "BeagleBoardxM: DPLL4_AWON_FCLKOUT= %d kHz", dpll4_clock * 2);

    unint4 mhz54clock = dpll4_clock / (INW(0x48004e40) >> 8);
    LOG(ARCH, INFO, "BeagleBoardxM: 54Mhz Clock       = %d kHz", mhz54clock);

    unint mhz96clock = dpll4_clock / (INW(CM_CLKSEL3_PLL) & 0x1f);
    LOG(ARCH, INFO,  "BeagleBoardxM: 96Mhz Clock       = %d kHz ", mhz96clock);
    LOG(ARCH, DEBUG, "BeagleBoardxM: DIV_96M           = %d", (INW(CM_CLKSEL3_PLL) & 0x1f));

    unint4 m_dpll5 = (INW(CM_CLKSEL4_PLL) >> 8) & 0x7ff;
    unint4 n_dpll5 = INW(CM_CLKSEL4_PLL) & 0x7f;
    unint4 dpll5_clock = (sys_clock * m_dpll5) / (n_dpll5 + 1);
    unint mhz120clock = dpll5_clock / (INW(CM_CLKSEL5_PLL) & 0x1f);
    LOG(ARCH, INFO, "BeagleBoardxM: 120Mhz Clock      = %d kHz ", mhz120clock);

    unint4 cm_idlest1 = INW(0x48004d20);
    if (!(cm_idlest1 & 1)) {
        LOG(ARCH, ERROR, "BeagleBoardxM: ERROR: DPLL3 not locked..");
    }
    if (!(cm_idlest1 & 2)) {
        LOG(ARCH, ERROR, "BeagleBoardxM: ERROR: DPLL4 not locked..");
    }
    if (!(cm_idlest1 & 1 << 2)) {
        LOG(ARCH, ERROR, "BeagleBoardxM: ERROR: 96Mhz FCLK clock not active..");
    }
    if (!(cm_idlest1 & 1 << 3)) {
        LOG(ARCH, ERROR, "BeagleBoardxM: ERROR: 48Mhz clock not active..");
    }
    if (!(cm_idlest1 & 1 << 4)) {
        LOG(ARCH, ERROR, "BeagleBoardxM: ERROR: 12Mhz clock not active..");
    }
    if (!(cm_idlest1 & 1 << 5)) {
        LOG(ARCH, ERROR, "BeagleBoardxM: ERROR: 54Mhz clock not active..");
    }
    if (!(cm_idlest1 & 1 << 9)) {
        LOG(ARCH, ERROR, "BeagleBoardxM: ERROR: DPLL4_M2_CLK clock not active..");
    }
    if (!(cm_idlest1 & 1 << 10)) {
        LOG(ARCH, ERROR, "BeagleBoardxM: ERROR: DPLL4_M3_CLK clock not active..");
    }

    unint4 cm_idlest2 = INW(0x48004d24);
    if (!(cm_idlest2 & 1)) {
        LOG(ARCH, ERROR, "BeagleBoardxM: ERROR: DPLL5 not locked..");
    }

    // CM_IDLEST_PLL_MPU
    cm_idlest2 = INW(0x48004924);
    if (!(cm_idlest2 & 1)) {
        LOG(ARCH, ERROR, "BeagleBoardxM: ERROR: DPLL1 not locked..");
    }

    // set all mux values
    MUX_BEAGLE()

    MUX_BEAGLE_XM()

    /* Set GPIO states before they are made outputs */
    // GPIO6
    OUTW(0x49058000 + 0x94, GPIO23 | GPIO10 | GPIO8 | GPIO2 | GPIO1);
    // GPIO5
    OUTW(0x49056000 + 0x94, GPIO31 | GPIO30 | GPIO29 | GPIO28 | GPIO22 | GPIO21 | GPIO15 | GPIO14 | GPIO13 | GPIO12);

    /* Configure GPIOs to output */
    //GPIO6
    OUTW(0x49058000 + 0x34, ~(GPIO23 | GPIO10 | GPIO8 | GPIO2 | GPIO1));
    // GPIO5
    OUTW(0x49056000 + 0x34, ~(GPIO31 | GPIO30 | GPIO29 | GPIO11 | GPIO28 | GPIO22 | GPIO21 | GPIO15 | GPIO14 | GPIO13 | GPIO12));

#ifdef HAS_Board_InterruptControllerCfd
    InterruptControllerCfd = new NEW_Board_InterruptControllerCfd;
    LOG(ARCH, INFO, "BeagleBoardxM: Board Interrupt Controller: [" STRINGIZE(Board_InterruptControllerCfdCl) "]");
    InterruptControllerCfd->clearIRQ(1);
#endif

    // Clock
#ifdef HAS_Board_ClockCfd
    LOG(ARCH, INFO, "BeagleBoardxM: Board Clock [" STRINGIZE(Board_ClockCfdCl) "]");
    INIT_Board_ClockCfd
    ClockCfd = new NEW_Board_ClockCfd;
    theClock = ClockCfd;  // clock is now available for other devices
#endif

    // Processor
#ifdef HAS_Board_ProcessorCfd
    LOG(ARCH, INFO, "BeagleBoardxM: Board Processor [" STRINGIZE(Board_ProcessorCfdCl) "]");
    ProcessorCfd = new NEW_Board_ProcessorCfd;
#endif

    // Watchdog
#ifdef HAS_Board_WatchdogCfd
    WatchdogCfd = new NEW_Board_WatchdogCfd;
    //getWatchdog()->enable();
#endif

    // Timer
#ifdef HAS_Board_TimerCfd
    LOG(ARCH, INFO, "BeagleBoardxM: Board Timer [" STRINGIZE(Board_TimerCfdCl) "]");
    INIT_Board_TimerCfd
    TimerCfd = new NEW_Board_TimerCfd;
#endif

    /* Timer 1 - 9 for user creation */
    T_OmapGPTimer_Init initTimer;
    initTimer.Address       = 0x49034000;
    initTimer.Length        = 4096;
    initTimer.Name          = "timer1";
    initTimer.INTC_IRQ      = 39;
    initTimer.INTC_Priority = 51; /* 63 highest, normal timer priority 50*/
    new OmapGPTimer(&initTimer);

    initTimer.Address       = 0x49036000;
    initTimer.Name          = "timer2";
    initTimer.INTC_IRQ      = 40;
    new OmapGPTimer(&initTimer);

    initTimer.Address       = 0x49038000;
    initTimer.Name          = "timer3";
    initTimer.INTC_IRQ      = 41;
    new OmapGPTimer(&initTimer);

    initTimer.Address       = 0x4903a000;
    initTimer.Name          = "timer4";
    initTimer.INTC_IRQ      = 42;
    new OmapGPTimer(&initTimer);

    initTimer.Address       = 0x4903c000;
    initTimer.Name          = "timer5";
    initTimer.INTC_IRQ      = 43;
    new OmapGPTimer(&initTimer);

    initTimer.Address       = 0x4903e000;
    initTimer.Name          = "timer6";
    initTimer.INTC_IRQ      = 44;
    new OmapGPTimer(&initTimer);

    initTimer.Address       = 0x49040000;
    initTimer.Name          = "timer7";
    initTimer.INTC_IRQ      = 45;
    new OmapGPTimer(&initTimer);

#ifdef HAS_Board_CacheCfd
    LOG(ARCH, INFO, "BeagleBoardxM: Board Cache [" STRINGIZE(Board_CacheCfdCl) "]");
    INIT_Board_CacheCfd
    CacheCfd = new NEW_Board_CacheCfd;
#endif

#ifdef HAS_Board_UART2Cfd
    INIT_Board_UART2Cfd
    UART2Cfd = new NEW_Board_UART2Cfd;
    LOG(ARCH, INFO, "BeagleBoardxM: Board UART2: [" STRINGIZE(Board_UART2CfdCl) "]");
#endif

#ifdef HAS_Board_ExtPowerControlCfd
    INIT_Board_ExtPowerControlCfd
    ExtPowerControlCfd = new NEW_Board_ExtPowerControlCfd;
    LOG(ARCH, INFO, "BeagleBoardxM: Board External Power: [" STRINGIZE(Board_ExtPowerControlCfdCl) "]");
#endif

#ifdef HAS_Board_SPICfd
    INIT_Board_SPICfd
    SPICfd = new NEW_Board_SPICfd;
    LOG(ARCH, INFO, "BeagleBoardxM: Board SPI: [" STRINGIZE(Board_SPICfdCl) "]");
#endif

    // LED Interface
#ifdef HAS_Board_LEDCfd
    LOG(ARCH, INFO, "BeagleBoardxM: Board LED: '" Board_LED_NAME "' [" STRINGIZE(Board_LEDCfdCl) "]");
    LEDCfd = new NEW_Board_LEDCfd;
#endif

#ifdef HAS_Board_MMCCfd
    LOG(ARCH, INFO, "BeagleBoardxM: MMC-Controller: [" STRINGIZE(Board_MMCCfdCl) "]");
    INIT_Board_MMCCfd
    MMCCfd = new NEW_Board_MMCCfd;
#endif

#ifdef HAS_Board_USB_HCCfd
    LOG(ARCH, INFO, "BeagleBoardxM: USB Host-Controller: [" STRINGIZE(Board_USB_HCCfdCl) "]");
    INIT_Board_USB_HCCfd
    USB_HCCfd = new NEW_Board_USB_HCCfd;

    /* enable the EHCI interrupt source inside the MPU interrupt controller */
    InterruptControllerCfd->setIRQPriority(77, 20); /* medium hardware priority */
    InterruptControllerCfd->unmaskIRQ(77);
#endif

#ifdef HAS_Board_ETHCfd
    ETHCfd = new NEW_Board_ETHCfd;
#endif

#ifdef HAS_Board_GPIO1Cfd
    INIT_Board_GPIO1Cfd
    GPIO1Cfd = new NEW_Board_GPIO1Cfd;
#endif

#ifdef HAS_Board_GPIO2Cfd
    INIT_Board_GPIO2Cfd
    GPIO2Cfd = new NEW_Board_GPIO2Cfd;
#endif

#ifdef HAS_Board_GPIO3Cfd
    INIT_Board_GPIO3Cfd
    GPIO3Cfd = new NEW_Board_GPIO3Cfd;
#endif

#ifdef HAS_Board_GPIO4Cfd
    INIT_Board_GPIO4Cfd
    GPIO4Cfd = new NEW_Board_GPIO4Cfd;
#endif

#ifdef HAS_Board_GPIO5Cfd
    INIT_Board_GPIO5Cfd
    GPIO5Cfd = new NEW_Board_GPIO5Cfd;
#endif

#ifdef HAS_Board_GPIO6Cfd
    INIT_Board_GPIO6Cfd
    GPIO6Cfd = new NEW_Board_GPIO6Cfd;
#endif

#ifdef HAS_Board_UART3Cfd
    UART3Cfd = new NEW_Board_UART3Cfd;
#endif

    // InterruptHandler
#ifdef HAS_Board_InterruptHandlerCfd
    InterruptHandlerCfd = new NEW_Board_InterruptHandlerCfd;
    LOG(ARCH, INFO, "BeagleBoardxM: Board Interrupt Handler: [" STRINGIZE(Board_InterruptHandlerCfdCl) "]");
#endif

    /* create and register IO mux device to allow user space processes to change
     * port multiplex settings */
    new OmapIOMux();

    unint prod_id = INW(0x4830A20C);
    LOG(ARCH, INFO, "BeagleBoardxM: OMAP Product ID : %x", prod_id);

#ifdef HAS_Board_DSSCfd
    LOG(ARCH, INFO, "Starting Display Subsystem (DSS) LCD Output.");
    INIT_Board_DSSCfd;
    DSSCfd = new NEW_Board_DSSCfd;
    DSSCfd->init();
#endif
}

BeagleBoardxM::~BeagleBoardxM() {
}
