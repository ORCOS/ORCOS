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

extern "C" Mutex* comStackMutex;

TCPTransportProtocol::TCPTransportProtocol() :
        TransportProtocol(6) {
}

TCPTransportProtocol::~TCPTransportProtocol() {
}

/*****************************************************************************
 * Method: TCPTransportProtocol::sendto(packet_layer* payload,
 *                                      const sockaddr* fromaddr,
 *                                      const sockaddr *dest_addr,
 *                                      AddressProtocol* NextLayer,
 *                                      Socket* fromsock)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT TCPTransportProtocol::sendto(packet_layer* payload, const sockaddr* fromaddr, const sockaddr *dest_addr, AddressProtocol* NextLayer, Socket* fromsock) {
    /* no sendto on connection oriented protocols */
    return (cError );
}

/*****************************************************************************
 * Method: TCPTransportProtocol::send(packet_layer* payload,
 *                                    AddressProtocol* NextLayer,
 *                                    Socket* fromsock)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT TCPTransportProtocol::send(packet_layer* payload, AddressProtocol* NextLayer, Socket* fromsock) {
    LOG(COMM, INFO, "TCP:send(): size: %d", payload->size);
    comStackMutex->acquire();

    // allocate a pbuf
    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, payload->size, PBUF_RAM);
    if (p != 0) {
        // unfortunately we need to copy here
        memcpy(p->payload, payload->bytes, payload->size);
        struct tcp_pcb *pcb = (struct tcp_pcb*) fromsock->arg;

        // try to write it to the tcp stack
        // will try to enqueue the packet and maybe merge it with previous ones still on queue
        int ret = tcp_write(pcb, p, p->len, 0);

        // free anyway
        pbuf_free(p);

        // check if enqueue operation was successfull
        if (ret != ERR_OK) {
            comStackMutex->release();
            return (cTCPEnqueueFailed );
        }

        // try to send directly
        tcp_output(pcb);

        comStackMutex->release();
        return (cOk );
    } else {
        comStackMutex->release();
        LOG(COMM, WARN, "TCP:send(): packet dropped.. no more memory..!");
        return (cPBufNoMoreMemory );
    }
}

/*****************************************************************************
 * Method: tcp_recv_wrapper(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t error)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
static err_t tcp_recv_wrapper(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t error) {
    Socket *sock = reinterpret_cast<Socket*>(arg);

    if (p != 0) {
        if (sock != 0) {
            return (sock->addMessage(p, 0));
        } else {
            /* socket has been destroyed already? */
            tcp_close(pcb);
            return (ERR_OK);
        }
    } else {
        /* p == 0 indicates a disconnect */
        if (sock != 0) {
            sock->arg = 0;
            sock->disconnected(error);
        }

        LOG(COMM, DEBUG, "TCP: closing pcb: %x", pcb);
        tcp_close(pcb);
    }

    return (ERR_OK);
}

/*****************************************************************************
 * Method: TCPTransportProtocol::received(Socket* socket, pbuf* p)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
void TCPTransportProtocol::received(Socket* socket, pbuf* p) {
    struct tcp_pcb* pcb = (struct tcp_pcb*) socket->arg;
    comStackMutex->acquire();
    if (pcb != 0) {
        // indicate that we received this packet
        tcp_recved(pcb, p->tot_len);
    }
    pbuf_free(p);
    comStackMutex->release();
}

/*****************************************************************************
 * Method: tcp_connected(void *arg, struct tcp_pcb *pcb, err_t err)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
static err_t tcp_connected(void *arg, struct tcp_pcb *pcb, err_t err) {
    if (arg == 0)
        return (ERR_OK);

    if (err == ERR_OK) {
        LOG(COMM, TRACE, "TCP: connected!");
        tcp_recv(pcb, &tcp_recv_wrapper);

        // signal conenction to socket so it can unblock the thread
        Socket *sock = reinterpret_cast<Socket*>(arg);
        sock->connected(cOk);
        sock->arg = pcb;
    } else {
        LOG(COMM, TRACE, "TCP: connect timed out!");
        Socket *sock = reinterpret_cast<Socket*>(arg);
        sock->connected(cError);
        sock->arg = 0;
    }
    return (ERR_OK);
}

/*****************************************************************************
 * Method: tcp_accept_wrapper(void *arg, struct tcp_pcb *newpcb, err_t err)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
static err_t tcp_accept_wrapper(void *arg, struct tcp_pcb *newpcb, err_t err) {
    Socket* oldsock = reinterpret_cast<Socket*>(arg);
    LOG(COMM, DEBUG, "TCP: accept wrapper: newpcb: %x", newpcb);

    /* decrease accepts pending counter anyway*/
    tcp_accepted(oldsock->arg);

    /* Create a new socket for the connection */
    LOG(COMM, DEBUG, "TCP: accepted...");
    tcp_recv(newpcb, &tcp_recv_wrapper);

    Socket* newsock = new Socket(oldsock->getAddressProtocol()->getId(),
                                 oldsock->getType(),
                                 oldsock->getTransportProtocol()->getId());
    /* set connection status of the new socket*/
    tcp_arg(newpcb, newsock);
    newsock->arg = newpcb;
    newsock->connected(cOk);

    /* add the new connection to the listening socket*/
    oldsock->accepted(newsock);

    return (ERR_OK);
}

