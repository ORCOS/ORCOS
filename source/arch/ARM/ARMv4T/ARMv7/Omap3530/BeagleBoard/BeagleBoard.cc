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

#include "BeagleBoard.hh"
#include "kernel/Kernel.hh"
#include "inc/memio.h"

extern Kernel* theOS;

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

static t_mapping OMAP3530Mappings[4] = { { 0x0, 0x0, 0xFFFFF,
        hatProtectionExecute | hatProtectionRead, 0 }, /* OMAP3630 ROM vectors */
{ 0x40200000, 0x40200000, 0xFFFF, hatProtectionExecute | hatProtectionRead
        | hatProtectionWrite, 0 }, /* SRAM irq vectors */
{ 0x48000000, 0x48000000, 0xFFFFFF, hatProtectionRead | hatProtectionWrite,
        hatCacheInhibit }, /* MMIO */
{ 0x49000000, 0x49000000, 0xFFFFF, hatProtectionRead | hatProtectionWrite,
        hatCacheInhibit }, /* L4 peripherie */
};

/* The important architecture kernel mapping structure */
t_archmappings arch_kernelmappings =
{   .count = 4, .mappings = OMAP3530Mappings};

BeagleBoard::BeagleBoard() {

}

void BeagleBoard::initialize() {

// created first so we can very early write to the serial console
// to e.g. write error messages!
#ifdef HAS_Board_UARTCfd
    INIT_Board_UARTCfd
    UARTCfd = new NEW_Board_UARTCfd;
#if __EARLY_SERIAL_SUPPORT__
    theOS->setStdOutputDevice(UARTCfd);
#endif

#endif

    OUTW(CM_CLKSEL2_PLL, 0x0001b00c);
    OUTW(CM_CLKSEL3_PLL, 0x00000009);

    //OUTW(0x48004A00,1<<17);
    printf("CM_FCLKEN1_CORE=%x\r", INW(0x48004A00));
    printf("CM_CLKSEL_CORE=%x\r", INW(0x48004A40));
    printf("CM_CLKSEL1_PLL=%x\r", INW(0x48004D40));

    unint sys_clk = INW(REG_PRM_CLKSEL);
    printf("PRM_CLKSEL : %d\r", sys_clk);

    unint sys_clock = 26000;

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

    printf("System Clock       = %d kHz\r", sys_clock);

    unint m = (INW(CM_CLKSEL1_PLL) >> 16) & 0x7ff;
    unint n = (INW(CM_CLKSEL1_PLL) >> 8) & 0x7f;
    unint f_clkout = (sys_clock * m) / (n + 1);
    printf("f_CLKOUT           = %d kHz ", f_clkout);
    printf("[m = %d, n = %d]\r", m, n);

    unint m2 = INW(0x48004D40) >> 27;
    unint core_clock = f_clkout / m2;
    printf("CORE_CLOCK         = %d kHz ", core_clock);
    printf("[m2 = %d]\r", m2);

    m = (INW(CM_CLKSEL2_PLL) >> 8) & 0x7ff;
    n = INW(CM_CLKSEL2_PLL) & 0x7f;
    unint dpll4_clock = (sys_clock * m) / (n + 1);
    printf("DPLL4_CLOCK        = %d kHz ", dpll4_clock);
    printf("[m = %d, n = %d]\r", m, n);
    printf("DPLL4_AWON_FCLKOUT = %d kHz \r", dpll4_clock * 2);

    unint mhz96clock = dpll4_clock / (INW(CM_CLKSEL3_PLL) & 0x1f);
    printf("96Mhz Clock        = %d kHz \r", mhz96clock);
    printf("DIV_96M            = %d\r", (INW(CM_CLKSEL3_PLL) & 0x1f));

#ifdef HAS_Board_UART2Cfd
    INIT_Board_UART2Cfd
    UART2Cfd = new NEW_Board_UART2Cfd;
#endif

    // Processor
#ifdef HAS_Board_ProcessorCfd
    INIT_Board_ProcessorCfd
    ProcessorCfd = new NEW_Board_ProcessorCfd;
    printf("[K][INFO ] Board Processor [" STRINGIZE(Board_ProcessorCfdCl) "]\r");
#endif

    // Watchdog
#ifdef HAS_Board_WatchdogCfd
    INIT_Board_WatchdogCfd
    WatchdogCfd = new NEW_Board_WatchdogCfd;
    //getWatchdog()->enable();
#endif

    // Timer
#ifdef HAS_Board_TimerCfd
    INIT_Board_TimerCfd
    TimerCfd = new NEW_Board_TimerCfd;
    printf("[K][INFO ] Board Timer [" STRINGIZE(Board_TimerCfdCl) "]\r");
#endif

    // Clock
#ifdef HAS_Board_ClockCfd
    INIT_Board_ClockCfd
    ClockCfd = new NEW_Board_ClockCfd;
    printf("[K][INFO ] Board Clock [" STRINGIZE(Board_ClockCfdCl) "]\r");
#endif

#ifdef HAS_Board_USB_HCCfd
    INIT_Board_USB_HCCfd
    USB_HCCfd = new NEW_Board_USB_HCCfd;
#endif

#ifdef HAS_Board_UARTCfd
#ifdef HAS_BoardLEDCfd
    UARTCfd->setLED( LEDCfd );
#endif
#endif

#ifdef HAS_Board_ETHCfd
    ETHCfd = new NEW_Board_ETHCfd;
#endif

#ifdef HAS_Board_UART3Cfd
    UART3Cfd = new NEW_Board_UART3Cfd;
#endif

    // InterruptHandler
#ifdef HAS_Board_InterruptHandlerCfd
    InterruptHandlerCfd = new NEW_Board_InterruptHandlerCfd;
    printf("[K][INFO ] Board Interrupt Handler: [" STRINGIZE(Board_InterruptHandlerCfdCl) "]\r");
#endif

    // OPB_Interrupt_Controller
#ifdef HAS_Board_InterruptControllerCfd
    InterruptControllerCfd = new NEW_Board_InterruptControllerCfd;
    printf("[K][INFO ] Board Interrupt Controller: [" STRINGIZE(Board_InterruptControllerCfdCl) "]\r");
    InterruptControllerCfd->clearIRQ(1);
    // InterruptControllerCfd->enableIRQs();
#endif

    //printf("Starting DSS\n");
    // dss = new BeagleBoardDSS("dss");
    //dss->init();
    // theOS->setStdOutputDevice( dss );

}

BeagleBoard::~BeagleBoard() {
}
