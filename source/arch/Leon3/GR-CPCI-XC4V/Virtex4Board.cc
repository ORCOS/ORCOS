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

#include "Virtex4Board.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

Virtex4Board::Virtex4Board() {
    /* Leon3 Interrupt Controller  */
    #ifdef HAS_Board_InterruptControllerCfd
        InterruptControllerCfd = new NEW_Board_InterruptControllerCfd;
    #endif

    // LEON_UART Interface
    #ifdef HAS_Board_UARTCfd
        UARTCfd = new NEW_Board_UARTCfd;
        #ifdef HAS_Board_LEDCfd
            UARTCfd->setLED(LEDCfd);
        #endif

    #if __EARLY_SERIAL_SUPPORT__
        theOS->setStdOutputDevice( UARTCfd );
    #endif
    #endif

    // Leon3 Processor
    #ifdef HAS_Board_ProcessorCfd
        ProcessorCfd = new NEW_Board_ProcessorCfd;
    #endif

    // Timer
    #ifdef HAS_Board_TimerCfd
        TimerCfd = new NEW_Board_TimerCfd;
        InterruptControllerCfd->enableIRQ(TimerCfd->getIRQ());
    #endif

    // Clock
    #ifdef HAS_Board_ClockCfd
        ClockCfd = new NEW_Board_ClockCfd;
    #endif

    // Leon3InterruptHandler
    #ifdef HAS_Board_InterruptHandlerCfd
        InterruptHandlerCfd = new NEW_Board_InterruptHandlerCfd;
    #endif

    // LED Interface
    #ifdef HAS_Board_LEDCfd
        LEDCfd = new NEW_Board_LEDCfd;
    #endif

    // ethernet core
    #ifdef HAS_Board_ETHCfd
        ETHCfd = new NEW_Board_ETHCfd;
        int LocalNodeNr = 0;
        GET_CPU_INDEX(LocalNodeNr);
        if (LocalNodeNr == 0) {
            InterruptControllerCfd->enableIRQ(ETHCfd->getIRQ());
        }
    #endif

    // Shared Memory Communication
    #ifdef HAS_Board_SHMCfd
        SHMCfd = new NEW_Board_SHMCfd;
        InterruptControllerCfd->enableIRQ(SHMCfd->getIRQ());
    #endif
}

Virtex4Board::~Virtex4Board() {
}
