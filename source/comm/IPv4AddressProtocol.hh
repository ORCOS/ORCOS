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

#ifndef IPV4ADDRESSPROTOCOL_HH_
#define IPV4ADDRESSPROTOCOL_HH_

#include "comm/AddressProtocol.hh"
#include "db/ArrayList.hh"
#include "filesystem/Directory.hh"

class IPv4AddressProtocol: public AddressProtocol {
public:
    /*****************************************************************************
     * Method: IPv4AddressProtocol(Directory* commdevsdir)
     *
     * @description
     *
     *******************************************************************************/
    explicit IPv4AddressProtocol(Directory* commdevsdir);

    ~IPv4AddressProtocol();

    /*****************************************************************************
     * Method: initialize()
     *
     * @description
     *
     *******************************************************************************/
    static void initialize() { }

    /*****************************************************************************
     * Method: bind(sockaddr* addr, Socket* sock)
     *
     * @description
     *  Bind Method. If the addr is a multicast address tells the protocol to accept these multicast packets.
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT bind(sockaddr* addr, Socket* sock);

    /*****************************************************************************
     * Method: unbind(sockaddr* addr, Socket* sock)
     *
     * @description
     *  Will unbind a socket from the addresprotocol domain. For multicast
     *  registered sockets this will stop the protocol to listen for that multicast address
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT unbind(sockaddr* addr, Socket* sock);
};

#endif /*IPV4ADDRESSPROTOCOL_HH_*/
