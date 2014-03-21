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

#ifndef UDPTRANSPORTPROTOCOL_HH_
#define UDPTRANSPORTPROTOCOL_HH_


#include "comm/TransportProtocol.hh"
#include "inc/types.hh"
#include "inc/const.hh"
#include "comm/Socket.hh"


/*!
 * \brief LWIP Wrapper for the UDP protocol
 * \ingroup comm
 *
 */
class UDPTransportProtocol: public TransportProtocol {

public:
	UDPTransportProtocol();

    virtual ~UDPTransportProtocol();

    //! Send method which adds the protocol header
    ErrorT send( packet_layer* payload, AddressProtocol* NextLayer, Socket* fromsock );

    // ignore sendto since we are conncetion oriented
    ErrorT sendto( packet_layer* payload, const sockaddr* fromaddr,
                const sockaddr *dest_addr, AddressProtocol* NextLayer, Socket* fromsock );


    //! Receive method which extracts the data out of the packet
    ErrorT recv( char* packetstart, int packetlength, AddressProtocol* FromLayer, sockaddr fromaddr );

    //! Receive method for packets using the packet_layer structure
    ErrorT recv( packet_layer* packet, AddressProtocol* FromLayer, sockaddr fromaddr );

    void received(Socket* socket, pbuf* p);

    //! Register a socket on the following so it can receive messages
    ErrorT register_socket( unint2 port, Socket* socket );

    ErrorT connect(AddressProtocol* nextLayer, sockaddr *toaddr, Socket* fromsocket);

    ErrorT listen(Socket* socket);

    //! Unregister a socket (after that it can not receive a message anymore)
    ErrorT unregister_socket( Socket* socket );
};

#endif
