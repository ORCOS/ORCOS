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

#include <QEMU_UART.hh>
#include <assembler.h>
#include "comm/AddressProtocol.hh"
#include <kernel/Kernel.hh>

extern Kernel* theOS;

QEMU_UART::~QEMU_UART() {
}

QEMU_UART::QEMU_UART(const char* name, int4 a) :
        CommDeviceDriver(name) {
    headerFound = false;
    currentPosition = 0;
    addr = a;

    // only enable interrupts from this device if you really use it as a communication device
    // e.g prototype 1 uses this device as a character device (direct reading and writing, no sockets)
    // enableIRQ();
}

ErrorT QEMU_UART::enableIRQ() {
    return cNotImplemented;
}

ErrorT QEMU_UART::disableIRQ() {
    return cNotImplemented ;
}

ErrorT QEMU_UART::readByte(char* byteptr) {
    inputSCC(1, (byte*) byteptr);
    return cOk ;
}

ErrorT QEMU_UART::writeByte(char byte) {
    outputSCC(-1, byte);
    return cOk ;
}

ErrorT QEMU_UART::readBytes(char* bytes, unint4 &length) {
    ErrorT err;
    unint4 count;

    for (count = 0, err = cOk ; ((count < length) && (err == cOk )); count++)
        err = inputSCC(1, (byte*) bytes + count);
    length = (err == cOk ) ? count : count - 1;
    return err;
}

ErrorT QEMU_UART::writeBytes(const char* bytes, unint4 length) {
    unint count;
    ErrorT err;

    for (count = 0, err = cOk ; (count < length) && (err == cOk ); count++)
        err = outputSCC(-1, bytes[count]);

    return err;
}

void QEMU_UART::sendByte(byte Data) {
    volatile unsigned char *port = (unsigned char *) addr;
    *port = (unsigned char) Data;
}

bool QEMU_UART::isTransmitBufferFull() {
    return false;
}

bool QEMU_UART::isReceiveBufferFull() {
    return false;
}

byte QEMU_UART::recvByte() {
    volatile unsigned char *port = (unsigned char *) addr;
    return (byte) * port;
}

ErrorT QEMU_UART::inputSCC(int4 Timeout, byte *c) {
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

bool QEMU_UART::hasPendingData() {
    return true;
}

ErrorT QEMU_UART::outputSCC(int4 Timeout, byte c) {
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


