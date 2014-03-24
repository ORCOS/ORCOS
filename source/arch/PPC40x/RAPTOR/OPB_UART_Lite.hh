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

/*!
 * This constant specifies the size of the FIFOs, the size of the
 * FIFOs includes the transmitter and receiver such that it is the total number
 * of bytes that the UART can buffer
 */
#define OPBUL_FIFO_SIZE               16

/*!
 *  Stop bits are fixed at 1. Baud, parity, and data bits are fixed on a
 * per instance basis
 */
#define OPBUL_STOP_BITS               1

/*!
 *  Parity definitions
 */
#define OPBUL_PARITY_NONE             0
#define OPBUL_PARITY_ODD              1
#define OPBUL_PARITY_EVEN             2

#define COMDD_MAX_PACKET_LENGTH 50

//! Enable/Disable the usage of leds as TX/RX signalling
#define ENABLE_TXRX_LEDS 0

#include <arch/PPC40x/RAPTOR/LED.hh>

//! MAC FRAME HEADER for the OPB_UART_Lite
struct PacketHeader {
    char packetHead[ 4 ]; //< the head of the mac frame
    unint2 addressProtocolID; //< the id of the addressprotocol on top
    unint2 packetLength; //< the length of the packet
    unint2 dataChecksum; //< the checksum of the data
    unint2 headerChecksum; //< the checksum of the header
};

/*!
 *  \brief On-Chip Periphal Bus (OPB) UART Lite driver (for Xilinx IP).
 *
 *  This class encapsulates the UART driver implementation.
 */
class OPB_UART_Lite: public CommDeviceDriver {

private:

    //! the memory mapped IO address of this device
    int4 addr;

    //! the buffer the object can work on to send and receive packets
    char packetBuffer[ COMDD_MAX_PACKET_LENGTH + sizeof(PacketHeader) ];

    //! current position inside the packerBuffer
    unint2 currentPosition;

    //! indicates whether the header of a packet was already found inside the last recv() calls
    bool headerFound;

#if ENABLE_TXRX_LEDS

    //! transmit counter
    byte TX_COUNT;
    //! receive rounter
    byte RX_COUNT;
    //! led reference
    LED* leds;

#endif

    /*!
     * \brief put a byte into the send buffer of the device
     */
    void sendByte( byte Data );

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
     * 		 it if the transmit buffer of the device is full.
     *
     *  A Timeout of -1 causes it to wait indefinitely until the transmit buffer has free space.
     */
    ErrorT outputSCC( int4 Timeout, byte c );

    /*!
     *  \brief Method that reads data. A Timeout specifies how may times it shall retry to read
     * 		 it if the receive buffer of the device is empty.
     *
     *  A Timeout of -1 causes it to wait indefinitely until the receive buffer has some data.
     */
    ErrorT inputSCC( int4 Timeout, byte *c );

public:

    //!  constructor
    OPB_UART_Lite( T_OPB_UART_Lite_Init* init );

    //!  destructor
    ~OPB_UART_Lite();

    //! enables Interrupt Requests of this device
    ErrorT enableIRQ();

    //! disables Interrupt Requests of this device. Interrupts may still be pending.
    ErrorT disableIRQ();

#ifdef HAS_BoardLEDCfd
    //! sets the leds which can be used for TX/RX signalling
    void setLED( LED* led );
#endif

    //! HAL implementation  of readByte()
    ErrorT readByte( char* byte );

    //! HAL implementation of writeByte()
    ErrorT writeByte( char byte );

    //! HAL implementation of readBytes()
    ErrorT readBytes( char *bytes, unint4 &length );

    //! HAL implementation of writeBytes()
    ErrorT writeBytes( const char *bytes, unint4 length );

    /*!
     * \brief Receive method as speciefied in the interface from the CommDeviceDriver
     *
     * Method which gets called whenver this devices throws an external IRQ.
     * The method will be called by a workerthread running insde the kernelspace.
     */
    void recv();

    /*!
     * \brief Send method which sends the bytes given to the destination addr
     *
     * ignores the destination addr and addr_len since they are not needed on a 1-1 connection
     * as serial line connections usually are.
     */
    ErrorT send(packet_layer* packet, char* dest_addr, int addr_len, int2 fromProtocol_ID );

    ErrorT lowlevel_send( char* data, int len ) {return cError;};
    /*!
     * \brief Broadcasts a packet.
     *
     * Calling this method just delegates the call to send since there exists no broadcast address
     * on serial line connections.
     */
    ErrorT broadcast( packet_layer* packet, int2 fromProtocol_ID );

    ErrorT multicast( packet_layer* packet, int2 fromProtocol_ID, unint4 dest_addr ) { return cNotImplemented; }


    //! Checks whether there is some data available inside the receive fifo.
    bool hasPendingData();

};

#endif /* _OPB_UART_LITE_HH */
