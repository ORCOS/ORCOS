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

#include "BeagleBone.hh"
#include "kernel/Kernel.hh"
#include "inc/memio.h"

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

#define SETBITS(a, UP, LOW, val) a = ((a & ~(( (1 << (UP - LOW + 1)) -1) << LOW)) | ((val & ((1 << (UP - LOW + 1)) -1)) << LOW) )
#define writew(b, addr) (void)((*(volatile unint2 *) (addr)) = (b))

#define CM_PER      0x44E00000
#define CM_WKUP     0x44E00400
#define CM_DPLL     0x44E00500
#define CM_MPU      0x44E00600
#define CM_DEVICE   0x44E00700

#define CM_WKUP_CONTROL_CLKCTRL 0x4
#define CM_WKUP_TIMER0_CLKCTRL  0x10
#define CM_WKUP_UART0_CLKCTRL   0xB4
#define CM_WKUP_TIMER1_CLKCTRL  0xC4
#define CM_WKUP_I2C0_CLKCTRL    0xB8
#define CM_WKUP_WDT1_CLKCTRL    0xD4
#define CM_WKUP_GPIO0_CLKCTRL   0x08

#define CM_PER_MMC0_CLKCTRL     0x3C
#define CM_PER_MMC1_CLKCTRL     0xF4

#define CM_PER_GPIO1_CLKCTRL    0xAC
#define CM_PER_GPIO2_CLKCTRL    0xB0
#define CM_PER_GPIO3_CLKCTRL    0xB4

#define CM_PER_TIMER2_CLKCTRL   0x80
#define CM_PER_TIMER3_CLKCTRL   0x84
#define CM_PER_TIMER4_CLKCTRL   0x88
#define CM_PER_TIMER5_CLKCTRL   0xec
#define CM_PER_TIMER6_CLKCTRL   0xf0
#define CM_PER_TIMER7_CLKCTRL   0x7c

#define CM_PER_CPGMAC0_CLKCTRL  0x14
#define CM_PER_CPSW_CLKCTRL     0x144

#define CM_CLKSEL_DPLL_MPU      0x2c
#define CM_DIV_M6_DPLL_CORE     0xd8
#define CM_DIV_M2_DPLL_MPU      0xa8

#define CM_CLKMODE_DPLL_MPU     0x88
#define CM_IDLEST_DPLL_MPU      0x20

#define CONTROL_MODULE          0x44E10000

#define CONF_MMC_CLK            0x900
#define CONF_GMPC_CSN0          0x87c
#define CONF_GMPC_CSN1          0x880
#define CONF_GMPC_CSN2          0x884

#define CONF_GMPC_ADVN_ALE      0x890

#define CONF_GMPC_AD0           0x800
#define CONF_GMPC_AD1           0x804
#define CONF_GMPC_AD2           0x808
#define CONF_GMPC_AD3           0x80c
#define CONF_GMPC_AD4           0x810

#define CONTROL_REVISION        0x0
#define CONTROL_HWINFO          0x4
#define CONTROL_SYSCONFIG       0x10
#define CONTROL_STATUS          0x40
#define CONTROL_FEATURE         0x604

#define CLKSEL_TIMER1MS_CLK     0x28
#define CLKSEL_TIMER2_CLK       0x8
#define CLKSEL_TIMER3_CLK       0xc
#define CLKSEL_TIMER4_CLK       0x10
#define CLKSEL_TIMER5_CLK       0x18
#define CLKSEL_TIMER6_CLK       0x1c
#define CLKSEL_TIMER7_CLK       0x4

#define WDT_WSPR (0x44E35000 + 0x48)
#define WDT_WWPS (0x44E35000 + 0x34)

/*****************************************************************************
 * ARCHITECTURE MEMORY MAPPING
 *******************************************************************************/

