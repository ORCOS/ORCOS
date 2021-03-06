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

#include "Socket.hh"
#include "kernel/Kernel.hh"
#include "inc/memtools.hh"
#include "assemblerFunctions.hh"
#include "inc/stringtools.hh"
#include "inc/defines.h"

extern Kernel* theOS;
extern Kernel_ThreadCfdCl* pCurrentRunningThread;

Socket::Socket(unint2 domain, SOCK_TYPE e_type, unint2 protocol) :
        Resource(cSocket, false, "Socket") {

    LOG(COMM, DEBUG, "Socket::Socket() created %x", this);
    /* create the message buffer. hold up to 20 messages till overflow */
    this->messageBuffer         = new FixedSizePBufList(20);
    this->arg                   = 0;
    this->m_lock                = 0;

    ProtocolPool* protopool     = theOS->getProtocolPool();
    this->aproto                = protopool->getAddressProtocolbyId(domain);
    this->tproto                = protopool->getTransportProtocolbyId(protocol);

    this->type                  = (SOCK_TYPE) e_type;
    this->blockedThread         = 0;
    this->sendBlockedThread     = 0;
    this->state                 = SOCKET_DISCONNECTED;
    this->hasListeningThread    = false;
    this->myboundaddr.sa_data   = 0;
    this->acceptedConnections   = 0;
    memset(&this->connected_socket, 0, sizeof(sockaddr));

#ifdef HAS_Kernel_ServiceDiscoveryCfd
    this->sd.address.name_data[0] = '\0';  // <- mark as free
#endif
}

Socket::~Socket() {
    LOG(COMM, DEBUG, "Socket::~Socket(): %x being destroyed!", this);

    if ((this->state & (SOCKET_CONNECTED | SOCKET_LISTENING)) || (this->type == SOCK_DGRAM)) {
        this->aproto->unbind(&myboundaddr, this);
        this->tproto->unregister_socket(this);
    }
    if (this->messageBuffer != 0) {
        delete messageBuffer;
        messageBuffer = 0;
    }
    if (this->acceptedConnections != 0) {
        /* delete and close all newly accepted connections not yet
         * handled by a thread */
        Socket* t = static_cast<Socket*>(acceptedConnections->removeHead());
        while (t != 0) {
            delete t;
            t = static_cast<Socket*>(acceptedConnections->removeHead());
        }

        delete this->acceptedConnections;
        this->acceptedConnections = 0;
    }

#ifdef HAS_Kernel_ServiceDiscoveryCfd
    // remove service from service discovery if weve got a service descriptor
    if (strcmp(sd.address. name_data, "") != 0) {
        // this socket has been registered as a service
        // remove now!
        theOS->getServiceDiscovery()->removeService(&sd, true);
    }
#endif
}

/*****************************************************************************
 * Method: Socket::bind(sockaddr* address)
 *
 * @description
 *  Bind call on sockets. Called by user space applications to
 *  bind a socket to a given transport protocol port and address
 *  protocol address (if applicable).
 * @params
 *  address     The socket address the socket shall be bound to. See sockaddr struct.
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT Socket::bind(sockaddr* address) {
    if (myboundaddr.sa_data != 0) {
        return (cSocketAlreadyBoundError);
    }

    ErrorT error = cOk;

    /* register this socket at transportprotocol so we can receive something from now on */
    if (tproto != 0) {
        error = tproto->register_socket(&address->port_data, this);
    } else {
        error = cTransportProtocolNotAvailable;
    }

    /* bind at addressprotocol */
    if (aproto != 0) {
        error |= aproto->bind(address, this);
    } else {
        error |= cAddressProtocolNotAvailable;
    }

    if (isOk(error)) {
        /* finally if both previous steps succeeded remember my bound address */
        myboundaddr         = *address;
        myboundaddr.sa_data = address->sa_data;
        LOG(COMM, INFO, "Socket::bind(): binding socket to addr: 0x%x port: %d", myboundaddr.sa_data, myboundaddr.port_data);
    } else {
        LOG(COMM, ERROR, "Socket::bind(): could not bind address. Errorcode: %d", error);
    }

