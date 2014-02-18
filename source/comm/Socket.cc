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
//#include <assembler.h>
#include "inc/stringtools.hh"
#include "inc/defines.h"

extern Kernel* theOS;

extern Task* pCurrentRunningTask;

Socket::Socket( unint2 domain, SOCK_TYPE e_type, unint2 protocol) :
    Resource( cSocket, false ) {


    // create the message buffer
    this->ownerTask 			= pCurrentRunningTask;
    //this->messageBuffer 		= new CAB( bufferstart, bufferlen , pCurrentRunningTask );
    this->messageBuffer 		= new FixedSizePBufList(40);
    this->arg					= 0;
    this->newSocketID			= -1;

    register ProtocolPool* protopool = theOS->getProtocolPool();

    this->aproto 				= protopool->getAddressProtocolbyId( domain );
    ASSERT(this->aproto);

    this->tproto 				= protopool->getTransportProtocolbyId( protocol );
    ASSERT(this->tproto);

    this->type 					= (SOCK_TYPE) e_type;

    this->blockedThread 		= 0;
    this->socket_connected 		= 0;
	this->hasListeningThread 	= false;
    this->myboundaddr.sa_data 	= 0;



#ifdef HAS_Kernel_ServiceDiscoveryCfd
    this->sd.address.name_data[0] = '\0'; // <- mark as free
#endif

}

Socket::~Socket() {

	 LOG(KERNEL,INFO,(KERNEL,INFO,"Socket::~Socket(): being destroyed!"));

	 if (this->socket_connected) {
		  this->aproto->unbind(&myboundaddr,this);
		  this->tproto->unregister_socket(this);
	  }

#ifdef HAS_Kernel_ServiceDiscoveryCfd
	// remove service from service discovery if weve got a service descriptor
	  if (strcmp(sd.address.name_data,"") != 0) {
		  // this socket has been registered as a service
		  // remove now!
		  theOS->getServiceDiscovery()->removeService(&sd,true);
	  }
#endif



}

