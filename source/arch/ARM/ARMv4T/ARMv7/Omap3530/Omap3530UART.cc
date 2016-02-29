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

#include <Omap3530UART.hh>
#include "inc/memio.h"
//#include "kernel/Kernel.hh"
#include "OMAP3530.h"

// Initialize the module.. only has an effect if the device is used in userspace
// code is added for user space initialization, argument passing memory regions are initialized.
ORCOS_MODULE_INIT(Omap3530UART, 256)

Omap3530UART::~Omap3530UART() {
}

Omap3530UART::Omap3530UART(T_Omap3530UART_Init* init) :
        CharacterDevice(false, init->Name) {
    // be sure our clock generation is enabled!

    baseAddr = init->Address;
    //char read_byte = 0;

    // configure and enable interrupts within interrupt controller
    //OUTW(MPU_INTCPS_ILR(74), (INW(MPU_INTCPS_ILR(74)) & ~ 0x1 )); // normal irq
    //OUTW(MPU_INTCPS_ILR(74), (INW(MPU_INTCPS_ILR(74)) & ~ 0xFC )); // priority 0
    //OUTW(MPU_INTCPS_MIR_CLEAR(2), 0x400 ); // enable interrupt

    // try the soft reset
    OUTW(baseAddr + 0x54, 0x2);

    while (!INW(baseAddr + 0x58)) {
    }

    // disable fifo
    OUTW(baseAddr + UART_FCR_REG_OFFSET, 0x0000);

    // disable mode select on uart1 to access DLL and DLH
    OUTW(baseAddr + UART_MDR1_REG_OFFSET, 0x7);

    // Switch to register configuration mode B
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x00BF);

    // Enable access to UARTi.IER_REG[7:4]: set ENHANCED_EN to 1
    OUTW(baseAddr + UART_EFR_REG_OFFSET, (INW(baseAddr + UART_EFR_REG_OFFSET) | UART_EFR_ENHANCED_EN));

    // Switch to register operational mode to access the UARTi.IER_REG register
    //OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x0000);
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x0080);

    // disable all fifo stuff
    OUTW(baseAddr + UART_FCR_REG_OFFSET, 0x0000);

    // Clear the UARTi.IER_REG. Set UARTi.IER_REG to 0x0000
    OUTW(baseAddr + UART_IER_REG_OFFSET, 0x0000);

    // Grant Access to DLL and DLH Registers
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x0080);

    // for baud rate 9600: set DLH and DLL to 0x01, 0x38
    // for baud rate 115200: set DLH and DLL to 0x00, 0x1A
    OUTW(baseAddr + UART_DLH_REG_OFFSET, 0x00);
    OUTW(baseAddr + UART_DLL_REG_OFFSET, 0x1A);

    //Switch to register operational mode to access the UARTi.IER_REG register
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x0000);
    enableIRQ(init->INTC_IRQ);

    // Switch to register configuration mode B
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x00BF);

    // Disable access to UARTi.IER_REG[7:4]: set ENHANCED_EN to 0
    OUTW(baseAddr + UART_EFR_REG_OFFSET, (INW(baseAddr + UART_EFR_REG_OFFSET) & ~UART_EFR_ENHANCED_EN));

    // Switch to register operational mode
    OUTW(baseAddr + UART_LCR_REG_OFFSET, (INW(baseAddr + UART_LCR_REG_OFFSET) & ~(UART_LCR_DIV_EN | UART_LCR_BREAK_EN)));

    // set protocol format: no parity, 1 stop bit, char length 8 bit
    OUTW(baseAddr + UART_LCR_REG_OFFSET, ((INW(baseAddr + UART_LCR_REG_OFFSET) | UART_LCR_CHAR_LENGTH) & ~(UART_LCR_PARITY_EN | UART_LCR_NB_STOP)));

    // set uart1 MODE_SELECT to uart16x mode (0x0)
    OUTW(baseAddr + UART_MDR1_REG_OFFSET, 0x0);

    OUTW(baseAddr + UART_FCR_REG_OFFSET, 0x00);

    disableIRQ(init->INTC_IRQ);
}

/*****************************************************************************
 * Method: Omap3530UART::enableIRQ()
 *
 * @description
 *  Enables IRQ generation from the UART
 *******************************************************************************/
ErrorT Omap3530UART::enableIRQ(int irq) {
    // enable all irqs within UART module
    OUTW(baseAddr + UART_IER_REG_OFFSET, (INW(baseAddr + UART_IER_REG_OFFSET) | UART_IER_MODEM | UART_IER_LINE | UART_IER_THR | UART_IER_RHR));
    return (cOk );
}

