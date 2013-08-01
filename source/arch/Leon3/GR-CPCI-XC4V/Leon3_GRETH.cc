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

#include "Leon3_GRETH.hh"
#include <kernel/Kernel.hh>
#include <inc/memtools.hh>
#include "lwip/netif.h"

/*! The network interface for this eem module inside lwip */
struct netif tEMAC0Netif;

extern Kernel* theOS;

// place greth buffer at the end of memory
// each buffer needs 1KB size
unsigned int* _greth_rx_buffer = (unsigned int*) 0x4F000000;
unsigned int* _greth_tx_buffer = (unsigned int*) 0x4F000400;

unsigned int * rxDescriptor1 = (unsigned int *) 0x4F000800;
unsigned int * rxDescriptor2 = (unsigned int *) 0x4F000804;

unsigned int * txDescriptor1 = (unsigned int *) 0x4F000808;
unsigned int * txDescriptor2 = (unsigned int *) 0x4F00080B;

// receive buffer
static char* recvBuffer = (char*) 0x4F00080C;

// send buffer
static char txBuffer[MAX_FRAME_SIZE];

static unsigned char BroadcastAddress[ MAC_ADDR_SIZE ] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

volatile unsigned int*  mutex = (unsigned int*)0x49000000;
//volatile unsigned int*  mutex2 = (unsigned int*)0x49000004;


Leon3_GRETH::Leon3_GRETH(const char *name, int4 a):
	CommDeviceDriver( name ){
	*((int*)0x49000000) = 0;
	port = a;

	//unsigned int tmp = 0;

	// reset the GRETH controller
	/*OUTW(port + GRETH_CTRL_REG_OFFSET, CTRL_REG_RS);
	do{
		tmp = INW(port + GRETH_CTRL_REG_OFFSET);
	} while (tmp & CTRL_REG_RS);*/


	// set the mac address of this device
	int LocalNodeNr = 1;
	GET_CPU_INDEX(LocalNodeNr);

	if (LocalNodeNr == 1) {
		unsigned int macAddr;

		macAddr =  ((LocalAddress[2] << 24) | (LocalAddress[3] << 16) | (LocalAddress[4] << 8) | LocalAddress[5]);
		OUTW(port + MAC_LSB_REG_OFFSET, macAddr);

		macAddr =  (LocalAddress[0] << 8) | LocalAddress[1] ;
		OUTW(port + MAC_MSB_REG_OFFSET, macAddr);

		OUTW(port + TX_DESCR_REG_OFFSET, (unsigned int)_greth_tx_buffer);


		// set descriptor pointer
		rxDescriptor1 = ( unsigned int * ) _greth_rx_buffer;

		OUTW(port + RX_DESCR_REG_OFFSET, (int) rxDescriptor1);
		*rxDescriptor1 = 0;
		rxDescriptor2 = rxDescriptor1 + 1;
		*rxDescriptor2 = (unsigned int) recvBuffer;

		// enable interrupt for received packet
		*rxDescriptor1 = *rxDescriptor1 | RX_DESCR_IE;

		// enable descriptor
		*rxDescriptor1 = *rxDescriptor1 | RX_DESCR_EN;


		enableIRQ();

		// enable receiver
		OUTW(port + GRETH_CTRL_REG_OFFSET, INW(port + GRETH_CTRL_REG_OFFSET) | CTRL_REG_RE);

		// clear status register
		OUTW(port + STATUS_REG_OFFSET, 0xFFFFFFFF);
	}

}


ErrorT	Leon3_GRETH::enableIRQ(){

	OUTW(port + GRETH_CTRL_REG_OFFSET, INW(port + GRETH_CTRL_REG_OFFSET) | CTRL_REG_RI);

	return cOk;
}

ErrorT	Leon3_GRETH::disableIRQ(){
	OUTW(port + GRETH_CTRL_REG_OFFSET, INW(port + GRETH_CTRL_REG_OFFSET) & ~CTRL_REG_RI);
	return cOk;
}

int Leon3_GRETH::getIRQ(){
	return 14;
}

ErrorT	Leon3_GRETH::readByte  (char* byte){
	return 0;
}

ErrorT	Leon3_GRETH::writeByte (char byte){
	return 0;
}

ErrorT	Leon3_GRETH::readBytes (char *bytes, int4 &length){
	return 0;
}

ErrorT	Leon3_GRETH::writeBytes(const char *bytes, int4 length){
	return 0;
}

void	Leon3_GRETH::recv (){

	// first check for errors
	if (*rxDescriptor1 & (unsigned int)(RX_DESCR_AE | RX_DESCR_FT | RX_DESCR_CR | RX_DESCR_OE | RX_DESCR_LE)){
		// error occured. Ignore package
		return;
	}

	// the receive buffer contains the packet content
	char* packet = recvBuffer;
	int length = (*rxDescriptor1) & RX_DESCR_LENGTH;


	 // pass to protocol
	int2 type = ((int2*)(packet + 12))[0];

    AddressProtocol* aproto = theOS->getProtocolPool()->getAddressProtocolbyId( type );
    if ( aproto != 0 ) {
    	aproto->recv( packet + 14, length - 14, this );
    }
	#if USE_ARP
	    else if (type == 0x0806) {
	    	ARP* arpproto = theOS->getProtocolPool()->getAddressResolutionProtocol();
	    	arpproto->recv( packet + 14, length - 14, this );
	    }
	#endif

    // prepare next descriptor
    rxDescriptor1 = (unsigned int*) INW(port + RX_DESCR_REG_OFFSET);


    this->interruptPending = false;

    rxDescriptor2 = rxDescriptor1 + 1;
    *rxDescriptor2 = (unsigned int) recvBuffer;

    // enable interrupt for received packet
    *rxDescriptor1 = *rxDescriptor1 | RX_DESCR_IE;

    // enable descriptor
    *rxDescriptor1 = *rxDescriptor1 | RX_DESCR_EN;

    // enable receiver
    OUTW(port + GRETH_CTRL_REG_OFFSET, INW(port + GRETH_CTRL_REG_OFFSET) | CTRL_REG_RE);

    enableIRQ();
}

