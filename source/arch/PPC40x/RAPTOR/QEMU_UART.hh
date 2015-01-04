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

#ifndef _QEMU_UART_HH
#define _QEMU_UART_HH

#include <error.hh>
#include <types.hh>
#include <hal/CommDeviceDriver.hh>
#include "../powerpc.h"

/*!
 *  \brief On-Chip Periphal Bus (OPB) UART Lite driver (for Xilinx IP).
 *
 *  This class encapsulates the UART driver implementation.
 */
class QEMU_UART: public CharacterDevice {
private:
    //! the memory mapped IO address of this device
    int4 addr;

    /*!
     * \brief put a byte into the send buffer of the device
     */
    void sendByte(byte Data);

    /*!
     * \brief read a byte out of the receive buffer
     */
    byte recvByte();

    //! checks whether the transmit buffer is full or nor
    bool isTransmitBufferFull();

    //! check whether the receive buffer is full or not
    bool isReceiveBufferFull();

    /*!
     *  \brief Method that sends data. A Timeout specifies how may times it shall retry to send
     *          it if the transmit buffer of the device is full.
     *
     *  A Timeout of -1 causes it to wait indefinitely until the transmit buffer has free space.
     */
    ErrorT outputSCC(int4 Timeout, byte c);

    /*!
     *  \brief Method that reads data. A Timeout specifies how may times it shall retry to read
     *          it if the receive buffer of the device is empty.
     *
     *  A Timeout of -1 causes it to wait indefinitely until the receive buffer has some data.
     */
    ErrorT inputSCC(int4 Timeout, byte *c);

public:
    //!  constructor
    QEMU_UART(const char *name, int4 a);

    //!  destructor
    ~QEMU_UART();

    //! enables Interrupt Requests of this device
    ErrorT enableIRQ();

    //! disables Interrupt Requests of this device. Interrupts may still be pending.
    ErrorT disableIRQ();

    //! HAL implementation  of readByte()
    ErrorT readByte(char* byte);

    //! HAL implementation of writeByte()
    ErrorT writeByte(char byte);

    //! HAL implementation of readBytes()
    ErrorT readBytes(char *bytes, unint4 &length);

    //! HAL implementation of writeBytes()
    ErrorT writeBytes(const char *bytes, unint4 length);

    //! Checks whether there is some data available inside the receive fifo.
    bool hasPendingData();
};

#endif /* _QEMU_UART_HH */