#ifdef HAS_Kernel_ServiceDiscoveryCfd
    if (strcmp(address->name_data, "") != 0) {
        LOG(COMM, ERROR, "Socket::bind(): Registering Service '%s'", address->name_data);
        sd.domain = this->aproto->getId();
        sd.type = 0;
        sd.transport_protocol = this->tproto->getId();
        sd.address = *address;
#ifdef SHM0_IP4ADDR
        if (address->sa_data == 0) sd.address.sa_data = SHM0_IP4ADDR;
#endif
        theOS->getServiceDiscovery()->addService(&sd, true);
    }
#endif

    return (error);
}

/*****************************************************************************
 * Method: Socket::connected(int error)
 *
 * @description
 *  Callback from transport protocol layer when the a connection
 *  belonging to this socket has been connected or the connection
 *  establishment failed. Error is provided inside the argument and
 *  must be either cOk (Connection successfull) or any other error code
 *  for non successfull connections.
 * @params
 *  error       Error Code. cOk on success.
 * @returns
 *  int         Error Code
 *******************************************************************************/
void Socket::connected(int error) {
    /* listening sockets may not initiate a connection!*/
    if (state & SOCKET_LISTENING)
        return;

    if (error != cOk) {
        /* we are not connected */
        this->state = SOCKET_DISCONNECTED;
    } else {
        state = SOCKET_CONNECTED;
    }

    if (this->blockedThread != 0) {
        this->blockedThread->unblock();
        this->blockedThread = 0;
    }
}

/*****************************************************************************
 * Method: Socket::disconnected(int error)
 *
 * @description
 *  Callback from transport protocol is a connection was closed.
 *
 * @params
 *  error       Error Code from transport protocol. May propagate
 *              reason the connection was closed.
 * @returns
 *  int         Error Code
 *******************************************************************************/
void Socket::disconnected(int error) {
    LOG(COMM, DEBUG, "Socket::disconnected(): status %d", error);
    this->state = SOCKET_DISCONNECTED;

    /* TODO: propagate error code to user if remotely closed. */
    SMP_SPINLOCK_GET(m_lock);
    if (this->blockedThread != 0) {
        this->blockedThread->unblock();
        this->blockedThread = 0;
    }
    SMP_SPINLOCK_FREE(m_lock);
}

/*****************************************************************************
 * Method: Socket::connect(Kernel_ThreadCfdCl* thread, sockaddr* toaddr)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
int Socket::connect(Kernel_ThreadCfdCl* thread, sockaddr* toaddr, int timeout_ms) {
    /* listening sockets are listening mode only!*/
    if (state & SOCKET_LISTENING)
        return (cError );

    if (this->type == SOCK_STREAM) {
        if (state & SOCKET_CONNECTED) {
            /* first disconnect if we are still connected*/
            this->tproto->disconnect(this);
        }

        this->state = SOCKET_DISCONNECTED;

        this->tproto->connect(this->aproto, toaddr, this);
        memcpy(&this->connected_socket, toaddr, sizeof(sockaddr));

        /* block the current thread */
        this->blockedThread = thread;

        /* block here! this will also save the context. after we get unblocked we will exit block(). */
        thread->block(timeout_ms ms);
        this->blockedThread = 0;

        /* connection success?? */
        if (this->state == SOCKET_CONNECTED) {
            return (cOk);
        } else {
            return (cErrorConnecting);
        }
    } else {
        return (cInvalidSocketType);
    }
}

/*****************************************************************************
 * Method: Socket::accepted(Socket* newConnection)
 *
 * @description
 *  Callback called by transportprotocol if a connection
 *  has been accepted.
 *
 * @params
 *  newConnection  The new socket which contains the newly
 *                 accepted connection.
 * @returns
 *  int            Error Code
 *******************************************************************************/
