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

#ifndef RAPTORBOARD_HH_
#define RAPTORBOARD_HH_

#include "SCLConfig.hh"
#include "hal/Board.hh"

#include Board_Processor_hh
#include Board_Watchdog_hh
#include Board_InterruptHandler_hh
#include Board_LED_hh
#include Board_UART_hh
#include Board_InterruptController_hh
#include Board_Timer_hh
#include Board_Clock_hh
#include Board_ETH_hh
#include "ppc405.h"

/*!
 * \brief Implementation of the HAL board for the RAPTOR architecture
 */
class RaptorBoard : public Board {
DEF_Board_ProcessorCfd
DEF_Board_WatchdogCfd    DEF_Board_InterruptHandlerCfd
    DEF_Board_LEDCfd
    DEF_Board_UARTCfd
    DEF_Board_InterruptControllerCfd
    DEF_Board_TimerCfd
    DEF_Board_ClockCfd
    DEF_Board_ETHCfd

public:

    RaptorBoard();
    ~RaptorBoard();

    void initialize();

};

#endif /*RAPTORBOARD_HH_*/
