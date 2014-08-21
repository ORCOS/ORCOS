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
#include <memtools.hh>
#include <assemblerFunctions.hh>
#include "inc/stringtools.hh"
#include "inc/defines.h"

extern Kernel* theOS;
extern Task* pCurrentRunningTask;
extern Kernel_ThreadCfdCl* pCurrentRunningThread;


Socket::Socket(unint2 domain, SOCK_TYPE e_type, unint2 protocol) :
        Resource(cSocket, false) {

    /* create the message buffer. hold up to 20 messages till overflow */
    this->messageBuffer     = new FixedSizePBufList(20);
    this->arg               = 0;

    ProtocolPool* protopool = theOS->getProtocolPool();
    this->aproto            = protopool->getAddressProtocolbyId(domain);
    this->tproto            = protopool->getTransportProtocolbyId(protocol);

    this->type                  = (SOCK_TYPE) e_type;
    this->blockedThread         = 0;
    this->state                 = SOCKET_DISCONNECTED;
    this->hasListeningThread    = false;
    this->myboundaddr.sa_data   = 0;

#ifdef HAS_Kernel_ServiceDiscoveryCfd
    this->sd.address.name_data[0] = '\0';  // <- mark as free
#endif

}

Socket::~Socket() {

    LOG(COMM, DEBUG, "Socket::~Socket(): being destroyed!");

    if ((this->state & (SOCKET_CONNECTED | SOCKET_LISTENING)) || (this->type == SOCK_DGRAM))
    {
        this->aproto->unbind(&myboundaddr, this);
        this->tproto->unregister_socket(this);
    }
    if (this->messageBuffer != 0)
    {
        delete messageBuffer;
        messageBuffer = 0;
    }
    if (this->acceptedConnections != 0) {

        /* delete and close all newly accepted connections not yet
         * handled by a thread */
        Socket* t = (Socket*) acceptedConnections->removeHead();
        while (t != 0) {
            delete t;
            t = (Socket*) acceptedConnections->removeHead();
        }

        delete this->acceptedConnections;
        this->acceptedConnections = 0;
    }

#ifdef HAS_Kernel_ServiceDiscoveryCfd
    // remove service from service discovery if weve got a service descriptor
    if (strcmp(sd.address.name_data,"") != 0)
    {
        // this socket has been registered as a service
        // remove now!
        theOS->getServiceDiscovery()->removeService(&sd,true);
    }
#endif

}

ErrorT Socket::bind(sockaddr* address) {

    if (myboundaddr.sa_data != 0)
    {
        return (cSocketAlreadyBoundError );
    }

    ErrorT error = cOk;

    // register this socket at transportprotocol so we can receive something from now on
    if (tproto != 0)
        error = tproto->register_socket(address->port_data, this);
    else
        error = cTransportProtocolNotAvailable;

    // bind at addressprotocol
    if (aproto != 0)
        error |= aproto->bind(address, this);
    else
        error |= cAddressProtocolNotAvailable;

    if (isOk(error))
    {
        // finally if both previous steps succeeded remember my bound address
        myboundaddr         = *address;
        myboundaddr.sa_data = address->sa_data;

        LOG(COMM, INFO, "Socket::bind(): binding socket to addr: 0x%x port: %d",myboundaddr.sa_data,myboundaddr.port_data);
    }
    else
    {
        LOG(COMM, ERROR, "Socket::bind(): could not bind address. Errorcode: %d",error);
    }

#ifdef HAS_Kernel_ServiceDiscoveryCfd
    if (strcmp(address->name_data,"") != 0)
    {
        LOG(COMM,ERROR,"Socket::bind(): Registering Service '%s'",address->name_data);
        sd.domain = this->aproto->getId();
        sd.type = 0;
        sd.transport_protocol = this->tproto->getId();
        sd.address = *address;
#ifdef SHM0_IP4ADDR
        if (address->sa_data == 0) sd.address.sa_data = SHM0_IP4ADDR;
#endif
        theOS->getServiceDiscovery()->addService(&sd,true);
    }
#endif

    return (error);
}

void Socket::connected(int error) {
    /* listening sockets may not initiate a connection!*/
    if (state & SOCKET_LISTENING)
         return;

    if (error != cOk)
    {
        // we are not connected
        this->state     = SOCKET_DISCONNECTED;
    }
    else
    {
        state           = SOCKET_CONNECTED;
    }

    if (this->blockedThread != 0)
    {
        this->blockedThread->unblock();
        this->blockedThread = 0;
    }
}