int Socket::accepted(Socket* newConnection) {
    if (!(state & SOCKET_LISTENING))
        return (cError);

    if (acceptedConnections != 0) {
        SMP_SPINLOCK_GET(m_lock);
        int error = acceptedConnections->addTail(newConnection);

        Thread* threadToUnblock = 0;
        if (isOk(error)) {
            /* wake up the waiting thread */
            if (this->blockedThread != 0) {
                threadToUnblock = blockedThread;
            }
        } else {
            LOG(COMM, ERROR, "Socket::accepted(): Could not accept connection. Too many connections %d", acceptedConnections->size());
        }
        SMP_SPINLOCK_FREE(m_lock);

        if (threadToUnblock != 0) {
            threadToUnblock->unblock();
        }

        return (error);
    }
    return (cError);
}

/*****************************************************************************
 * Method: Socket::listen(Kernel_ThreadCfdCl* thread)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
int Socket::listen(Kernel_ThreadCfdCl* thread, int backlog_size, int timout_ms) {
    if (this->type == SOCK_STREAM) {
        this->state = SOCKET_LISTENING;

        /* allow 10 pending accepted connections */
        if (acceptedConnections == 0)
            acceptedConnections = new ArrayList(backlog_size);

        if (!this->hasListeningThread) {
            this->hasListeningThread = true;
            int ret = this->tproto->listen(this);
            if (ret < 0)
            {
                return (ret);
            }
        }

        SMP_SPINLOCK_GET(m_lock);
        if ( this->blockedThread == 0) {
            this->blockedThread = thread;
            SMP_SPINLOCK_FREE(m_lock);
        } else {
            SMP_SPINLOCK_FREE(m_lock);
            LOG(COMM, ERROR, "Socket::listen(): Thread %x already waiting", blockedThread);
            return (cSocketAlreadyListened);
        }

        /* block if no new connection available */
        if (acceptedConnections->size() == 0) {
            thread->block(timout_ms ms);
        }

        /* got unblocked! maybe new connection? */
        SMP_SPINLOCK_GET(m_lock);
        Socket* newConn = static_cast<Socket*>(acceptedConnections->removeHead());
        SMP_SPINLOCK_FREE(m_lock);

        if (newConn != 0) {
            /* add new socket to task to allow access to it */
            int ret = pCurrentRunningThread->getOwner()->addResource(newConn);

            LOG(COMM, DEBUG, "Socket::listen(): new Connection %d", newConn->getId());
            /* if we get here we got a new connection.
             * A new socket has been created. Return the resource id of it.*/
            this->blockedThread = 0;
            return (ret);
        }

        LOG(COMM, DEBUG, "Socket::listen(): Timed out");
        /* probably socket destroyed? */
        this->blockedThread = 0;
        return (cTimeout);
    } else {
        return (cInvalidSocketType);
    }
}

/*****************************************************************************
 * Method: Socket::addMessage(pbuf* p, sockaddr *fromaddr)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT Socket::addMessage(pbuf* p, sockaddr *fromaddr) {
    ErrorT res;

    if (!messageBuffer) {
        return (cError);
    }

    if (fromaddr != 0) {
        LOG(COMM, DEBUG, "Socket::addMessage(): adding msg len: %d, from: 0x%x port: %d", p->len, fromaddr->sa_data, fromaddr->port_data);
    }

    LOG(COMM, DEBUG, "Socket::addMessage(): new message for Socket %d with len: %d, fromaddr: %x", this->getId(), p->len, fromaddr);

    SMP_SPINLOCK_GET(m_lock);
    if (this->type == SOCK_DGRAM) {
        LOG(COMM, TRACE, "Socket::addMessage() on SOCK_DGRAM ");
        res = messageBuffer->addPbuf(p, fromaddr);
    } else {
        LOG(COMM, TRACE, "Socket::addMessage() on SOCK_STREAM ");
        res = messageBuffer->addPbuf(p, &connected_socket);
    }

    if (isError(res)) {
        LOG(COMM, DEBUG, "Socket::addMessage(): messageBuffer->add result: %d", res);
    }

    Thread* threadToUnblock = 0;
    if (this->blockedThread != 0) {
        LOG(COMM, DEBUG, "Socket::addMessage(): unblocked thread %d", blockedThread->getId());
        threadToUnblock = this->blockedThread;
        this->blockedThread = 0;
    }
    SMP_SPINLOCK_FREE(m_lock);

    /* unblock thread */
    if (threadToUnblock != 0) {
        threadToUnblock->unblock();
    }

    return (cOk);
}

