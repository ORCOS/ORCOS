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

#ifndef TCPTRANSPORTPROTOCOL_HH_
#define TCPTRANSPORTPROTOCOL_HH_

#include "comm/TransportProtocol.hh"
#include "inc/types.hh"
#include "inc/const.hh"
#include "comm/Socket.hh"

/*!
 * \brief LWIP Wrapper for the TCP protocol
 * \ingroup comm
 *
 */
class TCPTransportProtocol: public TransportProtocol {
public:
    TCPTransportProtocol();

    ~TCPTransportProtocol();

    /*****************************************************************************
     * Method: send(packet_layer* payload, AddressProtocol* NextLayer, Socket* fromsock)
     *
     * @description
     *
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT send(packet_layer* payload, AddressProtocol* NextLayer, Socket* fromsock);

    /*****************************************************************************
     * Method: sendto(packet_layer* payload, const sockaddr* fromaddr, const sockaddr *dest_addr, AddressProtocol* NextLayer, Socket* fromsock);
     *
     * @description
     *  ignore sendto since we are conncetion oriented
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT sendto(packet_layer* payload, const sockaddr* fromaddr, const sockaddr *dest_addr, AddressProtocol* NextLayer, Socket* fromsock);

    /*****************************************************************************
     * Method: received(Socket* socket, pbuf* p)
     *
     * @description
     *  Callback from socket which tells the transportprotocol that the given
     *  packet has been received.
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    void received(Socket* socket, pbuf* p);

    /*****************************************************************************
     * Method: register_socket(unint2 port, Socket* socket)
     *
     * @description
     *  Register a socket on the following so it can receive messages
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT register_socket(unint2 port, Socket* socket);

    /*****************************************************************************
     * Method: connect(AddressProtocol* nextLayer, sockaddr *toaddr, Socket* fromsocket)
     *
     * @description
     *
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT connect(AddressProtocol* nextLayer, sockaddr *toaddr, Socket* fromsocket);

    /*****************************************************************************
     * Method: disconnect(Socket* fromsocket)
     *
     * @description
     *
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT disconnect(Socket* fromsocket);

    /*****************************************************************************
     * Method: listen(Socket* socket)
     *
     * @description
     *
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT listen(Socket* socket);

    /*****************************************************************************
     * Method: unregister_socket(Socket* socket)
     *
     * @description
     *  Unregister a socket (after that it can not receive a message anymore)
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT unregister_socket(Socket* socket);
};

#endif /*SIMPLETRANSPORTPROTOCOL_HH_*/
