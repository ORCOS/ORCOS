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

#include <comm/FixedSizePBufList.hh>
#include "SCLConfig.hh"
#include "hal/CommDeviceDriver.hh"
#include "comm/AddressProtocol.hh"
#include "comm/TransportProtocol.hh"
#include "inc/types.hh"
#include "inc/const.hh"
#include "filesystem/Resource.hh"
#include "lwip/pbuf.h"
#include "db/ArrayList.hh"

#define SENDBUF_SIZE 1200
#define DEFAULT_BUFFERSIZE 1200

#define SOCKET_CONNECTED    (1 << 1)
#define SOCKET_LISTENING    (1 << 2)
#define SOCKET_DISCONNECTED (1 << 0)

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

    //! The message buffer of received msgs
    FixedSizePBufList* messageBuffer;

    //! The type of the socket (SOCK_STREAM, SOCK_DGRAM)
    SOCK_TYPE type;

    //! The socket we are connected to if this is a connection oriented socket
    sockaddr connected_socket;

    //! The addr this socket is bound to
    sockaddr myboundaddr;

    //! The current state of the socket (CONNECTED, LISTENING..)
    int state;

    //! Array of accepted connections if this socket is in listening state
    ArrayList* acceptedConnections;

    Mutex* mutex;

    unint4 m_lock;

public:
    //! Thread blocked by this socket due to recv call on empty message buffer
    Kernel_ThreadCfdCl* blockedThread;


#ifdef HAS_Kernel_ServiceDiscoveryCfd
    servicedescriptor sd;
#endif

    //! is true if a thread is currently waiting for new connections
    bool hasListeningThread;

public:
    void* arg;

    /*
     * Socket Constructor.
     * Initializes the new socket in the given domain using the protocol
     * and the given socket Type.
     * May produce an invalid configuration of the socket. Check isValid()
     * if the socket is correctly created.
     */
    Socket(unint2 domain, SOCK_TYPE e_type, unint2 protocol);

    ~Socket();

    /*****************************************************************************
     * Method: bind(sockaddr* address)
     *
     * @description
     * bind the socket to an addr
     *******************************************************************************/
    ErrorT bind(sockaddr* address);

    /*****************************************************************************
     * Method: sendto(const void* buffer, unint2 length, const sockaddr *dest_addr)
     *
     * @description
     * send message which sends a given message to a known socket address
     *******************************************************************************/
    int sendto(const void* buffer, unint2 length, const sockaddr *dest_addr);

    /*****************************************************************************
     * Method: sendto(const void* buffer, unint2 length, const char* service_name)
     *
     * @description
     * send message which sends a given message to a service
     *******************************************************************************/
    int sendto(const void* buffer, unint2 length, const char* service_name);

    /*****************************************************************************
     * Method: connect(Kernel_ThreadCfdCl* thread, sockaddr* toaddr)
     *
     * @description
     *  try to connect to a given socket address! always blocking!
     *******************************************************************************/
    int connect(Kernel_ThreadCfdCl* thread, sockaddr* toaddr, int timeout_ms = 2000);

    /*****************************************************************************
     * Method: listen(Kernel_ThreadCfdCl* thread)
     *
     * @description
     *  listen to a given socket address
     *******************************************************************************/
    int listen(Kernel_ThreadCfdCl* thread);

    /*****************************************************************************
     * Method: connected(int error)
     *
     * @description
     *  callback from transportprotocol on connection
     *******************************************************************************/
    void connected(int error);

    /*****************************************************************************
     * Method: accepted(Socket* newConnection)
     *
     * @description
     * callback from transportprotocol on newly accepted connections
     *******************************************************************************/
    int accepted(Socket* newConnection);


    /*****************************************************************************
     * Method: disconnected(int error)
     *
     * @description
     *  callback from transportprotocol on disconnect
     *******************************************************************************/
    void disconnected(int error);

    /*****************************************************************************
     * Method: addMessage(pbuf* p, sockaddr *fromaddr)
     *
     * @description
     *  method which is called from the transport protocol whenever this socket receives a message
     *******************************************************************************/
    ErrorT addMessage(pbuf* p, sockaddr *fromaddr);


    /*****************************************************************************
     * Method: recvfrom(Kernel_ThreadCfdCl* thread,
                    char*      data_addr,
                    size_t     data_size,
                    unint4     flags,
                    sockaddr*  addr,
                    unint4     timeout = 0)
     *
     * @description
     *  method which is called by syscalls in order to receive a message
     *  and the address the method came from
     *******************************************************************************/
    size_t recvfrom(Kernel_ThreadCfdCl* thread,
                    char*      data_addr,
                    size_t     data_size,
                    unint4     flags,
                    sockaddr*  addr,
                    unint4     timeout = 0);

    /*****************************************************************************
     * Method: isValid()
     *
     * @description
     *
     * @returns
     *  bool        True if valid.
     *******************************************************************************/
    inline bool isValid() {
        return (this->tproto != 0 && this->aproto != 0 && messageBuffer != 0);
    }

    /*****************************************************************************
     * Method: getType()
     *
     * @description
     *  Returns the type of this socket
     *******************************************************************************/
    inline SOCK_TYPE getType() {
        return (type);
    }

    /*****************************************************************************
     * Method: getAddressProtocol()
     *
     * @description
     *  Returns the address protocol used by this socket
     *******************************************************************************/
    inline AddressProtocol* getAddressProtocol() {
        return (aproto);
    }

    /*****************************************************************************
     * Method: getTransportProtocol()
     *
     * @description
     * Returns the transport protocol used by this socket
     *******************************************************************************/
    inline TransportProtocol* getTransportProtocol() {
        return (tproto);
    }
};

#endif /*SOCKET_HH_*/
