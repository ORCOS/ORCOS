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


/*---------------------------------------------------------------------------*/
LEON_UART::~LEON_UART()
/*---------------------------------------------------------------------------*/
{

}

/*---------------------------------------------------------------------------*/
LEON_UART::LEON_UART(const char* name,int4 a) : CommDeviceDriver(name)
/*---------------------------------------------------------------------------*/
{
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


/*---------------------------------------------------------------------------*/
ErrorT LEON_UART::enableIRQ()
/*---------------------------------------------------------------------------*/
{
  OUTW(baseAddr + UART_CONTROL_REG_OFFSET, INW(baseAddr + UART_CONTROL_REG_OFFSET) | UART_CR_RI | UART_CR_TI);
  return cOk;
}


void LEON_UART::setLED(LED* led)
{
	#if ENABLE_TXRX_LEDS
	leds = led;
#else
	leds = 0;
	#endif
}


/*---------------------------------------------------------------------------*/
ErrorT LEON_UART::disableIRQ()
/*---------------------------------------------------------------------------*/
{
  OUTW(baseAddr + UART_CONTROL_REG_OFFSET, INW(baseAddr + UART_CONTROL_REG_OFFSET) & ~(UART_CR_RI | UART_CR_TI));
  return cOk;
}

/*---------------------------------------------------------------------------*/
int LEON_UART::getIRQ()
/*---------------------------------------------------------------------------*/
{
  return 2;
}

/*---------------------------------------------------------------------------*/
ErrorT LEON_UART::writeBytes(const char* bytes, unint4 length)
/*---------------------------------------------------------------------------*/
{
	// This method sends the bytes and waits whenever the queue is full.
	unint count;
	ErrorT err;

	for (count = 0, err = cOk; (count < length) && (err == cOk); count++)
		err = outputSCC(-1, bytes[count]);

	return err;

}


void LEON_UART::sendByte(byte Data)
/*---------------------------------------------------------------------------*/
{
	OUTW(baseAddr + UART_DATA_REG_OFFSET, (word)Data);

	#if ENABLE_TXRX_LEDS
	if (leds != 0){
		// only switch led every 4 bytes send
		if (++TX_COUNT % 16 == 0)
		{
			char ledval;
			leds->readByte((char*)&ledval);
			// toggle first led since we send something
			if ((ledval & 1) == 1) ledval &= ~(1);
			else ledval |= 1;
			leds->writeByte(ledval);
		}
	}
	#endif
}

/*---------------------------------------------------------------------------*/
bool LEON_UART::isTransmitBufferFull()
/*---------------------------------------------------------------------------*/
{;
	word reg = INW(baseAddr + UART_STATUS_REG_OFFSET);
	return ((reg & UART_SR_TF) == UART_SR_TF);
}

bool LEON_UART::isReceiveBufferFull()
{
	word reg = INW(baseAddr + UART_STATUS_REG_OFFSET);
	return ((reg & UART_SR_RF) == UART_SR_RF);
}


/*---------------------------------------------------------------------------*/
ErrorT LEON_UART::outputSCC(int4 Timeout, byte c)
/*---------------------------------------------------------------------------*/
{
	int ret = cError;
	unsigned int* status = (unsigned int*)0x80000104;
	int loops = 0;

	while (!((*status) & UART_SR_TH) && (loops < 100000))
	        	loops++;

	sendByte(c);	/* output char */

	loops = 0;
	while (!((*status) & UART_SR_TS) && (loops < 100000))
	        	loops++;

	return ret = cOk;;
}

/*---------------------------------------------------------------------------*/
ErrorT LEON_UART::inputSCC(int4 Timeout, byte *c)
/*---------------------------------------------------------------------------*/
{
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


/*---------------------------------------------------------------------------*/
bool LEON_UART::hasPendingData()
/*---------------------------------------------------------------------------*/
{
	word reg = INW(baseAddr + UART_STATUS_REG_OFFSET);
	return  (reg & UART_SR_DR);
}


/*---------------------------------------------------------------------------*/
byte LEON_UART::recvByte()
/*---------------------------------------------------------------------------*/
{
	#if ENABLE_TXRX_LEDS
	if (leds != 0){
		// only switch led every 4 bytes send
		if (++RX_COUNT % 16 == 0)
		{
			char ledval;
			leds->readByte(&ledval);
			// toggle fifth led since we received something
			if ((ledval & 16) == 16) ledval &= ~(16);
			else ledval |= 16;
			leds->writeByte(ledval);
		}
	}
	#endif

	return (byte) INW(baseAddr + UART_DATA_REG_OFFSET);
}


ErrorT LEON_UART::send( packet_layer* packet, char* dest_addr, int addr_len, int2 fromProtocol_ID ) {

		// this device irgnores dest_addr and addr_len since we got
		// a 1-1 communication here (serial line!!)
		// only devices with more than 1 connected neighbor (lan,wlan,bluetooth ...)
		// need to lookup the mac address of the host with dest_addr (may use ARP)
		// but not this device!

		/*PacketHeader packetHeader;

		// check if we support this packet.
		if (len > COMDD_MAX_PACKET_LENGTH) {
			return cError;
		}

		packetHeader.packetHead[0] = 0x55;
		packetHeader.packetHead[1] = 0x55;
		packetHeader.packetHead[2] = 0x55;
		packetHeader.packetHead[3] = 0x55;
		packetHeader.addressProtocolID = fromProtocol->getId();
		//memcpy(&packetHeader.destinationAddress, dest_addr, 4);
		packetHeader.packetLength = len;

		// calc data checksum
		packetHeader.dataChecksum = byte[0];
		for (unint2 i = 1; i < len; i++) {
			packetHeader.dataChecksum ^= byte[i];
		}

		// calc header checksum
		packetHeader.headerChecksum = ((char*)&packetHeader)[0];
		for (unint2 i = 1; i < sizeof(PacketHeader)-2; i++) {
			packetHeader.headerChecksum ^= ((char*)&packetHeader)[i];
		}

		// send the header to the wire
		writeBytes((char*)&packetHeader, sizeof(PacketHeader));
		// send the data to the wire
		writeBytes(byte, len);
	*/
		return cOk;
}


void LEON_UART::recv()
{
	PacketHeader* packetHeader;


	// this method got called since the opb_uart raised an external interrupt
	// this interrupt will be active as long as the opb_uarts receive fifo is not empty
	// so the first thing to do is to clear the uarts fifo and store the bytes in our private buffer
	// then we can reactivate the irq on this device

	#if ENABLE_TXRX_LEDS
	if (leds != 0){
		// set the frouth leds to on if we got a buffer overflow
		if (isReceiveBufferFull())
			{
			char ledval;
			leds->readByte(&ledval);
			ledval |= 8;
			leds->writeByte(ledval);
			}
	}
	#endif

	while (this->hasPendingData())
	{
		// check for out of bounds error
		if (currentPosition >= sizeof(packetBuffer))
		{
			// start all over again (we may miss a packet)
			currentPosition = 0;
			headerFound = false;
		}
		// read 1 byte and dont wait
		inputSCC(1,packetBuffer+currentPosition);
		//this->readBytes(packetBuffer+currentPosition,1);
		currentPosition++;
	}

	// we may now reactivate the irq since the fifo has been cleared
	this->enableIRQ();

	//theOS->getBoard()->getLED()->writeByte(num);

	// at this point we may have the following situations :
	// 1. a whole packet (header + data) is inside the buffer
	// 2. only parts of the packet are inside the buffer

	// if we dont have a packet header yet, try to find it!
	if (!headerFound) {
		for (int i = 0; i+sizeof(PacketHeader) < currentPosition; i++) {

			if (packetBuffer[i] == 0x55
				&&	packetBuffer[i+1] == 0x55
				&&	packetBuffer[i+2] == 0x55
				&&	packetBuffer[i+3] == 0x55)
			{


					// ok we may have received the header (enough bytes received)
					// with some luck, we found a header now. lets try
					packetHeader = (PacketHeader*) &packetBuffer[i];

					// calc header checksum
					unint2 headerChecksum = ((char*)packetHeader)[0];
					for (unint2 j = 1; j < sizeof(PacketHeader)-2; j++) {
						headerChecksum ^= ((char*)packetHeader)[j];
					}

					// check if checksum correct
					if (headerChecksum == packetHeader->headerChecksum) {
						// heureka, we found a header!
						headerFound = true;
						// move all bytes to the beginning
							// we got some bytes infront of the header that we consider as garbage
						if (i!= 0)
						{
							for (int j = i; j < currentPosition; j++) {
								packetBuffer[j-i] = packetBuffer[j];
							}
							currentPosition = currentPosition - i;
						}
						// escape the iteration since we found a header by now
						// but : the packet might still be incomplete!
						break;

					} // if headerchecksum

			} // if packerbuffer = 0x55 ..
		} // for i = 0 ..
	} // if !headerFound

	// if we have a header, check if the packet is complete.
	if (headerFound) {
		// adjuste pointer to packetheader since it is now at the
		// beginning of the buffer
		packetHeader = (PacketHeader*) &packetBuffer[0];
		if (packetHeader->packetLength + sizeof(PacketHeader) <= currentPosition) {
			// we have more than enough bytes in the buffer to consider
			// this packet as complete. check if its valid

			// calc data checksum
			unint2 dataChecksum = packetBuffer[sizeof(PacketHeader)];
			for (unint2 i = 1; i < packetHeader->packetLength; i++) {
				dataChecksum ^= packetBuffer[sizeof(PacketHeader) + i];
			}

			if (dataChecksum == packetHeader->dataChecksum) {
				// this is a valid packet. deliver it
				AddressProtocol* aproto = theOS->getProtocolPool()->getAddressProtocolbyId(packetHeader->addressProtocolID);
				if (aproto != 0)
				{
					aproto->recv(&packetBuffer[0] + sizeof(PacketHeader),packetHeader->packetLength,this);
				}
			}

			// packet has been processed or was invalid. discard what we have
			headerFound = false;
			unint2 len = packetHeader->packetLength + sizeof(PacketHeader);

			// clean up, move data to front of buffer
			// (we can't simply discard the whole buffer, because maybe the data belongs
			// to a following packet)
			for (int j = 0; j < len; j++) {
				packetBuffer[j] = packetBuffer[j+len];
			}
			currentPosition = currentPosition - len;
		}
	}
}

ErrorT 	LEON_UART::broadcast( packet_layer* packet, int2 fromProtocol_ID ) {
    return send(packet, 0, 0,fromProtocol_ID );

}

/*---------------------------------------------------------------------------*/
ErrorT LEON_UART::readByte  (char* byteptr)
/*---------------------------------------------------------------------------*/
{
	inputSCC( 1, (byte*) byteptr);
	return cOk;
}


/*---------------------------------------------------------------------------*/
ErrorT LEON_UART::readBytes (char* bytes, unint4 &length)
/*---------------------------------------------------------------------------*/
{
	// This method polls (no waiting) for new data

	ErrorT err;
	unint4 count;

	for (count = 0, err = cOk; ((count < length) && (err == cOk)); count++)
		err = inputSCC( 1, (byte*) bytes + count);
	length = (err == cOk) ? count : count - 1;
	return err;
}

ErrorT LEON_UART::writeByte (char byte)
/*---------------------------------------------------------------------------*/
{
	outputSCC(-1,byte);
	return cOk;
}
