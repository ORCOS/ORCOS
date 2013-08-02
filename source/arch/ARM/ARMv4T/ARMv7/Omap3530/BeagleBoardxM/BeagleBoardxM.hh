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
#include Board_LED_hh
#include Board_UART2_hh
#include Board_InterruptController_hh
#include Board_Timer_hh
#include Board_Clock_hh
#include Board_USB_HC_hh
#include Board_GPIO1_hh
#include Board_GPIO2_hh
#include Board_GPIO3_hh
#include Board_GPIO4_hh
#include Board_GPIO5_hh
#include Board_GPIO6_hh


//#include Board_ETH_hh
#include "BeagleBoardDSS.hh"
/*!
 * \brief Implementation of the HAL board for the BeagleBoard architecture
 */
class BeagleBoardxM {
	DEF_Board_ProcessorCfd
	DEF_Board_InterruptHandlerCfd
    DEF_Board_UARTCfd
    DEF_Board_LEDCfd
    DEF_Board_UART2Cfd
    DEF_Board_GPIO1Cfd
    DEF_Board_GPIO2Cfd
    DEF_Board_GPIO3Cfd
    DEF_Board_GPIO4Cfd
    DEF_Board_GPIO5Cfd
    DEF_Board_GPIO6Cfd
    DEF_Board_InterruptControllerCfd
    DEF_Board_TimerCfd
    DEF_Board_ClockCfd
    DEF_Board_USB_HCCfd

    //DEF_Board_ETHCfd
    CommDeviceDriver* getETH() { return 0; }
	BeagleBoardDSS *dss;
public:

    BeagleBoardxM();
    ~BeagleBoardxM();

    //BeagleBoardDSS *dss;

    char* getBoardInfo() {return (char*) "         BeagleBoardxM revision C. SOC: DM37xx (compatible OMAP3530)\r\n"; };
};

#endif /*BEAGLEBOARD_HH_*/
