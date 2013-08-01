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

#ifndef _OPB_UART_LITE_HH
#define _OPB_UART_LITE_HH

#include <error.hh>
#include <types.hh>
#include <arch/shared/UART/UART.hh>
#include <hal/CommDeviceDriver.hh>
#include "../powerpc.h"

/* UART Lite register offsets */

#define OPBUL_RX_FIFO_OFFSET              0   /* receive FIFO, read only */
#define OPBUL_TX_FIFO_OFFSET              4   /* transmit FIFO, write only */
#define OPBUL_STATUS_REG_OFFSET           8   /* status register, read only */
#define OPBUL_CONTROL_REG_OFFSET          12  /* control register, write only */

/* control register bit positions */

#define OPBUL_CR_ENABLE_INTR              0x10    /* enable interrupt */
#define OPBUL_CR_FIFO_RX_RESET            0x02    /* reset receive FIFO */
#define OPBUL_CR_FIFO_TX_RESET            0x01    /* reset transmit FIFO */

/* status register bit positions */

#define OPBUL_SR_PARITY_ERROR             0x80
#define OPBUL_SR_FRAMING_ERROR            0x40
#define OPBUL_SR_OVERRUN_ERROR            0x20
#define OPBUL_SR_INTR_ENABLED             0x10    /* interrupt enabled */
#define OPBUL_SR_TX_FIFO_FULL             0x08    /* transmit FIFO full */
#define OPBUL_SR_TX_FIFO_EMPTY            0x04    /* transmit FIFO empty */
#define OPBUL_SR_RX_FIFO_FULL             0x02    /* receive FIFO full */
#define OPBUL_SR_RX_FIFO_VALID_DATA       0x01    /* data in receive FIFO */

/* the following constant specifies the size of the FIFOs, the size of the
 * FIFOs includes the transmitter and receiver such that it is the total number
 * of bytes that the UART can buffer
 */
#define OPBUL_FIFO_SIZE               16

/* Stop bits are fixed at 1. Baud, parity, and data bits are fixed on a
 * per instance basis
 */
#define OPBUL_STOP_BITS               1

/* Parity definitions
 */
#define OPBUL_PARITY_NONE             0
#define OPBUL_PARITY_ODD              1
#define OPBUL_PARITY_EVEN             2

#define COMDD_MAX_PACKET_LENGTH 50

//! Enable/Disable the usage of leds as TX/RX signalling
#define ENABLE_TXRX_LEDS 0

#if ENABLE_TXRX_LEDS
#include <arch/PPC40x/Virtex2/LED.hh>
#endif

//! MAC FRAME HEADER for the OPB_UART_Lite
struct PacketHeader {
    char packetHead[ 4 ];
    unint2 addressProtocolID;
    //char destinationAddress[4]; < no addr needed since serial devices do not possess a mac
    unint2 packetLength;
    unint2 dataChecksum;
    unint2 headerChecksum;
};

/*!
 *  \brief On-Chip Periphal Bus (OPB) UART Lite driver (for Xilinx IP).
 *
 *  This class encapsulates the UART driver implementation.
 */
class OPB_UART_Lite: public CommDeviceDriver {

private:
    volatile byte inputPending;

    int4 addr;

#if ENABLE_TXRX_LEDS
    byte TX_COUNT;
    byte RX_COUNT;
    LED* leds;
#endif

    void sendByte( byte Data );

    byte recvByte();

    bool isTransmitBufferFull();

    bool isReceiveBufferFull();

    int4 getAddr() const {
        return addr;
    }

    char packetBuffer[ COMDD_MAX_PACKET_LENGTH + sizeof(PacketHeader) ];

    unint2 currentPosition;

    bool headerFound;

protected:

public:

    //!  constructor
    OPB_UART_Lite( const char *name, int4 a );

    //!  destructor
    ~OPB_UART_Lite();

    ErrorT enableIRQ();

    ErrorT disableIRQ();

    UARTBaudRate getBaudRate() {
        return kBaud9600;
    }

#ifdef HAS_BoardLEDCfd
    void setLED( LED* led );
#endif

    /// interface to meet the CharacterDeviceDriver
    ErrorT readByte( char* byte );
    ErrorT writeByte( char byte );
    ErrorT readBytes( char *bytes, int4 &length );
    ErrorT writeBytes( const char *bytes, int4 length );

    /// interface from the CommDeviceDriver
    /// method which gets called whenver this devices throws a extern IRQ
    void recv();

    /// send method which sends the bytes given to the destination addr
    /// ignores the destination addr, len since they are not needed on a 1-1 connection
    /// as serial line connections are
    ErrorT send( char* byte, int len, char* dest_addr, int addr_len, AddressProtocol* fromProtocol );

    ErrorT broadcast( char* bytes, int len, AddressProtocol* fromProtocol );

    /// Received FIFO has vaild data. Used to distinguish between IRQs.
    bool hasPendingData();

    ///  Timeout of -1 causes it to wait indefinitely until something is read.
    ErrorT outputSCC( int4 Timeout, byte c );

    ErrorT inputSCC( int4 Timeout, byte *c );

};

#endif /* _OPB_UART_LITE_HH */