void Socket::disconnected(int error) {
    LOG(COMM, DEBUG, "Socket::disconnected(): status %d",error);
    this->state         =  SOCKET_DISCONNECTED;

    if (this->blockedThread != 0)
    {
        this->blockedThread->unblock();
        this->blockedThread = 0;
    }
}

int Socket::connect(Kernel_ThreadCfdCl* thread, sockaddr* toaddr) {

    /* listening sockets are listening mode only!*/
    if (state & SOCKET_LISTENING)
        return (cError);

    if (this->type == SOCK_STREAM)
    {

        if (state & SOCKET_CONNECTED)
        {
            /* first disconnect if we are still connected*/
            this->tproto->disconnect(this);
        }

        this->state = SOCKET_DISCONNECTED;

        this->tproto->connect(this->aproto, toaddr, this);
        memcpy(&this->connected_socket, toaddr, sizeof(sockaddr));

        /* block the current thread */
        this->blockedThread = thread;

        /* block here! this will also save the context. after we get unblocked we will exit block(). */
        /* TODO: timeout? */
        thread->block();

        // we return here
        if (this->state == SOCKET_CONNECTED)
        {
            return (cOk );
        }
        else
        {
            return (cError );
        }

    }
    else
    {
        return (cError );
    }

}

int Socket::accepted(Socket* newConnection) {
   if (!(state & SOCKET_LISTENING))
      return (cError);


   if ( acceptedConnections!= 0) {
       int error = acceptedConnections->addTail(newConnection);

       /* wake up the waiting thread */
       if (this->blockedThread != 0)
           blockedThread->unblock();

       return (error);
   }

   return (cError);
}

int Socket::listen(Kernel_ThreadCfdCl* thread) {

    if (this->type == SOCK_STREAM)
    {

        this->state = SOCKET_LISTENING;

        /* allow 10 pending accepted connections */
        if (acceptedConnections == 0)
            acceptedConnections = new ArrayList(10);

        this->hasListeningThread = true;
        this->tproto->listen(this);


        this->blockedThread = thread;

        /* block if no new connection available */
        if (acceptedConnections->size() == 0)
            thread->block();

        this->hasListeningThread = false;

        Socket* newConn = (Socket*) acceptedConnections->removeHead();
        if (newConn != 0) {
            pCurrentRunningThread->getOwner()->aquiredResources.addTail(newConn);

            LOG(COMM, DEBUG, "Socket::listen(): new Connection %d",newConn->getId());
            /* if we get here we got a new connection.
             * A new socket has been created. Return the resource id of it.*/
            return (newConn->getId());
        }
        return (cError);

    }
    else
        return (cError );

}


ErrorT Socket::addMessage(pbuf* p, sockaddr *fromaddr) {
    ErrorT res;

    if (fromaddr != 0)
    {
        // be sure service name is unset!
        fromaddr->name_data[0] = '\0';
        LOG(COMM, DEBUG, "Socket::addMessage(): adding msg len: %d, from: 0x%x port: %d",p->len,fromaddr->sa_data,fromaddr->port_data);
    }

    LOG(COMM, DEBUG, "Socket::addMessage(): new message for Socket %d with len: %d, fromaddr: %x",this->getId(),p->len,fromaddr);

    if (this->type == SOCK_DGRAM)
    {
        LOG(COMM, TRACE, "Socket::addMessage() on SOCK_DGRAM ");
        res = messageBuffer->addPbuf(p, fromaddr);
    }
    else {
        LOG(COMM, TRACE, "Socket::addMessage() on SOCK_STREAM ");
        res = messageBuffer->addPbuf(p, &connected_socket);
    }

    if (isError(res))
    {
        LOG(COMM, ERROR, "Socket::addMessage(): messageBuffer->add result: %d",res);
        return (res);
    }

    if (this->blockedThread != 0)
    {
        LOG(COMM, DEBUG, "Socket::addMessage(): unblocked thread %d",blockedThread->getId());
        // the length of the message stored into register so it works with disabled vm

        this->blockedThread->unblock();
        this->blockedThread = 0;
    }

    return (cOk );
}

