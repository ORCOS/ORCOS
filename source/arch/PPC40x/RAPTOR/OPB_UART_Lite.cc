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

#include <OPB_UART_Lite.hh>
#include "inc/memio.h"
#include "comm/AddressProtocol.hh"
#include <kernel/Kernel.hh>
#include "kernel/Kernel.hh"

extern Kernel* theOS;

OPB_UART_Lite::~OPB_UART_Lite() {
}

OPB_UART_Lite::OPB_UART_Lite(T_OPB_UART_Lite_Init* init) :
        CommDeviceDriver(init->Name) {
    headerFound = false;
    currentPosition = 0;
    addr = init->Address;

#if ENABLE_TXRX_LEDS
    TX_COUNT = 0;
    RX_COUNT = 0;
#endif

    // clear the RX AND TX FIFO
    volatile unsigned char *port = (unsigned char *) addr;
    OUTW(port + OPBUL_CONTROL_REG_OFFSET, OPBUL_CR_FIFO_RX_RESET);
    __asm__ volatile ("eieio");
    OUTW(port + OPBUL_CONTROL_REG_OFFSET, OPBUL_CR_FIFO_TX_RESET);
    __asm__ volatile ("eieio");

    // only enable interrupts from this device if you really use it as a communication device
    // e.g prototype 1 uses this device as a character device (direct reading and writing, no sockets)
    // enableIRQ();
}

#ifdef HAS_BoardLEDCfd
void OPB_UART_Lite::setLED( LED* led ) {
#if ENABLE_TXRX_LEDS
    leds = led;
#endif
}
#endif

ErrorT OPB_UART_Lite::enableIRQ() {
    volatile unsigned char *port = (unsigned char *) addr;
    OUTW(port + OPBUL_CONTROL_REG_OFFSET, OPBUL_CR_ENABLE_INTR);
    __asm__ volatile ("eieio");
    return cOk ;
}

ErrorT OPB_UART_Lite::disableIRQ() {
    volatile unsigned char *port = (unsigned char *) addr;
    OUTW(port + OPBUL_CONTROL_REG_OFFSET, 0);
    __asm__ volatile ("eieio");
    return cOk ;
}

ErrorT OPB_UART_Lite::readByte(char* byteptr) {
    inputSCC(1, (byte*) byteptr);
    return cOk ;
}

ErrorT OPB_UART_Lite::writeByte(char byte) {
    outputSCC(-1, byte);
    return cOk ;
}

ErrorT OPB_UART_Lite::readBytes(char* bytes, unint4 &length) {
    // This method polls (no waiting) for new data

    ErrorT err;
    unint4 count;

    for (count = 0, err = cOk ; ((count < length) && (err == cOk )); count++)
        err = inputSCC(1, (byte*) bytes + count);
    length = (err == cOk ) ? count : count - 1;
    return err;
}

ErrorT OPB_UART_Lite::writeBytes(const char* bytes, unint4 length) {
    // This method sends the bytes and waits whenever the queue is full.
    unint count;
    ErrorT err;

    for (count = 0, err = cOk ; (count < length) && (err == cOk ); count++)
        err = outputSCC(-1, bytes[count]);

    return err;

}


void OPB_UART_Lite::sendByte(byte Data) {
    volatile unsigned char *port = (unsigned char *) addr;
    OUTW(port + OPBUL_TX_FIFO_OFFSET, (word) Data);
    __asm__ volatile ("eieio");

#if ENABLE_TXRX_LEDS
    // only switch led every 4 bytes send
    if ( ++TX_COUNT % 16 == 0 ) {
        char ledval;
        leds->readByte( (char*) &ledval );
        // toggle sixth led since we send something
        if ( ( ledval & 32 ) == 32 )
        ledval &= ~( 32 );
        else
        ledval |= 32;
        leds->writeByte( ledval );
    }
#endif
}

bool OPB_UART_Lite::isTransmitBufferFull() {
    volatile unsigned char *port = (unsigned char *) addr;
    word reg = INW(port + OPBUL_STATUS_REG_OFFSET);
    __asm__ volatile ("eieio");
    return ((reg & OPBUL_SR_TX_FIFO_FULL) == OPBUL_SR_TX_FIFO_FULL);
}

bool OPB_UART_Lite::isReceiveBufferFull() {
    volatile unsigned char *port = (unsigned char *) addr;
    word reg = INW(port + OPBUL_STATUS_REG_OFFSET);
    __asm__ volatile ("eieio");
    return ((reg & OPBUL_SR_RX_FIFO_FULL) == OPBUL_SR_RX_FIFO_FULL);
}

byte OPB_UART_Lite::recvByte() {
    volatile unsigned char *port = (unsigned char *) addr;

#if ENABLE_TXRX_LEDS
    // only switch led every 4 bytes send
    if ( ++RX_COUNT % 16 == 0 ) {
        char ledval;
        leds->readByte( &ledval );
        // toggle fifth led since we received something
        if ( ( ledval & 16 ) == 16 )
        ledval &= ~( 16 );
        else
        ledval |= 16;
        leds->writeByte( ledval );
    }
#endif

    return (byte) INW(port + OPBUL_RX_FIFO_OFFSET);
}

ErrorT OPB_UART_Lite::inputSCC(int4 Timeout, byte *c) {
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

bool OPB_UART_Lite::hasPendingData() {
    volatile unsigned char *port = (unsigned char *) addr;
    word reg = INW(port + OPBUL_STATUS_REG_OFFSET);
    __asm__ volatile ("eieio");
    return ((reg & OPBUL_SR_RX_FIFO_VALID_DATA) == OPBUL_SR_RX_FIFO_VALID_DATA);
}

ErrorT OPB_UART_Lite::outputSCC(int4 Timeout, byte c) {
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




