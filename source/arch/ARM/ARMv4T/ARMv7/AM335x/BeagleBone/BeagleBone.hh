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

#ifndef BEAGLEBONE_HH_
#define BEAGLEBONE_HH_

#include "SCLConfig.hh"
#include "hal/CommDeviceDriver.hh"

#include Board_Processor_hh
#include Board_InterruptHandler_hh
#include Board_UART_hh
#include Board_UART2_hh
#include Board_InterruptController_hh
#include Board_Clock_hh
#include Board_Cache_hh
#include Board_SPI_hh
#include Board_Timer_hh
#include Board_Ethernet_hh
#include Board_GPIO0_hh
#include Board_GPIO1_hh
#include Board_GPIO2_hh
#include Board_GPIO3_hh
#include Board_MMC0_hh
#include Board_MMC1_hh

#ifndef RAM_SIZE
#define RAM_SIZE     512 MB
#endif

#define PLATFORM    PLATFORM_ARM

#define SCHED_TIMER_IRQ 68

/*!
 * \brief Implementation of the HAL board for the BeagleBoard architecture
 */
class BeagleBone {
    /*
     *  Standard configurable board components definition.
     *  Depending on SCL Config members will be initialized
     *  with a corresponding class.
     */
    DEF_Board_ProcessorCfd
    DEF_Board_InterruptHandlerCfd
    DEF_Board_UARTCfd
    DEF_Board_UART2Cfd
    DEF_Board_InterruptControllerCfd
    DEF_Board_TimerCfd
    DEF_Board_CacheCfd
    DEF_Board_ClockCfd
    DEF_Board_SPICfd
    DEF_Board_EthernetCfd
    DEF_Board_GPIO0Cfd
    DEF_Board_GPIO1Cfd
    DEF_Board_GPIO2Cfd
    DEF_Board_GPIO3Cfd
    DEF_Board_MMC0Cfd
    DEF_Board_MMC1Cfd

public:
    unint4 sys_clock;

    BeagleBone();

    ~BeagleBone();

    /*****************************************************************************
     * Method: initialize()
     *
     * @description
     *  Board intitialization. Boots up the hardware devices.
     *******************************************************************************/
    void initialize();

    /*****************************************************************************
     * Method: setCPUFrequency(unint4 frequency)
     *
     * @description
     *  Sets the specific frequency in MHZ.
     *******************************************************************************/
    void setCPUFrequency(unint4 frequency);

    /*****************************************************************************
     * Method: getBoardInfo()
     *
     * @description
     *  Returns a board informations string
     *******************************************************************************/
    const char* getBoardInfo() {
        return (" BeagleBone. SOC: AM335x\r\n");
    }

    void early_init();
};

#endif /*BEAGLEBONE_HH_*/