int Socket::sendto(const void* buffer, unint2 length, const sockaddr *dest_addr) {

    if ((this->type == SOCK_DGRAM) || (this->state & SOCKET_CONNECTED))
    {

        sockaddr* dest = const_cast<sockaddr*>(dest_addr);

#ifdef HAS_Kernel_ServiceDiscoveryCfd
        if (strcmp(dest_addr->name_data,"") != 0)
        {

            LOG(COMM,DEBUG,"Socket::sendto(): service '%s' used!",dest_addr->name_data);

            // get the sock_addr of this service!
            servicedescriptor return_socks[1];
            int num = theOS->getServiceDiscovery()->nlookup(dest_addr->name_data,(servicedescriptor*) &return_socks,1);

            if (num > 0)
            {
                if ((return_socks[0].domain == this->aproto->getId()) & (return_socks[0].transport_protocol == this->tproto->getId()))
                {
                    // ok we can use this socket for sending
                    dest->port_data = return_socks[0].address.port_data;
                    dest->sa_data = return_socks[0].address.sa_data;
                    memcpy(dest->name_data,"\0",1);
                    LOG(COMM,DEBUG"Socket::sendto(): appropriate service found! sa_data:%x",dest->sa_data);
                }
                else
                {
                    // need to create a new socket to communicate with that service
                    LOG(COMM,ERROR,"Socket::sendto(): Service uses different Protocol and Domain! Need to create new Socket!");
                }
            }
            else return cError;
        }
#endif

        // create a new linked list packet_layer structure which will contain
        // the pointer to the payload to be send
        LOG(COMM, DEBUG, "Socket::sendto(): buffer: 0x%x, length: %d",buffer, length);

        packet_layer payload_layer;
        payload_layer.bytes = (const char*) buffer;
        payload_layer.size = length;
        payload_layer.next = 0;
        payload_layer.total_size = length;

        // send the message
        if (this->type == SOCK_DGRAM)
        {
            LOG(COMM, DEBUG, "Socket::sendto(): sending packet to 0x%x, port %d, len %d",dest->sa_data,dest->port_data, length);
            return (this->tproto->sendto(&payload_layer, &myboundaddr, dest_addr, this->aproto, this));
        }
        else if (this->state & SOCKET_CONNECTED)
        {
            LOG(COMM, DEBUG, "Socket::sendto(): sending packet to 0x%x, port %d",connected_socket.sa_data,connected_socket.port_data);
            return (this->tproto->send(&payload_layer, this->aproto, this));
        }
        else
        {
            LOG(COMM, ERROR, "Socket::sendto(): failed!");
            return (cNotConnected );
        }
    }

    return (cError );
}

size_t Socket::recvfrom( Kernel_ThreadCfdCl* thread,
                         char* data_addr,
                         size_t data_size,
                         unint4 flags,
                         sockaddr* addr,
                         unint4     timeout) {

    LOG(COMM, TRACE, "Socket::recvfrom()");

    if (!messageBuffer->hasData())
    {

        if ((this->type == SOCK_STREAM) && (this->state & SOCKET_DISCONNECTED))
        {
            /* we were disconnected
               tell thread we are disconnected */
            return (-1);
        }

        /* no data available. block thread if no other thread is already waiting for data on this socket */
        if (flags & MSG_WAIT)
        {
            if (blockedThread == 0)
            {

                LOG(COMM, DEBUG, "Socket::recvfrom(): blocked thread %d",thread->getId());

                /* block the current thread */
                this->blockedThread = thread;

                /* block here! this will also save the context. after we get unblocked we will exit block(). */
                thread->block(timeout);

                /* check if we got unblock because of disconnect */
                if ((this->type == SOCK_STREAM) && (this->state & SOCKET_DISCONNECTED))
                {
                    /* we were disconnected
                      tell thread we are disconnected */
                    return (-1);
                }

                /* we got unblocked! now there might be some data! */
                LOG(COMM, DEBUG,"Socket::recvfrom(): got unblocked! can finalize recv().");

            } else {
                /* some other thread is already waiting on this socket ..
                 signal an error */
                return (cError);
            }
        }
        else
        {
            return (0);
        }
    }

    size_t len  = 0;
    pbuf* pb    = 0;

    len = messageBuffer->getFirst(data_addr, data_size, addr, pb);
    /* tell lower layer that this data has been received */
    if (len > 0 && pb != 0)
    {
        LOG(COMM, DEBUG,"Socket::recvfrom(): received %x with len: %d",pb,pb->len);
        this->tproto->received(this, pb);
    }
    else {
        /* if we got unblocked on timeout we will get here anyway*/
        LOG(COMM, DEBUG,"Socket::recvfrom(): messageBuffer->getFirst() returned len %d",len);
    }

    return (len);
}

