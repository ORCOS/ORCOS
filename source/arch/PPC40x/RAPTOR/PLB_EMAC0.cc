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

#include <PLB_EMAC0.hh>
#include "inc/memio.h"
#include "comm/AddressProtocol.hh"
#include <kernel/Kernel.hh>
#include "kernel/Kernel.hh"
#include "inc/memtools.hh"
#include "lwip/netif.h"
#include "lwip/memp.h"
#include "netif/ethar.h"

#include "inet.h"
#include "kernel/Kernel.hh"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include "netif/etharp.h"
#include "lwip/stats.h"
extern Kernel* theOS;


// the receive buffer used by this device
static char recvBuffer[MAX_FRAME_SIZE];

// Default Mac Address...
static char default_macaddr[6]  __attribute__((aligned(4))) = {0x1,0x1,0x1,0x1,0x1,0x1};

// the position of the next packet
static unint2 recvBufPos;


/*! The network interface for this eem module inside lwip */
struct netif tEMAC0Netif;

/*! The ipv4 address structure of this device.  */
static struct ip4_addr tIpAddr;

extern "C" err_t ethernet_input(struct pbuf *p, struct netif *netif);



static err_t plbemac_low_level_output(struct netif *netif, struct pbuf *p) {

	if (p->tot_len > 1500) {
		LOG(ARCH,WARN,(ARCH,WARN,"SMSC95xxUSBDeviceDriver: not sending packet. Len > 1500."));
		return (ERR_MEM);
	}

	if (p == 0) return (ERR_ARG);


	PLB_EMAC0 *driver =  (PLB_EMAC0*) netif->state;
	driver->lowlevel_send((char*) p->payload,p->tot_len);
	return (ERR_OK);
}



err_t plbemac_ethernetif_init(struct netif *netif) {

#if LWIP_NETIF_HOSTNAME
    /* Initialize interface hostname */
    netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */


    netif->name[0] = 'E';
    netif->name[1] = '0';

    /* We directly use etharp_output() here to save a function call.
     * You can instead declare your own function an call etharp_output()
     * from it if you have to do some checks before sending (e.g. if link
     * is available...) */
#if LWIP_ARP
    netif->ip4_output = etharp_output;
#else
#warning "PLBEMAC Device Driver non operational without ARP support."
#endif
    netif->input = ethernet_input;
    netif->linkoutput = plbemac_low_level_output;
    netif->hwaddr_len = 6;

    for (int i = 0; i < netif->hwaddr_len; i++)
    {
       netif->hwaddr[i] = default_macaddr[i];
    }

   /* maximum transfer unit */
   netif->mtu = 1400;

   /* device capabilities */
   /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
   netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

   return (cOk);
}


/*---------------------------------------------------------------------------*/
PLB_EMAC0::~PLB_EMAC0()
/*---------------------------------------------------------------------------*/
{

}

/*---------------------------------------------------------------------------*/
PLB_EMAC0::PLB_EMAC0( T_PLB_EMAC0_Init* init ) :
    CommDeviceDriver( init->Name )
/*---------------------------------------------------------------------------*/
{
    base_addr = init->Address;
    tx_frame_number = 0;
    rx_frame_number = 0;
    recvBufPos = 24;

    // set the mac address of this device
   // *( (volatile unsigned int *) ( base_addr + CONTROL_REG_RX_OFFSET ) ) = 0;

    theOS->getInterruptManager()->registerIRQ(PLB_EMAC0_IRQ,this,4000);

    unsigned int MacAddr;

    MacAddr = ( ( LocalAddress )[ 5 ] << 8 ) | ( LocalAddress )[ 4 ];
    *( (volatile unsigned int *) ( base_addr + MAC_HIGH_ADDR_OFFSET ) ) = MacAddr;

    MacAddr = ( ( LocalAddress )[ 3 ] << 24 ) | ( ( LocalAddress )[ 2 ] << 16 ) | ( ( LocalAddress )[ 1 ] << 8 )
            | ( LocalAddress )[ 0 ];
    *( (volatile unsigned int *) ( base_addr + MAC_LOW_ADDR_OFFSET ) ) = MacAddr;

    // enable broadcast packet reception (e.g used by ARP a.s.o)
    *( (volatile unsigned int *) ( base_addr + BROADCAST_EN_OFFSET ) ) = 1;

    // enable multicast packet reception
    *( (volatile unsigned int *) ( base_addr + MULTICAST_EN_OFFSET ) ) = 1;


    // set netmask for this device
	struct ip4_addr eth_nm;

#ifndef Board_ETH_IP4NETMASK
//define default value
#define Board_ETH_IP4NETMASK 255,0,0,0
#endif
	int netmask[4] = {Board_ETH_IP4NETMASK};
	IP4_ADDR(&eth_nm, netmask[0], netmask[1], netmask[2], netmask[3]);

#ifndef Board_ETH_IP4ADDR
//define default value
#define Board_ETH_IP4ADDR 10,1,0,2
#endif
	int ipaddr[4] = {Board_ETH_IP4ADDR};
	IP4_ADDR(&tIpAddr, ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);


	tEMAC0Netif.state = this;
	// save driver in netif as state
	// use ethernet interface init method in lwip_ethernetif.c
	netif_add(&tEMAC0Netif, &tIpAddr, &eth_nm, 0, 0, &plbemac_ethernetif_init, 0);
	netif_set_default(&tEMAC0Netif);

	struct eth_addr ethaddr;
	for (int i = 0; i < tEMAC0Netif.hwaddr_len; i++) {
		ethaddr.addr[i] = tEMAC0Netif.hwaddr[i];
	}

	struct ip_addr tipaddr;
	tipaddr.version = IPV4;
	tipaddr.addr.ip4addr.addr = tIpAddr.addr;

	update_ar_entry(&tEMAC0Netif, &tipaddr, &ethaddr, /*ETHARP_TRY_HARD*/1);
	netif_set_up(&tEMAC0Netif);

    clearIRQ();
    enableIRQ();

}

