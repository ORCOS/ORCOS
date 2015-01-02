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
#include "synchro/Mutex.hh"

extern Kernel* theOS;

/*
 * The global communication Stack Mutex. Ensures only one thread is
 * executing inside the communication stack code at a time.
 * This ensures no race conditions to occur inside the tcp, ip, udp
 * and ethernet parts.
 *
 * This, however, leads to higher latencies for communication operations.
 */
Mutex* comStackMutex;
void*  sysArchMutex;

ProtocolPool::ProtocolPool() {
    this->addressprotocols   = new ArrayList(2);
    this->transportprotocols = new ArrayList(2);

    Directory* devdir = theOS->getFileManager()->getDirectory("dev/comm");

    if (isError(this->addressprotocols->addTail(reinterpret_cast<ListItem*>(new IPv4AddressProtocol(devdir)))))
        LOG(COMM, ERROR, "Error adding IPv4 Protocol to protocol pool");
#if LWIP_TCP
    if (isError(this->transportprotocols->addTail(reinterpret_cast<ListItem*>(new TCPTransportProtocol()))))
        LOG(COMM, ERROR, "Error adding TCP Protocol to protocol pool");
#endif
#if LWIP_UDP
    if (isError(this->transportprotocols->addTail(reinterpret_cast<ListItem*>(new UDPTransportProtocol()))))
        LOG(COMM, ERROR, "Error adding UDP Protocol to protocol pool");
#endif

    comStackMutex = new Mutex();
    sysArchMutex = reinterpret_cast<void*>(new Mutex());
}

ProtocolPool::~ProtocolPool() {
}

AddressProtocol*
ProtocolPool::getAddressProtocolbyId(unint2 id) {
    // search database for protocol with id 'id'

    for (int i = 0; i < addressprotocols->size(); i++) {
        AddressProtocol* ap = reinterpret_cast<AddressProtocol*>(addressprotocols->getItemAt(i));
        if (ap->getId() == id)
            return (ap);
    }

    return (0);
}

TransportProtocol*
ProtocolPool::getTransportProtocolbyId(unint2 id) {
    // search database for protocol with id 'id'

    for (int i = 0; i < transportprotocols->size(); i++) {
        TransportProtocol* tp = reinterpret_cast<TransportProtocol*>(transportprotocols->getItemAt(i));
        if (tp->getId() == id)
            return (tp);
    }

    return (0);
}
