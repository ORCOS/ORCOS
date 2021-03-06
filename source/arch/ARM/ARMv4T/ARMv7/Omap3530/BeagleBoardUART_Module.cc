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
#include "comm/AddressProtocol.hh"
#include "inc/memio.h"
#include "kernel/Kernel.hh"
#include "OMAP3530.h"

/* THIS FILE IS TODO */

//ORCOS_INIT_MODULE(BeagleBoardUART,CommDeviceDriver);

BeagleBoardUART::~BeagleBoardUART(){
}

BeagleBoardUART::BeagleBoardUART(const char* name, int4 a) : CommDeviceDriver(name) {
    baseAddr = a;
    //char read_byte = 0;

    // configure and enable interrupts within interrupt controller
    //OUTW(MPU_INTCPS_ILR(74), (INW(MPU_INTCPS_ILR(74)) & ~ 0x1 )); // normal irq
    //OUTW(MPU_INTCPS_ILR(74), (INW(MPU_INTCPS_ILR(74)) & ~ 0xFC )); // priority 0
    //OUTW(MPU_INTCPS_MIR_CLEAR(2), 0x400 ); // enable interrupt

    // disable fifo
    OUTW(baseAddr + UART_FCR_REG_OFFSET, 0x0000);

    // disable mode select on uart1 to access DLL and DLH
    OUTW(baseAddr + UART_MDR1_REG_OFFSET, 0x7);

    // Switch to register configuration mode B
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x00BF);

    // Enable access to UARTi.IER_REG[7:4]: set ENHANCED_EN to 1
    OUTW(baseAddr + UART_EFR_REG_OFFSET, (INW(baseAddr + UART_EFR_REG_OFFSET) | UART_EFR_ENHANCED_EN));

    // Switch to register operational mode to access the UARTi.IER_REG register
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x0000);

    // Clear the UARTi.IER_REG. Set UARTi.IER_REG to 0x0000
    OUTW(baseAddr + UART_IER_REG_OFFSET, 0x0000);

    // Switch to register configuration mode B
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x00BF);

    // for baud rate 9600: set DLH and DLL to 0x01, 0x38
    // for baud rate 115200: set DLH and DLL to 0x00, 0x1A
    OUTW(baseAddr + UART_DLH_REG_OFFSET, 0x00);
    OUTW(baseAddr + UART_DLL_REG_OFFSET, 0x1A);

    //Switch to register operational mode to access the UARTi.IER_REG register
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x0000);
    enableIRQ();

    // Switch to register configuration mode B
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x00BF);

    // Disable access to UARTi.IER_REG[7:4]: set ENHANCED_EN to 0
    OUTW(baseAddr + UART_EFR_REG_OFFSET, (INW(baseAddr + UART_EFR_REG_OFFSET) & ~ UART_EFR_ENHANCED_EN));

    // Switch to register operational mode
    OUTW(baseAddr + UART_LCR_REG_OFFSET, (INW(baseAddr + UART_LCR_REG_OFFSET) & ~(UART_LCR_DIV_EN | UART_LCR_BREAK_EN)));

    // set protocol format: no parity, 1 stop bit, char length 8 bit
    OUTW(baseAddr + UART_LCR_REG_OFFSET, ((INW(baseAddr + UART_LCR_REG_OFFSET) | UART_LCR_CHAR_LENGTH) & ~(UART_LCR_PARITY_EN | UART_LCR_NB_STOP)));

    // set uart1 MODE_SELECT to uart16x mode (0x0)
    OUTW(baseAddr + UART_MDR1_REG_OFFSET, 0x0);

    disableIRQ();

    /*    // Test:
     // read something
     OUT8(baseAddr + UART_FCR_REG_OFFSET, 0x02);    // RX_FIFO_CLEAR
     read_byte = IN8(baseAddr + UART_RHR_REG_OFFSET);

     // write something
     OUT8(baseAddr + UART_THR_REG_OFFSET, 0x01);

     int loops = 0;
     while (isTransmitBufferFull() )
     loops++;

     OUT8(baseAddr + UART_THR_REG_OFFSET, read_byte);

     loops = 0;
     while (isTransmitBufferFull())
     loops++;*/

#if ENABLE_TXRX_LEDS
    TX_COUNT = 0;
    RX_COUNT = 0;
