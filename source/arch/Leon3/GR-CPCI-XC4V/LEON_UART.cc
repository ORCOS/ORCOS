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

#include "arch/Leon3/GR-CPCI-XC4V/LEON_UART.hh"
#include "comm/AddressProtocol.hh"
#include <assembler.h>
#include "kernel/Kernel.hh"

extern Kernel* theOS;

LEON_UART::~LEON_UART() {

}

LEON_UART::LEON_UART(const char* name, int4 a) : CharacterDevice(false, name) {
    headerFound = false;
    currentPosition = 0;
    baseAddr = a;
    leds = 0;

#if ENABLE_TXRX_LEDS
    TX_COUNT = 0;
    RX_COUNT = 0;
#endif

    //init scaler
    OUTW(baseAddr + UART_SCALER_REG_OFFSET, SCALE);

    //unsigned int * scaler = (unsigned int *) 0x8000010C;

    // enableIRQ();

    //enable transmitter, receiver, loop back mode and debug mode
    OUTW(baseAddr + UART_CONTROL_REG_OFFSET, UART_CR_TE | UART_CR_LB| UART_CR_RE | UART_SR_DB | UART_CR_RF | UART_CR_RI | UART_CR_FL);
    //OUTW(baseAddr + UART_CONTROL_REG_OFFSET, UART_CR_TE | UART_CR_RE | UART_CR_LB | UART_CR_FL);
}

ErrorT LEON_UART::enableIRQ() {
    OUTW(baseAddr + UART_CONTROL_REG_OFFSET, INW(baseAddr + UART_CONTROL_REG_OFFSET) | UART_CR_RI | UART_CR_TI);
    return cOk ;
}

void LEON_UART::setLED(LED* led) {
#if ENABLE_TXRX_LEDS
    leds = led;
#else
    leds = 0;
#endif
}

ErrorT LEON_UART::disableIRQ() {
    OUTW(baseAddr + UART_CONTROL_REG_OFFSET, INW(baseAddr + UART_CONTROL_REG_OFFSET) & ~(UART_CR_RI | UART_CR_TI));
    return cOk ;
}

int LEON_UART::getIRQ() {
    return 2;
}

ErrorT LEON_UART::writeBytes(const char* bytes, unint4 length) {
    // This method sends the bytes and waits whenever the queue is full.
    unint count;
    ErrorT err;

    for (count = 0, err = cOk ; (count < length) && (err == cOk ); count++)
        err = outputSCC(-1, bytes[count]);

    return err;
}

void LEON_UART::sendByte(byte Data) {
    OUTW(baseAddr + UART_DATA_REG_OFFSET, (word )Data);

#if ENABLE_TXRX_LEDS
    if (leds != 0) {
        // only switch led every 4 bytes send
        if (++TX_COUNT % 16 == 0) {
            char ledval;
            leds->readByte((char*) &ledval);
            // toggle first led since we send something
            if ((ledval & 1) == 1)
                ledval &= ~(1);
            else
                ledval |= 1;
            leds->writeByte(ledval);
        }
    }
#endif
}

bool LEON_UART::isTransmitBufferFull() {
    word reg = INW(baseAddr + UART_STATUS_REG_OFFSET);
    return ((reg & UART_SR_TF) == UART_SR_TF);
}

bool LEON_UART::isReceiveBufferFull() {
    word reg = INW(baseAddr + UART_STATUS_REG_OFFSET);
    return ((reg & UART_SR_RF) == UART_SR_RF);
}

ErrorT LEON_UART::outputSCC(int4 Timeout, byte c) {
    int ret = cError;
    unsigned int* status = (unsigned int*) 0x80000104;
    int loops = 0;

    while (!((*status) & UART_SR_TH) && (loops < 100000))
        loops++;

    sendByte(c); /* output char */

    loops = 0;
    while (!((*status) & UART_SR_TS) && (loops < 100000))
        loops++;

    return ret = cOk;
}

ErrorT LEON_UART::inputSCC(int4 Timeout, byte *c) {
    int ret = cError;

    //while (Timeout == -1 || Timeout--) {
    if (hasPendingData()) {
        *c = recvByte(); /* get char */
        ret = cOk;
        //break;
    }
    //} /* end while */
    return ret;
}


bool LEON_UART::hasPendingData() {
    word reg = INW(baseAddr + UART_STATUS_REG_OFFSET);
    return (reg & UART_SR_DR);
}

byte LEON_UART::recvByte() {
#if ENABLE_TXRX_LEDS
    if (leds != 0) {
        // only switch led every 4 bytes send
        if (++RX_COUNT % 16 == 0) {
            char ledval;
            leds->readByte(&ledval);
            // toggle fifth led since we received something
            if ((ledval & 16) == 16)
                ledval &= ~(16);
            else
                ledval |= 16;
            leds->writeByte(ledval);
        }
    }
#endif

    return (byte) INW(baseAddr + UART_DATA_REG_OFFSET);
}


ErrorT LEON_UART::readByte(char* byteptr) {
    inputSCC(1, (byte*) byteptr);
    return cOk ;
}

ErrorT LEON_UART::readBytes(char* bytes, unint4 &length) {
    // This method polls (no waiting) for new data

    ErrorT err;
    unint4 count;

    for (count = 0, err = cOk ; ((count < length) && (err == cOk )); count++)
        err = inputSCC(1, (byte*) bytes + count);
    length = (err == cOk ) ? count : count - 1;
    return err;
}

ErrorT LEON_UART::writeByte(char byte) {
    outputSCC(-1, byte);
    return cOk ;
}
