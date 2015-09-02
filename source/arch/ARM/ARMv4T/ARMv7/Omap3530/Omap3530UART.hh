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

#ifndef BEAGLEBOARDUART_HH
#define BEAGLEBOARDUART_HH

#include <error.hh>
#include <types.hh>
// include ORCOS module defines
#include "arch/modules.hh"
#include "SCLConfig.hh"

#if MODULE_IN_USERSPACE
// include for the UserSpace baseclass version of the comm device driver
#include <hal/userspace/UserSpaceCommDeviceDriver.hh>
#else
// include for the Kernel-Space baseclass version
#include <hal/CommDeviceDriver.hh>
#endif

// UART register offsets
#define UART_MDR1_REG_OFFSET       0x020
#define UART_IER_REG_OFFSET        0x004
#define UART_DLL_REG_OFFSET        0x000
#define UART_DLH_REG_OFFSET        0x004
#define UART_EFR_REG_OFFSET        0x008    // enhanced feature register#define UART_LCR_REG_OFFSET        0x00C    // line control register#define UART_THR_REG_OFFSET        0x000    // transmit hold register#define UART_RHR_REG_OFFSET        0x000    // receive hold register#define UART_FCR_REG_OFFSET        0x008    // FIFO control register#define UART_LSR_REG_OFFSET        0x014    // line status register#define UART_SSR_REG_OFFSET        0x044    // supplementary status register// UART register bit positions
// IER register
#define UART_IER_SLEEP_MODE     0x10
#define UART_IER_MODEM          0x08
#define UART_IER_LINE           0x04
#define UART_IER_THR            0x02
#define UART_IER_RHR            0x01

// EFR register
#define UART_EFR_ENHANCED_EN    0x10

// LCR register
#define UART_LCR_DIV_EN             0x80
#define UART_LCR_BREAK_EN           0x40
#define UART_LCR_PARITY_TYPE1       0x20
#define UART_LCR_PARITY_TYPE2       0x10
#define UART_LCR_PARITY_EN          0x08
#define UART_LCR_NB_STOP            0x04
#define UART_LCR_CHAR_LENGTH        0x03    // char length 8 bit
/*!
 *  \brief UART driver for OMAP3530
 *
 *  This class encapsulates the UART driver implementation.
 */
ORCOS_MODULE(Omap3530UART, CharacterDevice) {
private:
    int4 baseAddr;

    /*****************************************************************************
     * Method: sendByte(byte Data)
     *
     * @description
     *
     *******************************************************************************/
    void sendByte(byte Data);


    /*****************************************************************************
     * Method: recvByte()
     *
     * @description
     *
     *******************************************************************************/
    byte recvByte();

    /*****************************************************************************
     * Method: isTransmitBufferFull()
     *
     * @description
     *
     *******************************************************************************/
    bool isTransmitBufferFull();

    /*****************************************************************************
     * Method: isReceiveBufferFull()
     *
     * @description
     *
     *******************************************************************************/
    bool isReceiveBufferFull();

    /*****************************************************************************
     * Method: inputSCC(int4 Timeout, byte *c)
     *
     * @description
     *
     *******************************************************************************/
    ErrorT inputSCC(int4 Timeout, byte *c);

    /*****************************************************************************
     * Method: outputSCC(int4 Timeout, byte c)
     *
     * @description
     *
     *******************************************************************************/
    ErrorT outputSCC(int4 Timeout, byte c);

public:
    //!  constructor
    Omap3530UART(T_Omap3530UART_Init* init);

    //!  destructor
    ~Omap3530UART();

    /*****************************************************************************
     * Method: enableIRQ()
     *
     * @description
     *  enables Interrupt Requests of this device
     *******************************************************************************/
    ErrorT enableIRQ(int irq);

    /*****************************************************************************
     * Method: disableIRQ()
     *
     * @description
     *  disables Interrupt Requests of this device. Interrupts may still be pending.
     *******************************************************************************/
    ErrorT disableIRQ(int irq);

    /*****************************************************************************
     * Method: handleIRQ()
     *
     * @description
     *   Generic Device Driver overrides
     *******************************************************************************/
    ErrorT handleIRQ(int irq);


    /*****************************************************************************
     * Method: readBytes(char *bytes, unint4 &length)
     *
     * @description
     *
     *******************************************************************************/
    ErrorT readBytes(char *bytes, unint4 &length);

    /*****************************************************************************
     * Method: writeBytes(const char *bytes, unint4 length)
     *
     * @description
     *
     *******************************************************************************/
    ErrorT writeBytes(const char *bytes, unint4 length);


    /*****************************************************************************
     * Method: hasPendingData()
     *
     * @description
     *
     *******************************************************************************/
    bool hasPendingData();

#if MODULE_IN_USERSPACE
    /*****************************************************************************
     * Method: new
     *******************************************************************************/
    void* operator new(size_t s, void* addr) {
        return addr;
    }

    /*****************************************************************************
     * Method: delete
     *******************************************************************************/
    void operator delete(void* addr) {}
#endif
};

#endif /* BEAGLEBOARDUART_HH */
