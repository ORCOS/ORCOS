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
#include Board_Timer_hh
#include Board_Clock_hh
#include Board_USB_HC_hh

#ifndef RAM_SIZE
#define RAM_SIZE 	128 MB
#endif

#define PLATFORM	PLATFORM_ARM
/*!
 * \brief Implementation of the HAL board for the BeagleBoard architecture
 */
class BeagleBoard {
DEF_Board_ProcessorCfdDEF_Board_InterruptHandlerCfdDEF_Board_UARTCfdDEF_Board_UART2CfdDEF_Board_InterruptControllerCfdDEF_Board_TimerCfdDEF_Board_ClockCfdDEF_Board_USB_HCCfd

    CommDeviceDriver* getETH() {
        return 0;
    }

public:

    BeagleBoard();
    ~BeagleBoard();

    void initialize();

    char* getBoardInfo() {
        return (char*) "         BeagleBoard revision B5. SOC: OMAP3530\n\n";
    }
    ;
};

#endif /*BEAGLEBOARD_HH_*/
