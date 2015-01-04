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

#include "Virtex2Board.hh"

Virtex2Board::Virtex2Board() {
    // PPC405Fx Processor
#ifdef HAS_Board_ProcessorCfd
    ProcessorCfd = new NEW_Board_ProcessorCfd;
#endif

    // PPC405Watchdog
#ifdef HAS_Board_WatchdogCfd
    WatchdogCfd = new NEW_Board_WatchdogCfd;
    //getWatchdog()->enable();
#endif

    // Timer
#ifdef HAS_Board_TimerCfd
    TimerCfd = new NEW_Board_TimerCfd;
#endif

    // Clock
#ifdef HAS_Board_ClockCfd
    ClockCfd = new NEW_Board_ClockCfd;
#endif

    // PPC405InterruptHandler
#ifdef HAS_Board_InterruptHandlerCfd
    InterruptHandlerCfd = new NEW_Board_InterruptHandlerCfd;
#endif

    // LED Interface
#ifdef HAS_Board_LEDCfd
    LEDCfd = new NEW_Board_LEDCfd;
#endif

    // OPB_UART_Lite Interace
#ifdef HAS_Board_UARTCfd
    UARTCfd = new NEW_Board_UARTCfd;
#ifdef HAS_BoardLEDCfd
    UARTCfd->setLED( LEDCfd );
#endif
#endif
}

Virtex2Board::~Virtex2Board() {
}