ErrorT Leon3_GRETH::send(packet_layer* packet, char* dest_addr,int addr_len, int2 fromProtocol_ID)
{
	int isLocked = 0;
	do {
		asm volatile(
			"set 0x49000000, %%g6;"
			"set 1, %%g7;"
			"swap [%%g6], %%g7;"
			"mov %%g7, %0;"
			:
			: "r" (isLocked)
			:
					);

	}while (isLocked);

	 bool int_enabled;
	 GET_INTERRUPT_ENABLE_BIT(int_enabled);

	 _disableInterrupts();

	char* sendBuffPtr = txBuffer;
	int len = packet->total_size;

	// destination address
	memcpy(sendBuffPtr, dest_addr, addr_len);
	sendBuffPtr += addr_len;
	len+=addr_len;

	// source address
	memcpy(sendBuffPtr, LocalAddress, addr_len);
	sendBuffPtr += addr_len;
	len+=addr_len;

	// protocol id
	memcpy(sendBuffPtr, (char*) &fromProtocol_ID, 2);
	sendBuffPtr += 2;
	len+=2;

	// data
	int pos = 0;
	do {
		memcpy(sendBuffPtr + pos, packet->bytes, packet->size);
		pos += packet->size;
		packet = packet->next;
	}while (packet != 0);


	// get address for current transmit descriptor
	txDescriptor1 = (unsigned int *) INW(port + TX_DESCR_REG_OFFSET);

	// set address to data
	txDescriptor2 = txDescriptor1 + 1;
	*txDescriptor2 = (unsigned int) txBuffer;

	len = len + (2* addr_len) + 2;
	// Set number of bytes to send
	*txDescriptor1 = (unsigned int ) len;

	// enable descriptor
	*txDescriptor1 = *txDescriptor1 | TX_DESCR_EN;

	// enable transmitter
	OUTW(port + GRETH_CTRL_REG_OFFSET, INW(port + GRETH_CTRL_REG_OFFSET) | CTRL_REG_TE);
	*((int*)0x49000000) = 0;

    if ( int_enabled ) {
            _enableInterrupts();
        }

	return 0;
}

ErrorT Leon3_GRETH::lowlevel_send(char* data, int len){

	int isLocked = 0;
	do {
		asm volatile(
			"set 0x49000000, %%g6;"
			"set 1, %%g7;"
			"swap [%%g6], %%g7;"
			"mov %%g7, %0;"
			:
			: "r" (isLocked)
			:
					);

	}while (isLocked);

	 bool int_enabled;
	 GET_INTERRUPT_ENABLE_BIT(int_enabled);

	 _disableInterrupts();

	char* sendBuffPtr = txBuffer;

	memcpy(sendBuffPtr, data, len);

	// get address for current transmit descriptor
	txDescriptor1 = (unsigned int *) INW(port + TX_DESCR_REG_OFFSET);

	// set address to data
	txDescriptor2 = txDescriptor1 + 1;
	*txDescriptor2 = (unsigned int) txBuffer;

	// Set number of bytes to send
	*txDescriptor1 = (unsigned int ) len;

	// enable descriptor
	*txDescriptor1 = *txDescriptor1 | TX_DESCR_EN;

	// enable transmitter
	OUTW(port + GRETH_CTRL_REG_OFFSET, INW(port + GRETH_CTRL_REG_OFFSET) | CTRL_REG_TE);
	*((int*)0x49000000) = 0;

	if ( int_enabled ) {
			_enableInterrupts();
		}

	return 0;
}

ErrorT Leon3_GRETH::broadcast(packet_layer* packet, int2 fromProtocol_ID) {
	return send(packet,(char*) &BroadcastAddress,6,fromProtocol_ID);
}

ErrorT Leon3_GRETH::multicast( packet_layer* packet, int2 fromProtocol_ID, unint4 dest_addr )
{
    // check for used protocol to create multicast mac address
    // see e.g. http://en.wikipedia.org/wiki/Multicast_address
    if (fromProtocol_ID == 0x800)
    {
        unsigned char MulticastAddress[ MAC_ADDR_SIZE ] = { 0x01,0x00,0x5E,0x00,0x00,0x00 };
        // take upper 23 bits of dest_addr and copy them into the MulticastAddress
        unint4 a = *((unint4*) &(MulticastAddress[2]));
        a |= (dest_addr & 0x7FFFFF);
        *((unint4*) &(MulticastAddress[2])) = a;
        return send(packet,(char*) &MulticastAddress,6,fromProtocol_ID);
    }

    return cError;
}

unint2 Leon3_GRETH::getMTU() {
    return MAX_FRAME_SIZE;
}