#endif
}

/*****************************************************************************
 * Method: BeagleBoardUART::enableIRQ()
 *
 * @description
 *
 *******************************************************************************/
ErrorT BeagleBoardUART::enableIRQ() {
    // enable all irqs within UART module
    OUTW(baseAddr + UART_IER_REG_OFFSET, (INW(baseAddr + UART_IER_REG_OFFSET) | UART_IER_MODEM | UART_IER_LINE | UART_IER_THR | UART_IER_RHR));
    return 0;
}

/*****************************************************************************
 * Method: BeagleBoardUART::disableIRQ()
 *
 * @description
 *
 *******************************************************************************/
ErrorT BeagleBoardUART::disableIRQ() {
    // disable all irqs
    OUTW(baseAddr + UART_IER_REG_OFFSET, (INW(baseAddr + UART_IER_REG_OFFSET) & ~(UART_IER_MODEM | UART_IER_LINE | UART_IER_THR | UART_IER_RHR )));
    return 0;
}

/*****************************************************************************
 * Method: BeagleBoardUART::readBytes(char* bytes, unint4 &length)
 *
 * @description
 *
 *******************************************************************************/
ErrorT BeagleBoardUART::readBytes(char* bytes, unint4 &length) {
    // This method polls (no waiting) for new data

    ErrorT err;
    unint4 count;

    for (count = 0, err = cOk ; ((count < length) && (err == cOk )); count++)
        err = inputSCC(1, (byte*) bytes + count);
    length = (err == cOk ) ? count : count - 1;
    return err;
}

/*****************************************************************************
 * Method: BeagleBoardUART::writeBytes(const char* bytes, unint4 length)
 *
 * @description
 *
 *******************************************************************************/
ErrorT BeagleBoardUART::writeBytes(const char* bytes, unint4 length) {
    // This method sends the bytes and waits whenever the queue is full.
    unint count;
    ErrorT err;

    for (count = 0, err = cOk ; (count < length) && (err == cOk ); count++)
        err = outputSCC(-1, bytes[count]);

    return err;
}

/*****************************************************************************
 * Method: BeagleBoardUART::recvByte()
 *
 * @description
 *
 *******************************************************************************/
byte BeagleBoardUART::recvByte() {
    return IN8(baseAddr + UART_RHR_REG_OFFSET);
}

/*****************************************************************************
 * Method: BeagleBoardUART::sendByte(byte Data)
 *
 * @description
 *
 *******************************************************************************/
void BeagleBoardUART::sendByte(byte Data) {
    OUT8(baseAddr + UART_THR_REG_OFFSET, Data);
}

/*****************************************************************************
 * Method: BeagleBoardUART::isTransmitBufferFull()
 *
 * @description
 *
 *******************************************************************************/
ErrorT BeagleBoardUART::inputSCC(int4 Timeout, byte *c) {
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
    return ret;
}

/*****************************************************************************
 * Method: BeagleBoardUART::isTransmitBufferFull()
 *
 * @description
 *
 *******************************************************************************/
ErrorT BeagleBoardUART::outputSCC(int4 Timeout, byte c) {
    int ret = cError;

    while (Timeout == -1 || Timeout--) {
        {
            if (!isTransmitBufferFull()) {
                sendByte(c); /* output char */
                ret = cOk;
                break;
            }
        }
    } /* end while */
    return ret;
}

/*****************************************************************************
 * Method: BeagleBoardUART::isTransmitBufferFull()
 *
 * @description
 *
 *******************************************************************************/
bool BeagleBoardUART::isTransmitBufferFull() {
    byte ret = ((IN8(baseAddr + UART_SSR_REG_OFFSET)) & 0x01);
    return (bool) ret;
}

/*****************************************************************************
 * Method: BeagleBoardUART::isReceiveBufferFull()
 *
 * @description
 *
 *******************************************************************************/
bool BeagleBoardUART::isReceiveBufferFull() {
    return 0;
}

/*****************************************************************************
 * Method: BeagleBoardUART::hasPendingData()
 *
 * @description
 *
 *******************************************************************************/
bool BeagleBoardUART::hasPendingData() {
    int ret = ((INW(baseAddr + UART_LSR_REG_OFFSET)) & 0x01);
    return (bool) ret;
}