ErrorT Socket::bind( sockaddr* address ) {

    if (myboundaddr.sa_data != 0 ){
    	return (cSocketAlreadyBoundError);
    }

    ErrorT error = cOk;

    // register this socket at transportprotocol so we can receive something from now on
    if ( tproto != 0 )
        error = tproto->register_socket( address->port_data, this );
    else
        error = cTransportProtocolNotAvailable;

    // bind at addressprotocol
    if ( aproto != 0 )
        error |= aproto->bind( address, this );
    else
        error |= cAddressProtocolNotAvailable;

    if (isOk(error))
    {
        // finally if both previous steps succeeded remember my bound address
        myboundaddr = *address;
        myboundaddr.sa_data = address->sa_data;

        LOG(COMM,INFO,(COMM,INFO,"Socket::bind(): binding socket to addr: 0x%x port: %d",myboundaddr.sa_data,myboundaddr.port_data));
    }
    else
    {
        LOG(COMM,ERROR,(COMM,ERROR,"Socket::bind(): could not bind address. Errorcode: %d",error));
    }

#ifdef HAS_Kernel_ServiceDiscoveryCfd
    if (strcmp(address->name_data,"") != 0) {
    	LOG(COMM,ERROR,(COMM,INFO,"Socket::bind(): Registering Service '%s'",address->name_data));
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

    return error;
}

void Socket::connected(int error) {
	LOG(COMM,DEBUG,(COMM,DEBUG,"Socket::connected(): status %d",error));

	if (error != cOk) {
		// we are not connected
		this->socket_connected = 0;
	} else {
		socket_connected = 1;
	}

	if (this->blockedThread != 0) {
		this->blockedThread->unblock();
		this->blockedThread = 0;
	}
}

void Socket::disconnected(int error) {
	LOG(COMM,DEBUG,(COMM,DEBUG,"Socket::disconnected(): status %d",error));
	this->socket_connected = 0;

	if (this->blockedThread != 0) {
		this->blockedThread->unblock();
		this->blockedThread = 0;
	}
}

int Socket::connect(Kernel_ThreadCfdCl* thread, sockaddr* toaddr ) {

	if (this->type == SOCK_STREAM) {

		if (socket_connected) {
			// TODO transportprotocol::disconnect()
		}

		this->socket_connected = 0;

		this->tproto->connect(this->aproto,toaddr,this);
		memcpy(&this->connected_socket,toaddr,sizeof(sockaddr));

		// block the current thread
		this->blockedThread = thread;

		// block here! this will also save the context. after we get unblocked we will exit block().
		thread->block();

		// we return here
		if (this->socket_connected != 0) {
			return (cOk);
		} else return  (cError);


	}
	else return (cError);

}

int Socket::listen(Kernel_ThreadCfdCl* thread) {

	if (this->type == SOCK_STREAM) {

		this->hasListeningThread = true;
		this->tproto->listen(this);

		// block
		this->blockedThread = thread;
		thread->block();

		this->hasListeningThread = false;
		// if we get here we got a new connection
		// the socket has been created and newSocketID contains the resourceID
		return (newSocketID);

	}
	else return (cError);

}

#if 0
ErrorT Socket::addMessage( char* msgstart, unint2 msglength, sockaddr *fromaddr ) {

	if (fromaddr != 0) {
		// be sure service name is unset!
		fromaddr->name_data[0] = '\0';

		LOG(COMM,DEBUG,(COMM,DEBUG,"Socket::addMessage(): adding msg len: %d, from: 0x%x port: %d",msglength,fromaddr->sa_data,fromaddr->port_data));
	}

	// DATAGRAM mode
	int buffer = messageBuffer->store( msgstart , msglength  );

	if (isError(buffer) || buffer > MAX_BUF)
		{
			LOG(COMM,ERROR,(COMM,ERROR,"Socket::addMessage(): CAB::store() returned error %d",buffer));
			return (cError);
		}


	// now store the address of the sender in the corresponding senderaddr array item
	if ( this->type == SOCK_DGRAM ) {
		memcpy(&senderaddr[buffer],fromaddr,sizeof(sockaddr));
	} else
		memcpy(&senderaddr[buffer],&connected_socket,sizeof(sockaddr));

	if ( this->blockedThread != 0 ) {
		LOG(COMM,DEBUG,(COMM,DEBUG,"Socket::addMessage(): unblocked thread %d",blockedThread->getId()));
		// the length of the message stored into register so it works with disabled vm

		this->blockedThread->unblock();
		this->blockedThread = 0;
	}


    return (cOk);
}
#endif


ErrorT  Socket::addMessage( pbuf* p, sockaddr *fromaddr ) {

	if (fromaddr != 0) {
		// be sure service name is unset!
		fromaddr->name_data[0] = '\0';
		LOG(COMM,DEBUG,(COMM,DEBUG,"Socket::addMessage(): adding msg len: %d, from: 0x%x port: %d",p->len,fromaddr->sa_data,fromaddr->port_data));
	}
	ErrorT res;

	LOG(COMM,DEBUG,(COMM,DEBUG,"Socket::addMessage(): adding message %x with len: %d",p,p->len));


	if ( this->type == SOCK_DGRAM ) {
	    res = messageBuffer->addPbuf(p,fromaddr);
	} else
		res = messageBuffer->addPbuf(p,&connected_socket);

	if (isError(res)){
		LOG(COMM,ERROR,(COMM,ERROR,"Socket::addMessage(): messageBuffer->add result: %d",res));
		return (res);
	}

	if ( this->blockedThread != 0 ) {
		LOG(COMM,DEBUG,(COMM,DEBUG,"Socket::addMessage(): unblocked thread %d",blockedThread->getId()));
		// the length of the message stored into register so it works with disabled vm

		this->blockedThread->unblock();
		this->blockedThread = 0;
	}


    return (cOk);
}

int Socket::sendto( const void* buffer, unint2 length, const sockaddr *dest_addr ) {


    if ( this->type == SOCK_DGRAM || this->socket_connected != 0 ) {

    	sockaddr* dest = const_cast<sockaddr*>(dest_addr);

#ifdef HAS_Kernel_ServiceDiscoveryCfd
    	 if (strcmp(dest_addr->name_data,"") != 0) {

    		 LOG(KERNEL,DEBUG,(KERNEL,DEBUG,"Socket::sendto(): service '%s' used!",dest_addr->name_data));

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
    				 LOG(KERNEL,DEBUG,(KERNEL,DEBUG,"Socket::sendto(): appropriate service found! sa_data:%x",dest->sa_data));
    			 }
    			 else {
    				 // need to create a new socket to communicate with that service
    				 LOG(KERNEL,ERROR,(KERNEL,ERROR,"Socket::sendto(): Service uses different Protocol and Domain! Need to create new Socket! "));
    			 }
    		 } else return cError;
    	 }
#endif


        // create a new linked list packet_layer structure which will contain
        // the pointer to the payload to be send
        LOG( COMM,DEBUG,(COMM,DEBUG,"Socket::sendto(): buffer: 0x%x, length: %d",buffer, length) );

        packet_layer payload_layer;
        payload_layer.bytes = (const char*) buffer;
        payload_layer.size = length;
        payload_layer.next = 0;
        payload_layer.total_size = length;

         // send the message
        if ( this->type == SOCK_DGRAM )
        {
            LOG( COMM,DEBUG,(COMM,DEBUG,"Socket::sendto(): sending packet to 0x%x, port %d, len %d",dest->sa_data,dest->port_data, length) );
            return (this->tproto->sendto( &payload_layer, &myboundaddr, dest_addr, this->aproto, this ));
        }
        else if ( this->socket_connected != 0 )
        {
            LOG( COMM,DEBUG,(COMM,DEBUG,"Socket::sendto(): sending packet to 0x%x, port %d",connected_socket.sa_data,connected_socket.port_data) );
            return (this->tproto->send(  &payload_layer, this->aproto, this ));
        }
        else
        {
            LOG( COMM,ERROR,(COMM,ERROR,"Socket::sendto(): failed!") );
            return (cNotConnected);
        }
    }

    return (cError);
}

