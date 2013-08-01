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

#ifndef SNSERVICEDISCOVERY_HH_
#define SNSERVICEDISCOVERY_HH_

#include "ServiceDiscovery.hh"
#include "comm/Socket.hh"


#define SNSD_SERVICE_BROADCAST 1 // service broadcast message
#define SNSD_ALIVE 3	// alive message! will cause service broadcast
#define SNSD_MIGRATION 5
#define SNSD_REMOVE 6	// remove service broadcast


#define SNSD_LOCAL 2
#define SNSD_REMOTE 4

//! SNSD Packet Header
struct SNSD_Header {
    unint2 type;
};

//! SNSD Service Broadcast Packet
struct SNSD_Broadcast {
    unint2 type;
    servicedescriptor addr;
};

//! SNSD Service Migration Broadcast Packet
struct SNSD_Migration {
    unint2 type;
    servicedescriptor fromaddr;
    servicedescriptor toaddr;
};

/*!
 * \brief The Simple/Static Network ServiceDiscovery Base Class.
 * \ingroup comm
 *
 * This Service Discovery Class uses the IPV4 and the SimpleTransportProtocl to allow
 * Service Name to Adress Translation. It uses a simple and small local database to allow fast translation.
 * Multicast messages are used to either request a service discovery or broadcast messages to other service
 * discovery components inside the network.
 *
 * When started the Service Discovery broadcasts a "ALIVE" message to all nodes. Those nodes will then
 * send information on the services they own back to the sender.
 * Whenever a new local service is added it is multicasted to the other nodes to update the local service table.
 *
 */
class SNServiceDiscovery : public ServiceDiscovery, CallableObject {
private:
	 //! The socket we are using
	 Socket* listenSocket;

	 //! pointer to our buffer we are using for message storage
	 char* mysocketbuffer;

	 //! memory area to store the service <-> addr translation
	 servicedescriptor services[10];

	 //! Method which broadcasts the services on this node to all other nodes
	 void broadcastServices();

	 //! Handles incoming service broadcast packets
	 void handleBroadcast(servicedescriptor* sd, sockaddr* sender);

	 //! Handles incoming remove service packets
	 void handleRemoval(servicedescriptor* sd,sockaddr* sender);

public:

	SNServiceDiscovery();

    ~SNServiceDiscovery() {
    }
    ;


    //! try to lookup n addresses of a service
    unint1 nlookup(const service_name name, servicedescriptor* return_socks, unint1 n = 1);

    //! add a new service with address to the database
    bool addService(servicedescriptor* addr, bool local);

    //! remove a service from the database
    bool removeService(servicedescriptor* addr, bool local);

    bool existsService(servicedescriptor* addr) {
    	for (int i=0; i< 10; i++)
    	{
    		if ( (services[i].address.name_data == addr->address.name_data) &&
    			 (services[i].address.sa_data == addr->address.port_data) &&
    			 (services[i].address.port_data == addr->address.port_data)
    		) return true;
    	}
    	return false;
    }

    //! Callback function for the WorkerThread
    void callbackFunc( void* param );
};

#endif /*SNSERVICEDISCOVERY_HH_*/