/*---------------------------------------------------------------------------*/
ErrorT PLB_EMAC0::clearIRQ()
/*---------------------------------------------------------------------------*/
{
    // clear all pending interrupts to be in a clear state
    unint4 regval = *( (unint4*) ( base_addr + 0x50020 ) );
    *( (volatile unint4*) ( base_addr + 0x50020 ) ) = regval;

    __asm__ volatile ("eieio");
    return cOk;
}

/*---------------------------------------------------------------------------*/
ErrorT PLB_EMAC0::enableIRQ()
/*---------------------------------------------------------------------------*/
{

    *( (volatile unint4 *) ( base_addr + 0x00050008 ) ) = 0x00000007; // Device Interrupt Enable Register IPIRE Bit
    *( (volatile unint4 *) ( base_addr + 0x00050028 ) ) = 0x00000003; // EMAC Interrupt Enable Register (enable two interrupts)
    *( (volatile unint4 *) ( base_addr + 0x0005001C ) ) = 0x80000000; // Device Global Interrupt Enable (GBIE Bit) (Enables EMAC to produce interrupts)

    __asm__ volatile ("eieio");
    return cOk;
}

/*---------------------------------------------------------------------------*/
ErrorT PLB_EMAC0::disableIRQ()
/*---------------------------------------------------------------------------*/
{
    *( (volatile unint4 *) ( base_addr + 0x00050028 ) ) = 0x0; // EMAC Interrupt Enable Register

    __asm__ volatile ("eieio");
    return cOk;
}

/*---------------------------------------------------------------------------*/
bool PLB_EMAC0::hasPendingData()
/*---------------------------------------------------------------------------*/
{
    return *( (volatile unsigned int *) ( base_addr + STATUS_REG_RX_OFFSET ) ) != rx_frame_number;
}

ErrorT PLB_EMAC0::lowlevel_send( char* data, int len ) {


	while ( *( (volatile unsigned int *) ( base_addr + CONTROL_REG_TX_OFFSET ) ) != tx_frame_number ) {

	}

	// the next code lines create the mac header inside the hardwares frame buffer
	int frame_start_offset = tx_frame_number * 4 * FRAME_BEGIN;

	*( (volatile unsigned int *) ( base_addr + frame_start_offset + TX_ADDR_OFFSET ) ) = (unint4) len;

	unint2 bytes = 0;

	while (bytes < len)
	{
		// get next 4 bytes
		unint4 word = * ((unint4*)data);
		data += 4;
		*( (volatile unsigned int *) ( base_addr + 4 + frame_start_offset + TX_ADDR_OFFSET + bytes  ) ) = word;
		bytes+=4;
	}

	tx_frame_number = ( tx_frame_number + 1 ) % 4;

	// OK frame is stored in TX frame buffer
	// indicate the availability of a TX-packet to the hardware
	*( (volatile unsigned int *) ( base_addr + STATUS_REG_TX_OFFSET ) ) = tx_frame_number;


	return cOk;
}


ErrorT PLB_EMAC0::broadcast( packet_layer* packet, int2 fromProtocol_ID ) {
    //return send(packet,(char*) &BroadcastAddress,6,fromProtocol_ID);
	return cError;
}


ErrorT PLB_EMAC0::multicast( packet_layer* packet, int2 fromProtocol_ID, unint4 dest_addr )
{
    // check for used protocol to create multicast mac address
    // see e.g. http://en.wikipedia.org/wiki/Multicast_address
    /*if (fromProtocol_ID == 0x800)
    {
        unsigned char MulticastAddress[ MAC_ADDR_SIZE ] = { 0x01,0x00,0x5E,0x00,0x00,0x00 };
        // take upper 23 bits of dest_addr and copy them into the MulticastAddress
        unint4 a = *((unint4*) &(MulticastAddress[2]));
        a |= (dest_addr & 0x7FFFFF);
        *((unint4*) &(MulticastAddress[2])) = a;
        return send(packet,(char*) &MulticastAddress,6,fromProtocol_ID);
    }*/

    return cError;
}

int status;


