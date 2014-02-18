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

#ifndef SOCKET_HH_
#define SOCKET_HH_

#include "SCLConfig.hh"
#include "hal/CommDeviceDriver.hh"
#include "comm/AddressProtocol.hh"
#include "comm/TransportProtocol.hh"
#include "inc/types.hh"
#include "inc/const.hh"
#include "comm/CAB.hh"
#include "filesystem/Resource.hh"
#include "lwip/pbuf.h"
#include "FixedSizePBufList.h"

#define SENDBUF_SIZE 1200
#define DEFAULT_BUFFERSIZE 1200





/*!
 * \brief Socket class which is used for communications.
 * \ingroup comm
 */
class Socket: public Resource {
private:
    //! The addressprotocol used
    AddressProtocol* aproto;

    //! The transportprotocol used
    TransportProtocol* tproto;

    //! the message buffer
    //CAB* messageBuffer;
    FixedSizePBufList* messageBuffer;

    //! the sender addresses for every message we received
    sockaddr senderaddr[MAX_BUF];

    //! the send buffer which is used by the protocols to wrap the message
   // void* sendBuffer;

    //! The type of the socket (SOCK_STREAM, SOCK_DGRAM)
    SOCK_TYPE type;

    //! The task the socket belongs to (needed for virtual memory)
    Task* ownerTask;

    //! The socket we are connected to if this is a connection oriented socket
    sockaddr connected_socket;

    int socket_connected;

    //! The addr this socket is bound to
    sockaddr myboundaddr;

public:
    //! Thread blocked by this socket due to recv call on empty message buffer
    Kernel_ThreadCfdCl* blockedThread;

    // socket id of the newly created socket on connection acceptance
    int newSocketID;

    // is true if a thread is currently waiting for new connections
    bool hasListeningThread;

#ifdef HAS_Kernel_ServiceDiscoveryCfd
    servicedescriptor sd;
#endif

public:
	 void*	arg;

	Socket( unint2 domain, SOCK_TYPE e_type, unint2 protocol);
    ~Socket();

    //! bind the socket to an addr
    ErrorT bind( sockaddr* address );

    //! send message which sends a given message to a known socket address
    int sendto( const void* buffer, unint2 length, const sockaddr *dest_addr );

    //! send message which sends a given message to a service
    int sendto( const void* buffer, unint2 length, const char* service_name );

    //! try to connect to a given socket address! always blocking!
    int connect(Kernel_ThreadCfdCl* thread, sockaddr* toaddr );

    //! listen to a given socket address
    int listen(Kernel_ThreadCfdCl* thread);

    //! callback from transportprotocol on connection
    void connected(int error);

    //! callback from transportprotocol on disconnect
    void disconnected(int error);

    //! method which is called from the transport protocol whenever this socket receives a message
    //ErrorT addMessage( char* msgstart, unint2 msglength, sockaddr *fromaddr );
    ErrorT addMessage( pbuf* p, sockaddr *fromaddr );

    //! method which is called by syscalles in order to receive a message and the address the method came from
   // size_t recvfrom( Kernel_ThreadCfdCl* thread, char** addressof_ret_ptrtomsg, unint4 flags, sockaddr* addr );
    size_t recvfrom( Kernel_ThreadCfdCl* thread, char* data_addr, size_t data_size,  unint4 flags, sockaddr* addr ) ;

    //! method which is called by syscalles in order to receive a message
    //size_t recv( Kernel_ThreadCfdCl* thread, char** addressof_ret_ptrtomsg, unint4 flags );

    //! Returns the type of this socket
    inline
    SOCK_TYPE getType() {
        return (type);
    }
    //! Returns the address protocol
    inline
    AddressProtocol* getAProto() {
        return (aproto);
    }
    //! Returns the transport protocol
    inline
    TransportProtocol* getTProto() {
        return (tproto);
    }
    //! Returns the ownerTask
    inline
    Task* getOwnerTask() {
        return (ownerTask);
    }

};

#endif /*SOCKET_HH_*/
