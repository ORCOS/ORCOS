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

#include "UDPTransportProtocol.hh"
#include "lwipopts.h"

#if LWIP_UDP

#include <memtools.hh>
#include "kernel/Kernel.hh"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "comm/Socket.hh"

extern Kernel* theOS;

extern Mutex* comStackMutex;

UDPTransportProtocol::UDPTransportProtocol() :
        TransportProtocol(17) {
}

UDPTransportProtocol::~UDPTransportProtocol() {
}

/*****************************************************************************
 * Method: UDPTransportProtocol::sendto(packet_layer* payload,
 *                                      const sockaddr* fromaddr,
 *                                      const sockaddr *dest_addr,
 *                                      AddressProtocol* NextLayer,
 *                                      Socket* fromsock)
 *
 * @description
 *
 *******************************************************************************/
ErrorT UDPTransportProtocol::sendto(packet_layer* payload,
                                    const sockaddr* fromaddr,
                                    const sockaddr *dest_addr,
                                    AddressProtocol* NextLayer,
                                    Socket* fromsock) {
    struct udp_pcb* pcb = (struct udp_pcb*) fromsock->arg;
    if (pcb == 0)
        return (cError );

    comStackMutex->acquire();
    LOG(COMM, DEBUG, "UDP:sendto(): size: %d", payload->size);

    // allocate a pbuf
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, payload->size, PBUF_RAM);
    if (p != 0) {
        memcpy(p->payload, payload->bytes, payload->size);

        ip_addr destaddr;
        destaddr.version = 4;
        destaddr.addr.ip4addr.addr = dest_addr->sa_data;

        int ret = udp_sendto(pcb, p, &destaddr, dest_addr->port_data);

        // free anyway
        pbuf_free(p);

        // check if sending was successfull
        if (ret != ERR_OK) {
            comStackMutex->release();
            LOG(COMM, WARN, "UDP:sendto(): failed: %d", ret);
            return (cError );
        }

        comStackMutex->release();
        return (cOk );
    }

    comStackMutex->release();
    return (cError );
}

/*****************************************************************************
 * Method: UDPTransportProtocol::received(Socket* socket, pbuf* p)
 *
 * @description
 *
 *******************************************************************************/
void UDPTransportProtocol::received(Socket* socket, pbuf* p) {
    comStackMutex->acquire();
    pbuf_free(p);
    comStackMutex->release();
}

/*****************************************************************************
 * Method: udp_recv_wrapper(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
 *
 * @description
 *
 *******************************************************************************/
static void udp_recv_wrapper(void *arg, struct udp_pcb *upcb, struct pbuf *p, struct ip_addr *addr, u16_t port) {
    Socket *sock = reinterpret_cast<Socket*>(arg);

    LOG(COMM, DEBUG, "UDP received port: %d", port);

    sockaddr from;
    from.port_data = port;
    from.sa_data = addr->addr.ip4addr.addr;

    if (p != 0) {
        if (sock != 0) {
            sock->addMessage(p, &from);
        }
    }

    return;
}

/*****************************************************************************
 * Method: UDPTransportProtocol::register_socket(unint2 port, Socket* socket)
 *
 * @description
 *
 *******************************************************************************/
ErrorT UDPTransportProtocol::register_socket(unint2* port, Socket* socket) {
    struct udp_pcb *pcb;
    /* Create a new TCP PCB. */
    comStackMutex->acquire();
    pcb = udp_new();
    comStackMutex->release();
    if (pcb != 0) {
        ip_addr bindaddr;
        bindaddr.version = 4;
        bindaddr.addr.ip4addr.addr = 0;

        err_t err = udp_bind(pcb, &bindaddr, *port);
        if (err != ERR_OK) {
            LOG(COMM, ERROR, "UDP:bind to port %d failed %d", port, err);
            return (cErrorBindingPort);
        }

        *port = pcb->local_port;
        /* set rx callback */
        udp_recv(pcb, udp_recv_wrapper, reinterpret_cast<void*>(socket));
        socket->arg = pcb;

        return (cOk );
    } else {
        LOG(COMM, ERROR, "UDP:bind could not create udp_pcb ");
        socket->arg = 0;
        return (cErrorAllocatingMemory);
    }
}

/*****************************************************************************
 * Method: UDPTransportProtocol::unregister_socket(Socket* socket)
 *
 * @description
 *
 *******************************************************************************/
ErrorT UDPTransportProtocol::unregister_socket(Socket* socket) {
    if (socket->arg != 0) {
        comStackMutex->acquire();
        udp_remove((struct udp_pcb*) socket->arg);
        comStackMutex->release();
        return (cOk);
    } else {
        LOG(COMM, ERROR, "UDPTransportProtocol::unregister_socket socket->arg == 0");
    }
    return (cInvalidArgument);
}
#endif
