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

#include "IPv4AddressProtocol.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

IPv4AddressProtocol::IPv4AddressProtocol(Directory* p_commdevsdir) :
        AddressProtocol(0x800, p_commdevsdir) {
}

IPv4AddressProtocol::~IPv4AddressProtocol() {
}

/*****************************************************************************
 * Method: IPv4AddressProtocol::bind(sockaddr* addr, Socket* sock)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT IPv4AddressProtocol::bind(sockaddr* addr, Socket* sock) {
    // lwip stuff
    return (cOk);
}

/*****************************************************************************
 * Method: IPv4AddressProtocol::unbind(sockaddr* addr, Socket* sock)
 *
 * @description
 *
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT IPv4AddressProtocol::unbind(sockaddr* addr, Socket* sock) {
    // lwip stuff
    return (cOk);
}