/*****************************************************************************
 * Method: Socket::sendto(const void* buffer, unint2 length, const sockaddr *dest_addr)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
int Socket::sendto(const void* buffer, unint2 length, const sockaddr *dest_addr) {
    ErrorT ret = cError;

    if ((this->type == SOCK_DGRAM) || (this->state & SOCKET_CONNECTED)) {
        sockaddr* dest = const_cast<sockaddr*>(dest_addr);

#ifdef HAS_Kernel_ServiceDiscoveryCfd
        if (strcmp(dest_addr->name_data, "") != 0) {
            LOG(COMM, DEBUG, "Socket::sendto(): service '%s' used!", dest_addr->name_data);

            // get the sock_addr of this service!
            servicedescriptor return_socks[1];
            int num = theOS->getServiceDiscovery()->nlookup(dest_addr->name_data, reinterpret_cast<servicedescriptor*>(&return_socks), 1);

            if (num > 0) {
                if ((return_socks[0].domain == this->aproto->getId()) & (return_socks[0].transport_protocol == this->tproto->getId())) {
                    // ok we can use this socket for sending
                    dest->port_data = return_socks[0].address.port_data;
                    dest->sa_data = return_socks[0].address.sa_data;
                    memcpy(dest->name_data, "\0", 1);
                    LOG(COMM, DEBUG, "Socket::sendto(): appropriate service found! sa_data:%x", dest->sa_data);
                } else {
                    // need to create a new socket to communicate with that service
                    LOG(COMM, ERROR, "Socket::sendto(): Service uses different Protocol and Domain! Need to create new Socket!");
                }
            } else {
                return cError;
            }
        }
#endif


        SMP_SPINLOCK_GET(m_lock);
        if (state & SOCKET_SENDING) {
            /* already another thread in send! not allowed! */
            SMP_SPINLOCK_FREE(m_lock);
            goto out;
        }

        state |= SOCKET_SENDING;
        SMP_SPINLOCK_FREE(m_lock);

        /* create a new linked list packet_layer structure which will contain
         * the pointer to the payload to be send */
        LOG(COMM, DEBUG, "Socket::sendto(): buffer: 0x%x, length: %d", buffer, length);

        /*TODO: change to pbuf */
        packet_layer payload_layer;
        payload_layer.bytes         = (const char*) buffer;
        payload_layer.size          = length;
        payload_layer.next          = 0;
        payload_layer.total_size    = length;

        // send the message
        if (this->type == SOCK_DGRAM) {
            LOG(COMM, DEBUG, "Socket::sendto(): sending packet to 0x%x, port %d, len %d", dest->sa_data, dest->port_data, length);
            ret = this->tproto->sendto(&payload_layer, &myboundaddr, dest_addr, this->aproto, this);
        } else if (this->state & SOCKET_CONNECTED) {
            LOG(COMM, DEBUG, "Socket::sendto(): sending packet to 0x%x, port %d", connected_socket.sa_data, connected_socket.port_data);
            ret = this->tproto->send(&payload_layer, this->aproto, this);
        } else {
            LOG(COMM, ERROR, "Socket::sendto(): failed!");
            ret = cNotConnected;
        }

        SMP_SPINLOCK_GET(m_lock);
        state &= ~SOCKET_SENDING;
        SMP_SPINLOCK_FREE(m_lock);
    }