static t_mapping AM335xMappings[5] = {
    { 0x0,        0x0,        0x100000,  hatProtectionExecute | hatProtectionRead, 0 },                       /* AM335x ROM vectors  (1 MB) */
    { 0x40300000, 0x40300000, 0x10000,   hatProtectionExecute | hatProtectionRead | hatProtectionWrite, 0 },  /* SRAM IRQ vectors (64 KB)  */
    { 0x44C00000, 0x44C00000, 0x400000,  hatProtectionRead | hatProtectionWrite, hatCacheInhibit },           /* L4_WKUP Domain (4 MB)) */
    { 0x48000000, 0x48000000, 0x1000000, hatProtectionRead | hatProtectionWrite, hatCacheInhibit },           /* L4_PER (16 MB) */
    { 0x4a000000, 0x4a000000, 0x1000000, hatProtectionRead | hatProtectionWrite, hatCacheInhibit }            /* L4_FAST (16 MB) */
};

/* The important architecture kernel mapping structure */
t_archmappings arch_kernelmappings =
{   .count = 5,
    .mappings = AM335xMappings
};

/*****************************************************************************
 * METHODS
 *******************************************************************************/
// cppcheck-suppress uninitMemberVar
BeagleBone::BeagleBone() {
}


/*****************************************************************************
 * Method: BeagleBone::setCPUFrequency(unint4 frequency)
 *
 * @description
 *  Sets the specific frequency in MHZ.
 *******************************************************************************/
void BeagleBone::setCPUFrequency(unint4 frequency) {
    if (frequency < 10 || frequency > 1000) {
        return;
    }

    /* put clock in bypass mode */
    int clkmode = INW(CM_WKUP + CM_CLKMODE_DPLL_MPU);
    clkmode &= ~(0x7);
    clkmode = 0x4;

    OUTW(CM_WKUP + CM_CLKMODE_DPLL_MPU, 0x4);

    /* wait until in bypass mode*/
    while ((INW(CM_WKUP + CM_IDLEST_DPLL_MPU) & (1 << 8)) == 0) {
    }

    /* set m and n and m2*/
    int mpuclksel = (frequency << 8) | (23);
    OUTW(CM_WKUP + CM_CLKSEL_DPLL_MPU, mpuclksel);

    /* set m2 */
    int m2div = INW(CM_WKUP + CM_DIV_M2_DPLL_MPU);
    m2div &= ~(0x1f);
    m2div |= 1;
    OUTW(CM_WKUP + CM_DIV_M2_DPLL_MPU, m2div);

    /* try to lock pll*/
    clkmode &= ~(0x7);
    clkmode |= 0x7;
    clkmode = 0x7;
    OUTW(CM_WKUP + CM_CLKMODE_DPLL_MPU, clkmode);

    /* wait until in bypass mode*/
    while ((INW(CM_WKUP + CM_IDLEST_DPLL_MPU) & (1)) == 0) {
    }
}


/*****************************************************************************
 * Method: BeagleBone::early_init()
 *
 * @description
 *  Early initialization code. Setups basic clocks and enables UART stdout.
 *******************************************************************************/
void BeagleBone::early_init() {
    /* enable module clocks */
     OUTW(CM_WKUP + CM_WKUP_CONTROL_CLKCTRL, 0x2);
     OUTW(CM_WKUP + CM_WKUP_TIMER0_CLKCTRL, 0x2);
     OUTW(CM_WKUP + CM_WKUP_UART0_CLKCTRL, 0x2);
     /* Timer 1 for clock emulation */
     OUTW(CM_WKUP + CM_WKUP_TIMER1_CLKCTRL, 0x2);
     OUTW(CM_WKUP + CM_WKUP_I2C0_CLKCTRL, 0x2);

     /* use M_OSC clock for timer 1 */
     OUTW(CM_DPLL + CLKSEL_TIMER1MS_CLK, 0x00);
     /* use M_OSC clock for timer 2 */
     OUTW(CM_DPLL + CLKSEL_TIMER2_CLK, 0x01);

     /* disable watchdog timer .. as it is enabled by rom code..
      * or make use of the watchdog timer */
     OUTW(CM_WKUP + CM_WKUP_WDT1_CLKCTRL, 0x2);

     // disable watchdog
     OUTW(WDT_WSPR, 0xAAAA);
     while (INW(WDT_WWPS) != 0) {
     }
     OUTW(WDT_WSPR, 0x5555);
     while (INW(WDT_WWPS) != 0) {
     }

     /* created first so we can very early write to the serial console
      * to e.g. write error messages! */
 #ifdef HAS_Board_UARTCfd
     INIT_Board_UARTCfd
     UARTCfd = new NEW_Board_UARTCfd;

 #if __EARLY_SERIAL_SUPPORT__
     theOS->setStdOutputDevice(UARTCfd);
 #endif

 #endif
}

