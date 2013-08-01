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

#ifndef LEON_UART_HH
#define LEON_UART_HH

#include <arch/shared/UART/UART.hh>
#include <archtypes.hh>
#include <hal/CommDeviceDriver.hh>

/* UART register offsets */

#define UART_DATA_REG_OFFSET		0x0
#define UART_STATUS_REG_OFFSET		0x4
#define UART_CONTROL_REG_OFFSET		0x8
#define UART_SCALER_REG_OFFSET		0xC

/* control register bit positions */
#define UART_CR_RF		0x400 /* receiver FIFO interrupt enable */
#define UART_CR_TF 		0x200 /* transmitter interrupt enable */
#define UART_CR_LB		0x80  /* loop back mode enable */
#define UART_CR_FL		0x40  /* flow control */
#define UART_CR_PE		0x20  /* parity enable */
#define UART_CR_PS		0x10  /* parity select */
#define UART_CR_TI		0x08  /* transmitter interrupt enable */
#define UART_CR_RI		0x04  /* receiver interrupt enable */
#define UART_CR_TE		0x02  /* transmitter enable */
#define UART_CR_RE		0x01  /* receiver enable */

/* status register bit positions */
#define UART_SR_RCNT	0xF8000000  /* receiver FIFO count */
#define UART_SR_TCNT	0x07C00000  /* transmitter FIFO count */

#define UART_SR_DB		0x800 /* FIFO debug mode enable */
#define UART_SR_RF		0x400 /* receiver FIFO full */
#define UART_SR_TF 		0x200 /* transmitter FIFO full */
#define UART_SR_RH 		0x100 /* receiver FIFO half full */
#define UART_SR_TH		0x80  /* transmitter FIFO half full */
#define UART_SR_FE		0x40  /* framing error */
#define UART_SR_PE		0x20  /* parity error */
#define UART_SR_OV		0x10  /* indicates overrun */
#define UART_SR_BR		0x08  /* break received */
#define UART_SR_TE		0x04  /* transmitter FIFO empty */
#define UART_SR_TS		0x02  /* transmitter shift register empty */
#define UART_SR_DR		0x01  /* data ready */

#define BAUD_RATE		9600
#define SCALE			0x38F6


/* the following constant specifies the size of the FIFOs, the size of the
 * FIFOs includes the transmitter and receiver such that it is the total number
 * of bytes that the UART can buffer
 */
#define UART_FIFO_SIZE               32

/* Stop bits are fixed at 1.
 */
#define UART_STOP_BITS               1

/* Parity definitions
 */
#define UART_PARITY_EVEN             0
#define UART_PARITY_ODD              1


#define COMDD_MAX_PACKET_LENGTH 50

//! Enable/Disable the usage of leds as TX/RX signalling
#define ENABLE_TXRX_LEDS 1

#if ENABLE_TXRX_LEDS
#include <arch/Leon3/GR-CPCI-XC4V/LED.hh>
#endif

//! MAC FRAME HEADER for the OPB_UART_Lite
struct PacketHeader {
		char packetHead [4];
		unint2 addressProtocolID;
		//char destinationAddress[4]; < no addr needed since serial devices do not possess a mac
		unint2 packetLength;
		unint2 dataChecksum;
		unint2 headerChecksum;
};

/*!
 *  \brief Leon UART driver
 *
 *  This class encapsulates the UART driver implementation.
 */
class LEON_UART : public CommDeviceDriver
{

private:
  volatile byte inputPending;

  int4  	baseAddr;

  #if ENABLE_TXRX_LEDS
  byte TX_COUNT;
  byte RX_COUNT;
  LED* leds;
  #endif

  void		sendByte(byte Data);

  byte		recvByte();

  bool		isTransmitBufferFull();

  bool		isReceiveBufferFull();

  int4   	getAddr() const         { return baseAddr; }


  char packetBuffer[COMDD_MAX_PACKET_LENGTH+sizeof(PacketHeader)];

  unint2 currentPosition;

  bool headerFound;

protected:

public:

  //!  constructor
  LEON_UART(const char *name, int4 a);

  //!  destructor
  ~LEON_UART();

  ErrorT        enableIRQ();

  ErrorT        disableIRQ();

  int	        getIRQ();

  UARTBaudRate  getBaudRate(){ return kBaud38400; }

  void 			setLED(LED* led);


  /// interface to meet the CharacterDeviceDriver
  ErrorT 		readByte  (char* byte);
  ErrorT 		writeByte (char byte);
  ErrorT 		readBytes (char *bytes, unint4 &length);
  ErrorT 		writeBytes(const char *bytes, unint4 length);



  /// interface from the CommDeviceDriver
  /// method which gets called whenver this devices throws a extern IRQ
  void 			recv ();

  /// send method which sends the bytes given to the destination addr
  /// ignores the destination addr, len since they are not needed on a 1-1 connection
  /// as serial line connections are
  ErrorT 		send  (packet_layer* packet, char* dest_addr,int addr_len, int2 fromProtocol_ID);
  ErrorT lowlevel_send( char* data, int len ) {return cError;};

  ErrorT 		broadcast(packet_layer* packet, int2 fromProtocol_ID);

  virtual
  ErrorT multicast( packet_layer* packet, int2 fromProtocol_ID, unint4 dest_addr ) { return 0; }

  /// Received FIFO has vaild data. Used to distinguish between IRQs.
  bool          hasPendingData();

  ///  Timeout of -1 causes it to wait indefinitely until something is read.
  ErrorT        outputSCC(int4 Timeout, byte c);

  ErrorT        inputSCC(int4 Timeout, byte *c);




};
#endif /* LEON_UART_HH */
