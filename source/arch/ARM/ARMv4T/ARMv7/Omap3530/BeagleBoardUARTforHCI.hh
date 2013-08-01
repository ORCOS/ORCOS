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

#ifndef BEAGLEBOARDUARTFORHCI_HH
#define BEAGLEBOARDUARTFORHCI_HH

#include <error.hh>
#include <types.hh>
#include <hal/CommDeviceDriver.hh>

// UART register offsets
#define UART_MDR1_REG_OFFSET	0x020
#define UART_IER_REG_OFFSET		0x004
#define UART_DLL_REG_OFFSET		0x000
#define UART_DLH_REG_OFFSET		0x004
#define UART_EFR_REG_OFFSET		0x008	// enhanced feature register
#define UART_LCR_REG_OFFSET		0x00C	// line control register
#define UART_THR_REG_OFFSET		0x000	// transmit hold register
#define UART_RHR_REG_OFFSET		0x000	// receive hold register
#define UART_FCR_REG_OFFSET		0x008	// FIFO control register
#define UART_LSR_REG_OFFSET		0x014	// line status register
#define UART_SSR_REG_OFFSET		0x044	// supplementary status register
#define UART_MCR_REG_OFFSET		0x010
#define UART_TCR_REG_OFFSET		0x018
#define UART_SYSC_REG_OFFSET 	0x054
#define UART_TLR_REG_OFFSET 	0x01C
#define UART_SCR_REG_OFFSET		0x040

// UART register bit positions
// IER register
#define UART_IER_SLEEP_MODE	0x10
#define UART_IER_MODEM		0x08
#define UART_IER_LINE		0x04
#define UART_IER_THR		0x02
#define UART_IER_RHR		0x01

// EFR register
#define UART_EFR_ENHANCED_EN	0x10

// LCR register
#define UART_LCR_DIV_EN			0x80
#define UART_LCR_BREAK_EN		0x40
#define UART_LCR_PARITY_TYPE1	0x20
#define UART_LCR_PARITY_TYPE2	0x10
#define UART_LCR_PARITY_EN		0x08
#define UART_LCR_NB_STOP		0x04
#define UART_LCR_CHAR_LENGTH	0x03	// char length 8 bit

#define COMDD_MAX_PACKET_LENGTH 50

struct PacketHeader {
    char packetType; // HCI packet type
};

/*!
 *  \brief UART driver for OMAP3530
 *
 *  This class encapsulates the UART driver implementation.
 */
class BeagleBoardUARTforHCI: public CommDeviceDriver {

private:

	//! the memory mapped IO address of this device
	int4 baseAddr;

	//! the buffer the object can work on to send and receive packets
	char packetBuffer[ COMDD_MAX_PACKET_LENGTH + sizeof(PacketHeader) ];

	//! current position inside the packerBuffer
	unint2 currentPosition;

	//! indicates whether the header of a packet was already found inside the last recv() calls
	bool headerFound;

	void sendByte(byte Data);

	byte recvByte();

	bool isTransmitBufferFull();

	bool isReceiveBufferFull();

protected:

public:

	//!  constructor
	BeagleBoardUARTforHCI( const char *name, int4 a );

	//!  destructor
	~BeagleBoardUARTforHCI();

	//! enables Interrupt Requests of this device
	ErrorT enableIRQ();

	//! disables Interrupt Requests of this device. Interrupts may still be pending.
	ErrorT disableIRQ();

	#ifdef HAS_BoardLEDCfd
	    //! sets the leds which can be used for TX/RX signaling
	    void setLED( LED* led );
	#endif

	// interface to meet the CharacterDeviceDriver
	ErrorT readByte  (char* byte);
	ErrorT writeByte (char byte);
	ErrorT readBytes (char *bytes, unint4 &length);
	ErrorT writeBytes(const char *bytes, unint4 length);

	// interface to meet the CommDeviceDriver
    void recv();
    ErrorT send(packet_layer* packet, char* dest_addr, int addr_len, int2 fromProtocol_ID );
    ErrorT lowlevel_send( char* data, int len ) {return cError;};
    ErrorT broadcast( packet_layer* packet, int2 fromProtocol_ID );
    ErrorT multicast( packet_layer* packet, int2 fromProtocol_ID, unint4 dest_addr ) { return cNotImplemented; }

    ErrorT inputSCC(int4 Timeout, byte *c);
    ErrorT outputSCC(int4 Timeout, byte c);
    bool hasPendingData(void);

};

#endif /* BEAGLEBOARDUARTFORHCI_HH */
