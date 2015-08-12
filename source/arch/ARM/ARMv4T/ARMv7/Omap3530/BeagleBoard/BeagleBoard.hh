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

#ifndef BEAGLEBOARD_HH_
#define BEAGLEBOARD_HH_

#include "OMAP3530.h"
#include "SCLConfig.hh"
#include "hal/CommDeviceDriver.hh"

#include Board_Processor_hh
#include Board_InterruptHandler_hh
#include Board_UART_hh
#include Board_UART2_hh
#include Board_InterruptController_hh
#include Board_Clock_hh
#include Board_USB_HC_hh
#include <OmapGPTimer.hh>

#ifndef RAM_SIZE
#define RAM_SIZE     128 MB
#endif

#define PLATFORM        PLATFORM_ARM
#define SCHED_TIMER_IRQ 37
/*!
 * \brief Implementation of the HAL board for the BeagleBoard architecture
 */
class BeagleBoard {
DEF_Board_ProcessorCfd
DEF_Board_InterruptHandlerCfd
DEF_Board_UARTCfd
DEF_Board_UART2Cfd
DEF_Board_InterruptControllerCfd
DEF_Board_TimerCfd
DEF_Board_ClockCfd
DEF_Board_USB_HCCfd


public:
    BeagleBoard();
    ~BeagleBoard();

    /*****************************************************************************
     * Method: initialize()
     *
     * @description
     *  Initializes the beagleboard device drivers.
     *******************************************************************************/
    void initialize();

    /*****************************************************************************
     * Method: getBoardInfo()
     *
     * @description
     *  Returns the board information string.
     *******************************************************************************/
    char* getBoardInfo() {
        return (char*) "         BeagleBoard revision B5. SOC: OMAP3530\n\n";
    }

    void early_init() {};
};

#endif /*BEAGLEBOARD_HH_*/