/*****************************************************************************
 * Method: BeagleBone::initialize()
 *
 * @description
 *  Initializes the BeagleBoard hardware components.
 *  Starts up functional and interface clocks of devices and
 *  initializes the device drivers.
 *
 *  Called by Kernel::initialize at startup.
 *******************************************************************************/
void BeagleBone::initialize() {
    /* on reset we will be running with 500 MHZ cpu clock*/

    /* enable module clocks */
    OUTW(CM_WKUP + CM_WKUP_CONTROL_CLKCTRL, 0x2);
    OUTW(CM_WKUP + CM_WKUP_TIMER0_CLKCTRL, 0x2);
    OUTW(CM_WKUP + CM_WKUP_UART0_CLKCTRL, 0x2);
    /* Timer 1 for clock emulation */
    OUTW(CM_WKUP + CM_WKUP_TIMER1_CLKCTRL, 0x2);
    OUTW(CM_WKUP + CM_WKUP_I2C0_CLKCTRL, 0x2);

    /* use M_OSC clock for timer 1 */
    OUTW(CM_DPLL + CLKSEL_TIMER1MS_CLK, 0x00);
    /* use M_OSC clock for timer 2 */
    OUTW(CM_DPLL + CLKSEL_TIMER2_CLK, 0x01);

    /* disable watchdog timer .. as it is enabled by rom code..
     * or make use of the watchdog timer */
    OUTW(CM_WKUP + CM_WKUP_WDT1_CLKCTRL, 0x2);

    // disable watchdog
    OUTW(WDT_WSPR, 0xAAAA);
    while (INW(WDT_WWPS) != 0) {
    }
    OUTW(WDT_WSPR, 0x5555);
    while (INW(WDT_WWPS) != 0) {
    }

    /* created first so we can very early write to the serial console
     * to e.g. write error messages! */
    LOG(ARCH, INFO, "Booting ORCOS..");
    LOG(ARCH, INFO, "BeagleBone Initializing...");
    LOG(ARCH, INFO, "BeagleBone: Board UART: [" STRINGIZE(Board_UARTCfdCl) "]");

    /* enable module clocks */
    /* timer 2 for scheduling */
    OUTW(CM_PER + CM_PER_TIMER2_CLKCTRL, 0x2);
    OUTW(CM_PER + CM_PER_GPIO1_CLKCTRL,  0x2);
    OUTW(CM_PER + CM_PER_GPIO2_CLKCTRL,  0x2);
    OUTW(CM_PER + CM_PER_GPIO3_CLKCTRL,  0x2);

    int revision = INW(CONTROL_MODULE + CONTROL_REVISION);
    LOG(ARCH, INFO, "BeagleBone: AM335x Revision %d.%d RTL v.%d", (revision >> 8) & 0x3, (revision & 0x1f), (revision >> 11) & 0x1f);
    revision = INW(CONTROL_MODULE + CONTROL_HWINFO);
    LOG(ARCH, INFO, "BeagleBone: HWINFO %x", revision);

    int devfeature = INW(CONTROL_MODULE + CONTROL_FEATURE);
    if (devfeature == 0x00FC0382) {
        LOG(ARCH, INFO, "BeagleBone: SOC: AM3352 [DEV_FEATURE: %x]", devfeature);
    }
    if (devfeature == 0x20FC0382) {
        LOG(ARCH, INFO, "BeagleBone: SOC: AM3354 [DEV_FEATURE: %x]", devfeature);
    }
    if (devfeature == 0x00FD0383) {
        LOG(ARCH, INFO, "BeagleBone: SOC: AM3356 [DEV_FEATURE: %x]", devfeature);
    }
    if (devfeature == 0x00FF0383) {
        LOG(ARCH, INFO, "BeagleBone: SOC: AM3357 [DEV_FEATURE: %x]", devfeature);
    }
    if (devfeature == 0x20FD0383) {
        LOG(ARCH, INFO, "BeagleBone: SOC: AM3358 [DEV_FEATURE: %x]", devfeature);
    }
    if (devfeature == 0x20FF0383) {
        LOG(ARCH, INFO, "BeagleBone: SOC: AM3359 [DEV_FEATURE: %x]", devfeature);
    }

    /* do not allow idle mode */
    OUTW(CONTROL_MODULE + CONTROL_SYSCONFIG, (1 << 2));

    int status = INW(CONTROL_MODULE + CONTROL_STATUS);
    LOG(ARCH, INFO, "BeagleBone: SYSBOOT[7:0]  (Boot Mode) %x", status & 0xff);
    LOG(ARCH, INFO, "BeagleBone: SYSBOOT[15:14](Crystal Frequency) %x", (status >> 22) & 0x3);
    LOG(ARCH, INFO, "BeagleBone: SYSBOOT[13:12] %x", (status >> 20) & 0x3);

    sys_clock = 26000;
    int sys_clk = (status >> 22) & 0x3;
    if (sys_clk == 0)
        sys_clock = 19200;
    else if (sys_clk == 1)
        sys_clock = 24000;
    else if (sys_clk == 2)
        sys_clock = 25000;
    else if (sys_clk == 3)
        sys_clock = 26000;

    LOG(ARCH, INFO, "BeagleBone: System Clock      = %d kHz", sys_clock);

    int mpu_dpll = INW(CM_WKUP + CM_CLKSEL_DPLL_MPU);
    int m  = (mpu_dpll >> 8) & 0x7ff;
    int n  = (mpu_dpll & 0x7f);
    int m2 = INW(CM_WKUP + CM_DIV_M2_DPLL_MPU) & 0x7f;

    /* for clk input klow
    int divm6 =  INW(CM_WKUP + CM_DIV_M6_DPLL_CORE);
    int m6 = divm6  & 0x1f;
    */

    int mpu_clock = ((sys_clock * m) / (n+1)) / (m2);

    m2 = INW(CM_WKUP + CM_DIV_M2_DPLL_MPU) & 0x7f;

    LOG(ARCH, INFO, "BeagleBone: MPU Clock        = %d kHz [m: %u, n: %u, m2: %u]", mpu_clock, m, n, m2);

    /* CORE_CLKOUTM6 == MPU Clock
    REGM4XEN='0'
    CLKOUT [M / (N+1)] * CLKINP * [1/M2]
    REGM4XEN='1'
    CLKOUT [4M / (N+1)] * CLKINP * [1/M2]
   */

#ifdef HAS_Board_InterruptControllerCfd
    LOG(ARCH, INFO, "BeagleBone: Board Interrupt Controller: [" STRINGIZE(Board_InterruptControllerCfdCl) "]");
    INIT_Board_InterruptControllerCfd
    InterruptControllerCfd = new NEW_Board_InterruptControllerCfd;
    InterruptControllerCfd->clearIRQ(1);
#endif

    // Clock
#ifdef HAS_Board_ClockCfd
    LOG(ARCH, INFO, "BeagleBone: Board Clock [" STRINGIZE(Board_ClockCfdCl) "]");
    INIT_Board_ClockCfd
    ClockCfd = new NEW_Board_ClockCfd;
    theClock = ClockCfd;  // clock is now available for other devices
#endif

    // Processor
#ifdef HAS_Board_ProcessorCfd
    LOG(ARCH, INFO, "BeagleBone: Board Processor [" STRINGIZE(Board_ProcessorCfdCl) "]");
    ProcessorCfd = new NEW_Board_ProcessorCfd;
#endif

    // Watchdog
#ifdef HAS_Board_WatchdogCfd
    WatchdogCfd = new NEW_Board_WatchdogCfd;
    //getWatchdog()->enable();
#endif

    // Timer
#ifdef HAS_Board_TimerCfd
    LOG(ARCH, INFO, "BeagleBone: Board Timer 'sched_timer' [" STRINGIZE(Board_TimerCfdCl) "]");
    INIT_Board_TimerCfd
    TimerCfd = new NEW_Board_TimerCfd;
#endif

    /* use M_OSC clock for timer 3-7 */
    OUTW(CM_DPLL + CLKSEL_TIMER3_CLK, 0x01);
    OUTW(CM_DPLL + CLKSEL_TIMER4_CLK, 0x01);
    OUTW(CM_DPLL + CLKSEL_TIMER5_CLK, 0x01);
    OUTW(CM_DPLL + CLKSEL_TIMER6_CLK, 0x01);
    OUTW(CM_DPLL + CLKSEL_TIMER7_CLK, 0x01);

    /* enable clocks for timer 3 - 7*/
    OUTW(CM_PER + CM_PER_TIMER3_CLKCTRL, 0x2);
    OUTW(CM_PER + CM_PER_TIMER4_CLKCTRL, 0x2);
    OUTW(CM_PER + CM_PER_TIMER5_CLKCTRL, 0x2);
    OUTW(CM_PER + CM_PER_TIMER6_CLKCTRL, 0x2);
    OUTW(CM_PER + CM_PER_TIMER7_CLKCTRL, 0x2);

    LOG(ARCH, INFO, "BeagleBone: Board 'timer1' [" STRINGIZE(Board_TimerCfdCl) "]");
    /* Timer 3 - 7 for user */
    T_AM335xTimer_Init initTimer;
    initTimer.Address       = 0x48042000; /* DMTIMER3*/
    initTimer.Length        = 4096;
    initTimer.Name          = "timer1";
    initTimer.INTC_IRQ      = 69;
    initTimer.INTC_Priority = 51; /* 63 highest, normal timer priority 50*/
    new AM335xTimer(&initTimer);

    LOG(ARCH, INFO, "BeagleBone: Board 'timer2' [" STRINGIZE(Board_TimerCfdCl) "]");
    initTimer.Address = 0x48044000; /* DMTIMER4*/
    initTimer.Length = 4096;
    initTimer.Name = "timer2";
    initTimer.INTC_IRQ = 92;
    initTimer.INTC_Priority = 51; /* 63 highest, normal timer priority 50*/
    new AM335xTimer(&initTimer);

    LOG(ARCH, INFO, "BeagleBone: Board 'timer3' [" STRINGIZE(Board_TimerCfdCl) "]");
    initTimer.Address = 0x48046000; /* DMTIMER5*/
    initTimer.Length = 4096;
    initTimer.Name = "timer3";
    initTimer.INTC_IRQ = 93;
    initTimer.INTC_Priority = 51; /* 63 highest, normal timer priority 50*/
    new AM335xTimer(&initTimer);

    LOG(ARCH, INFO, "BeagleBone: Board 'timer4' [" STRINGIZE(Board_TimerCfdCl) "]");
    initTimer.Address = 0x48048000; /* DMTIMER6*/
    initTimer.Length = 4096;
    initTimer.Name = "timer4";
    initTimer.INTC_IRQ = 94;
    initTimer.INTC_Priority = 51; /* 63 highest, normal timer priority 50*/
    new AM335xTimer(&initTimer);

    LOG(ARCH, INFO, "BeagleBone: Board 'timer5' [" STRINGIZE(Board_TimerCfdCl) "]");
    initTimer.Address = 0x4804a000; /* DMTIMER7*/
    initTimer.Length = 4096;
    initTimer.Name = "timer5";
    initTimer.INTC_IRQ = 95;
    initTimer.INTC_Priority = 51; /* 63 highest, normal timer priority 50*/
    new AM335xTimer(&initTimer);

#ifdef HAS_Board_CacheCfd
    LOG(ARCH, INFO, "BeagleBone: Board Cache [" STRINGIZE(Board_CacheCfdCl) "]");
    INIT_Board_CacheCfd
    CacheCfd = new NEW_Board_CacheCfd;
#endif

#ifdef HAS_Board_UART2Cfd
    LOG(ARCH, INFO, "BeagleBone: Board UART2: [" STRINGIZE(Board_UART2CfdCl) "]");
    INIT_Board_UART2Cfd
    UART2Cfd = new NEW_Board_UART2Cfd;
#endif

#ifdef HAS_Board_ExtPowerControlCfd
    LOG(ARCH, INFO, "BeagleBone: Board External Power: [" STRINGIZE(Board_ExtPowerControlCfdCl) "]");
    INIT_Board_ExtPowerControlCfd
    ExtPowerControlCfd = new NEW_Board_ExtPowerControlCfd;
#endif

#ifdef HAS_Board_SPICfd
    LOG(ARCH, INFO, "BeagleBone: Board SPI: [" STRINGIZE(Board_SPICfdCl) "]");
    INIT_Board_SPICfd
    SPICfd = new NEW_Board_SPICfd;
#endif

#ifdef HAS_Board_GPIO0Cfd
    /* enabel clock for GPIO0*/
    OUTW(CM_WKUP + CM_WKUP_GPIO0_CLKCTRL, 0x2);
    LOG(ARCH, INFO, "BeagleBone: Board GPIO0: [" STRINGIZE(Board_GPIO0CfdCl) "]");
    INIT_Board_GPIO0Cfd
    GPIO0Cfd = new NEW_Board_GPIO0Cfd;
#endif

#ifdef HAS_Board_GPIO1Cfd
    /* enable clock for GPIO1*/
    OUTW(CM_PER + CM_PER_GPIO1_CLKCTRL, 0x2);
    LOG(ARCH, INFO, "BeagleBone: Board GPIO1: [" STRINGIZE(Board_GPIO1CfdCl) "]");
    INIT_Board_GPIO1Cfd
    GPIO1Cfd = new NEW_Board_GPIO1Cfd;
#endif

#ifdef HAS_Board_GPIO2Cfd
    /* enable clock for GPIO1*/
    OUTW(CM_PER + CM_PER_GPIO2_CLKCTRL, 0x2);
    LOG(ARCH, INFO, "BeagleBone: Board GPIO2: [" STRINGIZE(Board_GPIO2CfdCl) "]");
    INIT_Board_GPIO2Cfd
    GPIO2Cfd = new NEW_Board_GPIO2Cfd;
#endif

#ifdef HAS_Board_GPIO3Cfd
    /* enable clock for GPIO1*/
    OUTW(CM_PER + CM_PER_GPIO3_CLKCTRL, 0x2);
    LOG(ARCH, INFO, "BeagleBone: Board GPIO3: [" STRINGIZE(Board_GPIO3CfdCl) "]");
    INIT_Board_GPIO3Cfd
    GPIO3Cfd = new NEW_Board_GPIO3Cfd;
#endif

    // LED Interface
#ifdef HAS_Board_LEDCfd
    LOG(ARCH, INFO, "BeagleBone: Board LED:  [" STRINGIZE(Board_LEDCfdCl) "]");
    LEDCfd = new NEW_Board_LEDCfd;
#endif

    // LED Interface
#ifdef HAS_Board_EthernetCfd
    /* enable clocks for ethernet subsystem */
    OUTW(CM_PER + CM_PER_CPGMAC0_CLKCTRL , 0x2);
    OUTW(CM_PER + CM_PER_CPSW_CLKCTRL , 0x2);

    LOG(ARCH, INFO, "BeagleBone: Ethernet: [" STRINGIZE(Board_EthernetCfdCl) "]");
    INIT_Board_EthernetCfd
    EthernetCfd = new NEW_Board_EthernetCfd;
#endif

#ifdef HAS_Board_MMC0Cfd
    /* init SD OCP clock and CLKADPI */
    OUTW(CM_PER + CM_PER_MMC0_CLKCTRL, 0x2);
    OUTW(CONTROL_MODULE + CONF_MMC_CLK, 0x20);

    LOG(ARCH, INFO, "BeagleBone: MMC0:  [" STRINGIZE(Board_MMC0CfdCl) "]");
    INIT_Board_MMC0Cfd
    MMC0Cfd = new NEW_Board_MMC0Cfd;
#endif

#ifdef HAS_Board_MMC1Cfd
    /* init SD OCP clock and CLKADPI */
    OUTW(CM_PER + CM_PER_MMC1_CLKCTRL, 0x2);

#define PULLUP (1 << 4)
#define INPUT (1<< 5)
    int led2value = 0x0;
    GPIO2Cfd->writeBytes(reinterpret_cast<char*>(&led2value), 4);
    int led1value = 0x0;
    GPIO1Cfd->writeBytes(reinterpret_cast<char*>(&led1value), 4);


    /* GPMC_SCN1 is muxed to mmc1 clk. activate loopback of clk as
     * otherwise card communication will not work*/
    OUTW(CONTROL_MODULE + CONF_GMPC_CSN0, PULLUP | INPUT | 0x7);
    OUTW(CONTROL_MODULE + CONF_GMPC_CSN1, PULLUP | INPUT | 0x2);
    OUTW(CONTROL_MODULE + CONF_GMPC_CSN2, PULLUP | INPUT | 0x2);
    OUTW(CONTROL_MODULE + CONF_GMPC_AD0, PULLUP | INPUT | 0x1);
    OUTW(CONTROL_MODULE + CONF_GMPC_AD1, PULLUP | INPUT | 0x1);
    OUTW(CONTROL_MODULE + CONF_GMPC_AD2, PULLUP | INPUT | 0x1);
    OUTW(CONTROL_MODULE + CONF_GMPC_AD3, PULLUP | INPUT | 0x1);
    OUTW(CONTROL_MODULE + CONF_GMPC_ADVN_ALE, PULLUP | INPUT | 0x7);

    /* GPIO2_0 is connected as inverted to MMC1 reset.
     * Put MMC1 out of reset! */
    //led2value = 0xffffffff;
    //GPIO2Cfd->writeBytes((char*) &led2value,4);

    led1value = 0xffffffff; //(1 << 21) | 0x1;
    GPIO1Cfd->writeBytes(reinterpret_cast<char*>(&led1value), 4);
    GPIO0Cfd->writeBytes(reinterpret_cast<char*>(&led1value), 4);
    GPIO2Cfd->writeBytes(reinterpret_cast<char*>(&led1value), 4);
    GPIO3Cfd->writeBytes(reinterpret_cast<char*>(&led1value), 4);

    LOG(ARCH, INFO, "BeagleBone: MMC1:  [" STRINGIZE(Board_MMC1CfdCl) "]");
    INIT_Board_MMC1Cfd
    MMC1Cfd = new NEW_Board_MMC1Cfd;
#endif


#ifdef HAS_Board_USB_HCCfd
    LOG(ARCH, INFO, "BeagleBone: USB Host-Controller: [" STRINGIZE(Board_USB_HCCfdCl) "]");
    INIT_Board_USB_HCCfd
    USB_HCCfd = new NEW_Board_USB_HCCfd;

    /* enable the EHCI interrupt source inside the MPU interrupt controller */
    InterruptControllerCfd->setIRQPriority(77, 20); /* medium hardware priority */
    InterruptControllerCfd->unmaskIRQ(77);
#endif

    // InterruptHandler
#ifdef HAS_Board_InterruptHandlerCfd
    LOG(ARCH, INFO, "BeagleBone: Board Interrupt Handler: [" STRINGIZE(Board_InterruptHandlerCfdCl) "]");
    InterruptHandlerCfd = new NEW_Board_InterruptHandlerCfd;
#endif
}

BeagleBone::~BeagleBone() {
}