/*****************************************************************************
 * Method: Omap3530UART::disableIRQ()
 *
 * @description
 *  Disables IRQ generation from the UART
 *******************************************************************************/
ErrorT Omap3530UART::disableIRQ(int irq) {
    // disable all irqs
    OUTW(baseAddr + UART_IER_REG_OFFSET, (INW(baseAddr + UART_IER_REG_OFFSET) & ~(UART_IER_MODEM | UART_IER_LINE | UART_IER_THR | UART_IER_RHR)));
    return (cOk );
}

/*****************************************************************************
 * Method: Omap3530UART::readBytes(char* bytes, unint4 &length)
 *
 * @description
 *  Read bytes from the UART FIFO
 *******************************************************************************/
ErrorT Omap3530UART::readBytes(char* bytes, unint4 &length) {
    // This method polls (no waiting) for new data
    ErrorT err;
    unint4 count;

    for (count = 0, err = cOk ; ((count < length) && (err == cOk)); count++) {
        err = inputSCC(1, reinterpret_cast<byte*>(&bytes[count]));
    }

    length = (err == cOk) ? count : count - 1;
    return (err);
}

/*****************************************************************************
 * Method: Omap3530UART::writeBytes(const char* bytes, unint4 length)
 *
 * @description
 *
 *******************************************************************************/
ErrorT Omap3530UART::writeBytes(const char* bytes, unint4 length) {
    // This method sends the bytes and waits whenever the queue is full.
    unint count;
    ErrorT err;

    for (count = 0, err = cOk ; (count < length) && (err == cOk ); count++)
        err = outputSCC(-1, bytes[count]);

    return (err);
}

/*****************************************************************************
 * Method: BeagleBoardUART::handleIRQ()
 *
 * @description
 *
 *******************************************************************************/
ErrorT Omap3530UART::handleIRQ(int irq) {
    return (cError );
}

/*****************************************************************************
 * Method: BeagleBoardUART::recvByte()
 *
 * @description
 *
 *******************************************************************************/
byte Omap3530UART::recvByte() {
    return IN8(baseAddr + UART_RHR_REG_OFFSET);
}

/*****************************************************************************
 * Method: BeagleBoardUART::sendByte(byte Data)
 *
 * @description
 *
 *******************************************************************************/
void Omap3530UART::sendByte(byte Data) {
    OUT8(baseAddr + UART_THR_REG_OFFSET, Data);
}

/*****************************************************************************
 * Method: Omap3530UART::inputSCC(int4 Timeout, byte *c)
 *
 * @description
 *
 *******************************************************************************/
ErrorT Omap3530UART::inputSCC(int4 Timeout, byte *c) {
    int ret = cError;

    while (Timeout == -1 || Timeout--) {
        {
            if (hasPendingData()) {
                *c = recvByte(); /* get char */
                ret = cOk;
                break;
            }
        }
    } /* end while */
    return (ret);
}

/*****************************************************************************
 * Method: Omap3530UART::outputSCC(int4 Timeout, byte c)
 *
 * @description
 *
 *******************************************************************************/
ErrorT Omap3530UART::outputSCC(int4 Timeout, byte c) {
    int ret = cError;

    while (Timeout == -1 || Timeout--) {
        SMP_SPINLOCK_GET(lock);
        if (!isTransmitBufferFull()) {
            sendByte(c); /* output char */
            ret = cOk;
            break;
        }
        SMP_SPINLOCK_FREE(lock);
    } /* end while */
    return (ret);
}

/*****************************************************************************
 * Method: Omap3530UART::isTransmitBufferFull()
 *
 * @description
 *
 *******************************************************************************/
bool Omap3530UART::isTransmitBufferFull() {
    byte ret = ((IN8(baseAddr + UART_SSR_REG_OFFSET)) & 0x01);
    return (ret != 0);
}

/*****************************************************************************
 * Method: Omap3530UART::isReceiveBufferFull()
 *
 * @description
 *
 *******************************************************************************/
bool Omap3530UART::isReceiveBufferFull() {
    return (0);
}

/*****************************************************************************
 * Method: Omap3530UART::hasPendingData()
 *
 * @description
 *
 *******************************************************************************/
bool Omap3530UART::hasPendingData() {
    int ret = ((INW(baseAddr + UART_LSR_REG_OFFSET)) & 0x01);
    return (ret != 0);
}

