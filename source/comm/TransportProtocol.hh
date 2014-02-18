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

#ifndef TRANSPORTPROTOCOL_HH_
#define TRANSPORTPROTOCOL_HH_

#include "inc/types.hh"
#include "inc/const.hh"
#include "comm/AddressProtocol.hh"
#include "inc/types.hh"
#include "lwip/pbuf.h"

class Socket;

/*!
 * \brief The TransportProtocol Base Class.
 * \ingroup comm
 *
 * This is the base class of all transportprotocols. Transportprotocols
 * are used to ensure some QoS for communications or multiplex the hosts
 * address. Its the last layer a message passes before beeing placed
 * in the applications socket mailbox.
 */
class TransportProtocol {
private:
    int2 id;

public:
    TransportProtocol( int2 protocol_id ) {
        this->id = protocol_id;
    }
    ;
    ~TransportProtocol() {
    }
    ;

    int2 getId() {
        return (id);
    }
    ;


    virtual
    ErrorT sendto( packet_layer* payload, const sockaddr* fromaddr,
            const sockaddr *dest_addr, AddressProtocol* NextLayer, Socket* fromsock ) = 0;

    virtual
    ErrorT send( packet_layer* payload, AddressProtocol* NextLayer, Socket* fromsock ) = 0;


    virtual
    ErrorT recv( char* packetstart, int packetlength, AddressProtocol* FromLayer, sockaddr fromaddr ) = 0;


    virtual void received(Socket* socket, pbuf* p) = 0;

    virtual
    ErrorT recv( packet_layer* packet, AddressProtocol* FromLayer, sockaddr fromaddr ) = 0;

    virtual
    ErrorT connect(AddressProtocol* nextLayer, sockaddr *toaddr, Socket* fromsocket) = 0;

    virtual
    ErrorT listen( Socket* socket) = 0;

    //! Register a socket on the following so it can receive messages
    virtual
    ErrorT register_socket( unint2 port, Socket* socket ) = 0;

    //! Unregister a socket (after that it can not receive a message anymore)
    virtual
    ErrorT unregister_socket( Socket* socket ) = 0;
};

#endif /*TRANSPORTPROTOCOL_HH_*/
