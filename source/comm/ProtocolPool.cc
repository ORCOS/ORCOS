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

#include "ProtocolPool.hh"
#include "kernel/Kernel.hh"

#include "comm/IPv4AddressProtocol.hh"

extern Kernel* theOS;

ProtocolPool::ProtocolPool() {
    this->addressprotocols = new ArrayDatabase( 1 );
    this->transportprotocols = new ArrayDatabase( 1 );
    

    Directory* devdir = theOS->getFileManager()->getDirectory( "dev/comm" );

	this->addressprotocols->addTail((DatabaseItem*) new IPv4AddressProtocol(devdir));
#if LWIP_TCP
    this->transportprotocols->addTail((DatabaseItem*) new TCPTransportProtocol());
#endif
}

ProtocolPool::~ProtocolPool() {
}

AddressProtocol*
ProtocolPool::getAddressProtocolbyId( int2 id ) {
    // search database for protocol with id 'id'

    for ( int i = 0; i < addressprotocols->size(); i++ ) {
        AddressProtocol* ap = (AddressProtocol*) addressprotocols->getItemAt( i );
        if ( ap->getId() == id )
            return ap;
    }

    return 0;
}

TransportProtocol*
ProtocolPool::getTransportProtocolbyId( int2 id ) {
    // search database for protocol with id 'id'

    for ( int i = 0; i < transportprotocols->size(); i++ ) {
        TransportProtocol* tp = (TransportProtocol*) transportprotocols->getItemAt( i );
        if ( tp->getId() == id )
            return tp;
    }

    return 0;
}