/*****************************************************************************
 * Method: TCPTransportProtocol::connect(AddressProtocol* nextLayer,
 *                                       sockaddr *toaddr,
 *                                       Socket* fromsocket)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT TCPTransportProtocol::connect(AddressProtocol* nextLayer, sockaddr *toaddr, Socket* fromsocket) {
    LOG(COMM, TRACE, "TCP: trying to connect!");
    comStackMutex->acquire();
    struct tcp_pcb* pcb = (struct tcp_pcb*) fromsocket->arg;
    if (pcb != 0) {
        LOG(COMM, TRACE, "TCP: pcb fine!");
        // TODO since we support IPv6 as well add this information to the sockaddr structure
        // so we can use IPv6 too
        struct ip_addr ipaddr;
        ipaddr.version = 4;
        ipaddr.addr.ip4addr.addr = toaddr->sa_data;

        comStackMutex->release();
        tcp_connect(pcb, &ipaddr, toaddr->port_data, &tcp_connected);
        return (cOk );
    }

    comStackMutex->release();
    return (cError );
}

/*****************************************************************************
 * Method: TCPTransportProtocol::disconnect(Socket* fromsocket)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT TCPTransportProtocol::disconnect(Socket* fromsocket) {
    return (unregister_socket(fromsocket));
}

/*****************************************************************************
 * Method: TCPTransportProtocol::listen(Socket* socket)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT TCPTransportProtocol::listen(Socket* socket) {
    struct tcp_pcb* pcb = (struct tcp_pcb*) socket->arg;

    if (pcb != 0) {
        /* stop if this pcb is already listening for connections */
        if (pcb->state == LISTEN)
            return (cOk );

        LOG(COMM, DEBUG, "TCP: Setting up Listening PCB: %x", pcb);

        comStackMutex->acquire();
        pcb = tcp_listen(pcb);
        comStackMutex->release();

        if (pcb != 0) {
            socket->arg = pcb;
            LOG(COMM, DEBUG, "TCP: PCB set to LISTEN Mode!");
            tcp_accept(pcb, &tcp_accept_wrapper);
            return (cOk );
        } else {
            LOG(COMM, ERROR, "TCP: tcp_listen returned null");
            return (cError );
        }
    }

    return (cError );
}

/*****************************************************************************
 * Method: TCPTransportProtocol::register_socket(unint2 port, Socket* socket)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT TCPTransportProtocol::register_socket(unint2 port, Socket* socket) {
    comStackMutex->acquire();

    struct tcp_pcb *pcb;
    /* Create a new TCP PCB. */
    pcb = tcp_new();
    if (pcb == 0) {
        comStackMutex->release();
        LOG(COMM, ERROR, "TCP::register_socket() tcp_new return 0");
        return (cError );
    }

    err_t err = tcp_bind(pcb, 0, port);
    if (err != ERR_OK) {
        comStackMutex->release();
        LOG(COMM, ERROR, "RCP::register_socket() tcp_bind return err: %d", err);
        return (cError );
    }

    tcp_arg(pcb, socket);

    socket->arg = pcb;

    comStackMutex->release();

    return (cOk );
}

/*****************************************************************************
 * Method: TCPTransportProtocol::unregister_socket(Socket* socket)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT TCPTransportProtocol::unregister_socket(Socket* socket) {
    if (socket->arg != 0) {
        comStackMutex->acquire();
        struct tcp_pcb *pcb = (struct tcp_pcb*) socket->arg;
        err_t err = ERR_OK;

        /* check if we are too late and pcb has been closed remotely ... */
        if (pcb->state != CLOSED)
            err = tcp_close(pcb);

        if (err != ERR_OK)
            LOG(COMM, WARN, "Error closing tcp_pcb : %d", err);

        tcp_arg((struct tcp_pcb*) socket->arg, 0);
        comStackMutex->release();

        return (cOk );
    }
    return (cError );
}

#endif

#endif