out:

    return (ret);
}

/*****************************************************************************
 * Method: Socket::recvfrom(Kernel_ThreadCfdCl* thread,
 *                          char*               data_addr,
 *                          size_t              data_size,
 *                          unint4              flags,
 *                          sockaddr*           addr,
 *                          unint4              timeout)
 *
 * @description
 *   Tries to receive the next message from the socket.
 *   If no data is available blocks the thread if MSG_WAIT is
 *   set in flags.
 *   If another thread is already blocked waiting for data
 *   this method returns with an error code.
 *
 *
 * @params
 *  timeout     Timeout in milliseconds
 *
 * @returns
 *  int         Error Code if < 0. Length of message on success
 *******************************************************************************/
size_t Socket::recvfrom(Kernel_ThreadCfdCl* thread,
                        char*               data_addr,
                        size_t              data_size,
                        unint4              flags,
                        sockaddr*           addr,
                        unint4              timeout) {
    LOG(COMM, TRACE, "Socket::recvfrom()");
    int ret    = 0;
    size_t len = 0;
    pbuf* pb   = 0;

    SMP_SPINLOCK_GET(m_lock);

    if (!messageBuffer->hasData()) {
        if ((this->type == SOCK_STREAM) && (this->state & SOCKET_DISCONNECTED)) {
            /* we were disconnected
             tell thread we are disconnected */
            ret = -1;
            goto out;
        }

        /* no data available. block thread if no other thread is already waiting for data on this socket */
        if (flags & MSG_WAIT) {
            if (blockedThread == 0) {
                LOG(COMM, DEBUG, "Socket::recvfrom(): blocked thread %d", thread->getId());

                /* block the current thread */
                this->blockedThread = thread;
                /* release mutex */
                SMP_SPINLOCK_FREE(m_lock);

                /* block here! this will also save the context. after we get unblocked we will exit block(). */
                thread->block(timeout ms);

                /* on return remove reference */
                this->blockedThread = 0;

                /* check if we got unblocked because of disconnect
                 * We inform the client if no data has been received.
                 * If data is still to be received we let the client fetch those packets first
                 * before telling it about the disconnect */
                if (!messageBuffer->hasData() && (this->type == SOCK_STREAM) && (this->state & SOCKET_DISCONNECTED)) {
                    /* we were disconnected
                     tell thread we are disconnected */
                    ret = -1;
                    return (ret);
                }

                /* we got unblocked! now there might be some data! */
                LOG(COMM, DEBUG, "Socket::recvfrom(): got unblocked! can finalize recv().");

            } else {
                /* some other thread is already waiting on this socket ..
                 signal an error */
                ret = cInvalidConcurrentAccess;
                goto out;
            }
        } else {
            ret = 0;
            goto out;
        }
    } else {
        SMP_SPINLOCK_FREE(m_lock);
    }

    SMP_SPINLOCK_GET(m_lock);
    len = messageBuffer->getFirst(data_addr, data_size, addr, pb);
    /* tell lower layer that this data has been received */
    if (len > 0 && pb != 0) {
        LOG(COMM, DEBUG, "Socket::recvfrom(): received %x with len: %d", pb, pb->len);
        this->tproto->received(this, pb);
    } else {
        /* if we got unblocked on timeout we will get here anyway*/
        LOG(COMM, DEBUG, "Socket::recvfrom(): messageBuffer->getFirst() error: returned len %d", len);
    }

    ret = len;
    SMP_SPINLOCK_FREE(m_lock);
    return (ret);

out:
    SMP_SPINLOCK_FREE(m_lock);
    return (ret);
}