size_t Socket::recvfrom( Kernel_ThreadCfdCl* thread, char* data_addr, size_t data_size,  unint4 flags, sockaddr* addr ) {

    LOG(COMM,TRACE,(COMM,TRACE,"Socket::recvfrom()"));

    if ( !messageBuffer->hasData() ) {

    	if (this->type == SOCK_STREAM && this->socket_connected == 0) {
			// we were disconnected
			// tell thread we are disconnected
			return (-1);
		}

        // no data available. block thread if no other thread is already waiting for data on this socket
        if ( flags & MSG_WAIT  ) {
        	if (blockedThread == 0) {

            LOG(COMM,DEBUG, (COMM,DEBUG,"Socket::recvfrom(): blocked thread %d",thread->getId()) );

            // block the current thread
            this->blockedThread = thread;

            // block here! this will also save the context. after we get unblocked we will exit block().
            thread->block();

            if (this->type == SOCK_STREAM && this->socket_connected == 0) {
            	// we were disconnected
            	// tell thread we are disconnected
            	return (-1);
            }

            // we got unblocked! now there is some data!
            LOG(COMM,DEBUG,(COMM,DEBUG,"Socket::recvfrom(): got unblocked! can finalize recv()."));

        	}
        }
        else {
            return (0);
        }
    }

    size_t len = 0;
    pbuf* pb = 0;

    len = messageBuffer->getFirst( data_addr, data_size, addr,pb);
    // tell lower layer that this data has been received
    if (len > 0 && pb != 0) {
    	LOG(COMM,DEBUG,(COMM,DEBUG,"Socket::recvfrom(): received %x with len: %d",pb,pb->len));
    	this->tproto->received(this,pb);
    } else
    	LOG( COMM, ERROR,(COMM, ERROR,"Socket::recvfrom(): messageBuffer->getFirst() returned len %d",len));



    return (len);
}


