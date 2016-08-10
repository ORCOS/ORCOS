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

extern Mutex* comStackMutex;

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
    int ret;

    LOG(COMM, INFO, "TCP:send(): size: %d", payload->size);
    comStackMutex->acquire();

    struct tcp_pcb *pcb = (struct tcp_pcb*) fromsock->arg;
    if (pcb == 0) {
        return (cInvalidArgument);
    }

    ret = tcp_write(pcb, payload->bytes, payload->size, TCP_WRITE_FLAG_COPY);
    if (ret != ERR_OK) {
        /* try to send directly */
        tcp_output(pcb);
        comStackMutex->release();
        fromsock->sendBlockedThread = pCurrentRunningThread;
        pCurrentRunningThread->block(1000 ms);
        /* try again! */
        comStackMutex->acquire();
        ret = tcp_write(pcb, payload->bytes, payload->size, TCP_WRITE_FLAG_COPY);
        /* check if enqueue operation was successfull now.. */
        if (ret != ERR_OK) {
            ret = cTCPEnqueueFailed;
            goto outret;
        }
    }

    /* try to send directly */
    tcp_output(pcb);

    ret = cOk;
    goto outret;

outret:
    comStackMutex->release();
    return (ret);
}


static err_t tcp_sent_wrapper(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    Socket *sock = reinterpret_cast<Socket*>(arg);

    if (sock && sock->sendBlockedThread) {
        Thread* t = sock->sendBlockedThread;
        sock->sendBlockedThread = 0;
        t->unblock();
    }
    return (ERR_OK);
}


/*****************************************************************************
 * Method: tcp_recv_wrapper(void *arg, struct tcp_pcb *pcb, struct pbuf *p, err_t error)
 *
 * @description
 *  called with comm stack mutex held..
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

        tcp_close(pcb);
        LOG(COMM, DEBUG, "TCP: closing pcb: %x", pcb);
    }

    return (ERR_OK);
}


/*****************************************************************************
 * Method:  tcp_err_wrapper(void *arg, err_t err)
 *
 * @description
 *   Error callback from lwip tcp layer.
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
static void  tcp_err_wrapper(void *arg, err_t err)
{
    Socket *sock = reinterpret_cast<Socket*>(arg);
    if (sock != 0)
    {
        LOG(COMM, WARN, "TCP Error on socket %x: %d", sock, err);
        if (ERR_IS_FATAL(err))
        {
            sock->disconnected(err);
        }
    }
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
        /* indicate that we received this packet */
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
        tcp_sent(pcb, &tcp_sent_wrapper);
        tcp_err (pcb, &tcp_err_wrapper);
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
    if (!oldsock || !newpcb)
    {
        return ERR_BUF;
    }
    struct tcp_pcb* pcb = (struct tcp_pcb*) oldsock->arg;
    if (!pcb || pcb->state != LISTEN)
    {
        // pcb has been closed..
        return ERR_BUF;
    }
    LOG(COMM, DEBUG, "TCP: accept wrapper: newpcb: %x", newpcb);

    /* decrease accepts pending counter anyway*/
    tcp_accepted(pcb);

    /* Create a new socket for the connection */
    tcp_recv(newpcb, &tcp_recv_wrapper);
    tcp_sent(newpcb, &tcp_sent_wrapper);
    tcp_err (newpcb, &tcp_err_wrapper);

    Socket* newsock = new Socket(oldsock->getAddressProtocol()->getId(),
                                 oldsock->getType(),
                                 oldsock->getTransportProtocol()->getId());
    /* set connection status of the new socket*/
    tcp_arg(newpcb, newsock);
    newsock->arg = newpcb;
    newsock->connected(cOk);

    /* add the new connection to the listening socket*/
    int error = oldsock->accepted(newsock);
    if (isOk(error)) {
        return (ERR_OK);
    }

    LOG(COMM, DEBUG, "TCP: Error accepting newpcb: %x", newpcb);
    delete newsock;
    // no further connections allowed!
    return (ERR_BUF);
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
    return (cInvalidArgument);
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
    struct tcp_pcb* lpcb;

    int ret = cOk;

    if (pcb != 0) {
        /* stop if this pcb is already listening for connections */
        if (pcb->state == LISTEN)
            return (cOk);

        LOG(COMM, DEBUG, "TCP: Socket %x, Setting up Listening PCB: %x", socket, pcb);

        comStackMutex->acquire();
        lpcb = tcp_listen(pcb);

        if (lpcb != 0) {
            tcp_arg(pcb, 0);
            // upon tcp_listen call we get an new pcb assigned.. the old one is freed
            socket->arg = lpcb;
            LOG(COMM, DEBUG, "TCP: Socket %x got new PCB %x in LISTEN Mode!", socket, lpcb);
            tcp_accept(lpcb, &tcp_accept_wrapper);
        } else {
            LOG(COMM, ERROR, "TCP: tcp_listen returned null");
            ret = cError;
        }
    }

    comStackMutex->release();
    return (ret);
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
ErrorT TCPTransportProtocol::register_socket(unint2* port, Socket* socket) {
    comStackMutex->acquire();

    struct tcp_pcb *pcb;
    /* Create a new TCP PCB. */
    pcb = tcp_new();
    if (pcb == 0) {
        comStackMutex->release();
        LOG(COMM, ERROR, "TCP::register_socket() tcp_new return 0");
        return (cErrorAllocatingMemory);
    }

    LOG(COMM, TRACE, "TCP::register_socket() Socket %x got pcb %x", socket, pcb);

    err_t err = tcp_bind(pcb, 0, *port);
    if (err != ERR_OK) {
        comStackMutex->release();
        tcp_close(pcb);
        LOG(COMM, ERROR, "TCP::register_socket() tcp_bind return err: %d", err);
        return (cErrorBindingPort);
    }

    // get assigned port
    *port = pcb->local_port;
    // set tcp pcb argument == socket
    tcp_arg(pcb, socket);
    socket->arg = pcb;

    comStackMutex->release();

    return (cOk);
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
        struct tcp_pcb *pcb = (struct tcp_pcb*) socket->arg;
        LOG(COMM, DEBUG, "Socket %x Closing tcp_pcb %x", socket, pcb);

        comStackMutex->acquire();
        tcp_arg(pcb, 0);
        socket->arg = 0;

        err_t err = ERR_OK;

        /* check if we are too late and pcb has been closed remotely ... */
        //if (pcb->state != CLOSED) {
        err = tcp_close(pcb);
        //}

        if (err != ERR_OK) {
            LOG(COMM, WARN, "Error closing tcp_pcb : %d", err);
        }

        comStackMutex->release();

        return (cOk);
    }
    return (cInvalidArgument);
}

#endif

#endif
