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

#ifndef VIRTEX4BOARD_HH_
#define VIRTEX4BOARD_HH_

#include "SCLConfig.hh"

#include Board_Processor_hh
#include Board_InterruptHandler_hh
#include Board_SHM_hh
#include Board_LED_hh
#include Board_Clock_hh
#include Board_UART_hh
#include Board_ETH_hh
#include Board_InterruptController_hh
#include  <arch/Leon3/GR-CPCI-XC4V/AISMemManager.hh>
#include <OmapGPTimer.hh>

/*!
 * \brief Implementation of the HAL board for the Virtex4 architecture
 */
class Virtex4Board {
    DEF_Board_ProcessorCfd
    DEF_Board_InterruptHandlerCfd
    DEF_Board_SHMCfd
    DEF_Board_LEDCfd
    DEF_Board_TimerCfd
    DEF_Board_ClockCfd
    DEF_Board_UARTCfd
    //DEF_Board_ETHCfd

private:
    Leon3_GRETH* ETHCfd;

public:
    void setETH(Leon3_GRETH* o) {ETHCfd = o;}
    Leon3_GRETH* getETH() { return (Leon3_GRETH*) ETHCfd; }
    DEF_Board_InterruptControllerCfd

    Virtex4Board();
    ~Virtex4Board();
};

#endif /*VIRTEX4BOARD_HH_*/
