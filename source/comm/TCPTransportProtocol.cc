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

#ifndef TCPTRANSPORTPROTOCOL_HH
#define TCPTRANSPORTPROTOCOL_HH


#include "TCPTransportProtocol.hh"
#include "lwipopts.h"

#if LWIP_TCP

#include <memtools.hh>
#include "kernel/Kernel.hh"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "comm/Socket.hh"

extern Kernel* theOS;

extern Task*   pCurrentRunningTask;


TCPTransportProtocol::TCPTransportProtocol() :
    TransportProtocol( 6 ) {
}

TCPTransportProtocol::~TCPTransportProtocol() {
}

ErrorT TCPTransportProtocol::sendto( packet_layer* payload, const sockaddr* fromaddr,
                const sockaddr *dest_addr, AddressProtocol* NextLayer, Socket* fromsock ) {

	/* no sendto on connection oriented protocols */
	return (cError);
}

ErrorT TCPTransportProtocol::send(packet_layer* payload, AddressProtocol* NextLayer, Socket* fromsock) {
	LOG(COMM,INFO,(COMM,INFO,"TCP:send(): size: %d",payload->size));

	// allocate a pbuf
	struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, payload->size, PBUF_RAM);
	if (p != 0) {
		// unfortunately we need to copy here
		memcpy(p->payload,payload->bytes,payload->size);
		struct tcp_pcb *pcb = (struct tcp_pcb*) fromsock->arg;

		// try to write it to the tcp stack
		// will try to enqueue the packet and maybe merge it with previous ones still on queue
		int ret = tcp_write(pcb,p,p->len,0);

		// free anyway
		pbuf_free(p);

		// check if enqueue operation was successfull
		if (ret != ERR_OK) return (cTCPEnqueueFailed);

		// try to send directly
		tcp_output(pcb);

		return (cOk);
	}
	else {
		LOG(COMM,WARN,(COMM,WARN,"TCP:send(): packet dropped.. no more memory..!"));
		return (cPBufNoMoreMemory);
	}
}

ErrorT TCPTransportProtocol::recv( char* packetstart, int packetlength, AddressProtocol* FromLayer,
        sockaddr fromaddr ) {

    return (cOk);
}


ErrorT TCPTransportProtocol::recv( packet_layer* packet, AddressProtocol* FromLayer,
        sockaddr fromaddr ) {

    return (cOk);
}

static err_t tcp_recv_wrapper(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t error) {

	Socket *sock = (Socket*) arg;

	if (p != 0) {
		if (sock != 0) {
			return (sock->addMessage(p,0));
		} else
			return (ERR_BUF);
	} else {
		/* p == 0 indicates a disconnect */
		if (sock != 0)
			sock->disconnected(error);

		LOG(COMM,DEBUG,(COMM,DEBUG,"TCP: closing pcb: %x",pcb));
		tcp_close(pcb);
	}

	return (ERR_OK);
}

void TCPTransportProtocol::received(Socket* socket, pbuf* p) {
	struct tcp_pcb* pcb = (struct tcp_pcb*) socket->arg;
	// indicate that we received
	tcp_recved(pcb,p->len);
	pbuf_free(p);
}


static err_t tcp_connected(void *arg, struct tcp_pcb *pcb, err_t err) {
	LOG(COMM,TRACE,(COMM,TRACE,"TCP: connected!"));
	tcp_recv(pcb, &tcp_recv_wrapper);

	// signal conenction to socket so it can unblock the thread
	Socket *sock = (Socket*) arg;
	sock->connected(cOk);
	sock->arg = pcb;
	return (ERR_OK);
}

static err_t  tcp_accept_wrapper(void *arg, struct tcp_pcb *newpcb,err_t err)
{

	Socket* oldsock = (Socket*) arg;
	LOG(COMM,DEBUG,(COMM,DEBUG,"TCP: accept wrapper: newpcb: %x",newpcb));

	/* we are not accepting connections if the thread is not ready and waiting
	   for connections */
	if (!oldsock->hasListeningThread) return (ERR_ABRT);

	/* Create a new socket for the connection */
	LOG(COMM,DEBUG,(COMM,DEBUG,"TCP: accepted..."));
	tcp_recv(newpcb, &tcp_recv_wrapper);

	Socket* newsock = new Socket(oldsock->getAProto()->getId(), oldsock->getType(), oldsock->getTProto()->getId());
	newsock->connected(cOk);

	oldsock->newSocketID = newsock->getId();
	tcp_arg(newpcb,newsock);
	newsock->arg = newpcb;

	oldsock->blockedThread->getOwner()->aquiredResources.addTail((DatabaseItem*) newsock);
	oldsock->blockedThread->unblock();

	return (ERR_OK);
}


ErrorT TCPTransportProtocol::connect(AddressProtocol* nextLayer, sockaddr *toaddr, Socket* fromsocket) {

	LOG(COMM,TRACE,(COMM,TRACE,"TCP: trying to connect!"));

	struct tcp_pcb* pcb = (struct tcp_pcb*) fromsocket->arg;
	if (pcb != 0) {

		LOG(COMM,TRACE,(COMM,TRACE,"TCP: pcb fine!"));
		// TODO since we support IPv6 as well add this information to the sockaddr structure
		// so we can use IPv6 too
		struct ip_addr ipaddr;
		ipaddr.version = 4;
		ipaddr.addr.ip4addr.addr = toaddr->sa_data;

		tcp_connect(pcb,&ipaddr,toaddr->port_data,&tcp_connected);
		return (cOk);
	}

	return (cError);
}

ErrorT TCPTransportProtocol::listen( Socket* socket) {


	struct tcp_pcb* pcb = (struct tcp_pcb*) socket->arg;
	LOG(COMM,DEBUG,(COMM,DEBUG,"TCP: Setting up Listining PCB: %x",pcb));

	/* stop if this pcb is already listening for connections */
	if (pcb->state == LISTEN) return cOk;

	if (pcb != 0) {
		pcb = tcp_listen(pcb);
		if (pcb != 0) {
			socket->arg = pcb;
			LOG(COMM,DEBUG,(COMM,DEBUG,"TCP: PCB set to LISTEN Mode!"));
			tcp_accept(pcb, &tcp_accept_wrapper);
			return (cOk);
		}
	}

	return (cError);
}

ErrorT TCPTransportProtocol::register_socket( unint2 port, Socket* socket ) {

	struct tcp_pcb *pcb;
	/* Create a new TCP PCB. */
	pcb = tcp_new();
	tcp_bind(pcb, 0, port);
	tcp_arg(pcb,socket);

	socket->arg = pcb;

	return (cOk);
}

ErrorT TCPTransportProtocol::unregister_socket( Socket* socket ) {

	if (socket->arg != 0){
		tcp_close((struct tcp_pcb*) socket->arg);
		tcp_arg((struct tcp_pcb*) socket->arg,0);
		return (cOk);
	}
    return (cError);
}

#endif

#endif