void PLB_EMAC0::recv() {


    status = *( (volatile unsigned int *) ( base_addr + STATUS_REG_RX_OFFSET ) );
   // LOG(ARCH,ERROR,(ARCH,ERROR,"Ethernet frame received!  %d, %d",rx_frame_number,status));

     // while rx_frame_number is not equal to status there are packets in the eth fifo
    while ( status != ( rx_frame_number ) ) {

       // LOG(ARCH,ERROR,(ARCH,ERROR,"Ethernet frame received!  %d, %d",rx_frame_number,status));

        // debug ..
        register int4 ledval = ( ( rx_frame_number + 1 ) ^ 0xFFFFFFFF ) << 12;
        OUTW(0x90220000,ledval);

        unsigned int ByteCount;
        unsigned int WordCount;
        unsigned int ExtraByteCount;
        unsigned int *WordBuffer = (unsigned int *) ( (int) &recvBuffer + recvBufPos );

        // get the packetlength from the EMAC0 device //
        ByteCount
                = *( (volatile unsigned int *) ( base_addr + 4 * ( rx_frame_number * FRAME_BEGIN ) + RX_ADDR_OFFSET ) );

        // put the length of the packet infront
        ( (unint2*) WordBuffer )[ 0 ] = (unint2) ByteCount;
        WordBuffer = (unsigned int*) ( (int) WordBuffer + 2 );

        WordCount = ByteCount / 4;
        ExtraByteCount = ByteCount % 4;

        // store the received packet inside the buffer
        for ( unint i = 0; i < WordCount; i++ ) {
            WordBuffer[ i ] = *( (volatile unsigned int *) ( base_addr + RX_ADDR_OFFSET + 4 + 4 * i + 4
                    * ( rx_frame_number * FRAME_BEGIN ) ) );
        }

        if ( ExtraByteCount > 0 ) {

            unsigned long LastWord;
            unsigned char *WordPtr;
            unsigned char *ExtraBytesBuffer = (unsigned char *) ( WordBuffer + WordCount );

            /* read the last word */

            LastWord = *( (volatile unsigned int *) ( base_addr + RX_ADDR_OFFSET + 4 + 4 * WordCount + 4
                    * ( rx_frame_number * FRAME_BEGIN ) ) );

            /* only one extra byte
             */
            WordPtr = (unsigned char *) &LastWord;
            if ( ExtraByteCount == 1 ) {
                ExtraBytesBuffer[ 0 ] = WordPtr[ 0 ];
            }

            /* two extra bytes
             */
            else if ( ExtraByteCount == 2 ) {
                ExtraBytesBuffer[ 0 ] = WordPtr[ 0 ];
                ExtraBytesBuffer[ 1 ] = WordPtr[ 1 ];
            }
            /* three extra bytes
             */
            else if ( ExtraByteCount == 3 ) {
                ExtraBytesBuffer[ 0 ] = WordPtr[ 0 ];
                ExtraBytesBuffer[ 1 ] = WordPtr[ 1 ];
                ExtraBytesBuffer[ 2 ] = WordPtr[ 2 ];
            }

        }

        // testing so we keep overwriting packets �-�
        //recvBufPos += ByteCount;

        // incr rx_frame index
        rx_frame_number = ( rx_frame_number + 1 ) % 4;

        // indicate that the packet has successfully been read
        *( (volatile unsigned int *) ( base_addr + CONTROL_REG_RX_OFFSET ) ) = rx_frame_number;

        // pass to protocol
        //int2 protocol_num = ((int2*) ( (int) WordBuffer + 12 ))[ 0 ];
        //AddressProtocol* aproto = theOS->getProtocolPool()->getAddressProtocolbyId( protocol_num );
        //if ( aproto != 0 ) {
        //    aproto->recv( ((char*) WordBuffer) + 14, ByteCount - 14, this );
        //}

    	struct pbuf* ptBuf = pbuf_alloc(PBUF_RAW, ByteCount, PBUF_RAM);
    	if (ptBuf != 0) {

			memcpy(ptBuf->payload, WordBuffer, ByteCount );


			ethernet_input(ptBuf,&tEMAC0Netif);
			pbuf_free(ptBuf);

    	} else {
    		LOG(ARCH,ERROR,(ARCH,ERROR,"no memory for pbuf!"));
    	}

    //	LOG(ARCH,ERROR,(ARCH,ERROR,"Packet done.."));
        // get status again since we may have received another packet by now which did not cause an interrupt
        status = *( (volatile unsigned int *) ( base_addr + STATUS_REG_RX_OFFSET ) );
    }

    // indicate that we have processed all packets
    // we may still have missed some in between existing the loop and these 2 statements
    // unfortunatly this cant be compensated since the hardware is programmed in a bad way
    // which means that the eth will not throw a irq if the CONTROL_REG_RX value is the same
    // as the status value ??arg why?
    status = *( (volatile unsigned int *) ( base_addr + STATUS_REG_RX_OFFSET ) );
    *( (volatile unsigned int *) ( base_addr + CONTROL_REG_RX_OFFSET ) ) = status;

    this->interruptPending = false;

}

