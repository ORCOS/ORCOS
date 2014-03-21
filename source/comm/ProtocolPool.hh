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

#ifndef PROTOCOLPOOL_HH_
#define PROTOCOLPOOL_HH_

#include "SCLConfig.hh"
#include "lwipopts.h"

#include "db/ArrayDatabase.hh"
#include "comm/AddressProtocol.hh"
#include "comm/TransportProtocol.hh"

#if LWIP_TCP
#include "comm/TCPTransportProtocol.hh"
#endif

#if LWIP_UDP
#include "comm/UDPTransportProtocol.hh"
#endif


/*!
 * \brief Pool of address and transport protocols
 *
 * This class holds all references to transport protocols the os uses.
 * The set of protocols which shall be available by this os can be configured useing
 * the scl configuration file.
 *
 * Transportprotocols are looked up inside the Socket Class. The Transportprotocols encapuslate
 * wrapper functionalities used to communicate with lwip.
 *
 * Before the instance of this class can be created be sure that all communication devices
 * are created since protocols will need the communication devices for initialization.
 * Thus you must not create aan instance of this class before initDeviceDrivers() is called in Kernel.cc.
 *
 */
class ProtocolPool {
    ArrayDatabase* addressprotocols;

    ArrayDatabase* transportprotocols;


public:
    ProtocolPool();
    ~ProtocolPool();

    AddressProtocol* getAddressProtocolbyId( unint2 id );

    TransportProtocol* getTransportProtocolbyId( unint2 id );


};

#endif /*PROTOCOLPOOL_HH_*/
