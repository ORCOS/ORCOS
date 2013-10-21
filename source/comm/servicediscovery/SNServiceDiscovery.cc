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

#include "kernel/Kernel.hh"
#include "lib/defines.h"
#include "inc/stringtools.hh"
#include "inc/memtools.hh"
#include "SNServiceDiscovery.hh"

extern Kernel* theOS;
extern Kernel_ThreadCfdCl* pCurrentRunningThread;

SNServiceDiscovery::SNServiceDiscovery() :
	ServiceDiscovery(2314) {

#if USE_WORKERTASK
	TimedFunctionCall* jobparam = new TimedFunctionCall;
	jobparam->objectptr = this; // call this object
	jobparam->parameterptr = 0;
	jobparam->time = theOS->getClock()->getTimeSinceStartup() + (1000 ms); // call the first time in 5 seconds

	// we got a workertask inside the kernel so we can assign the periodic send  job to a workerthread
	if ( !theOS->getWorkerTask()->addJob( TimedFunctionCallJob, 0, jobparam, 10000 ) ) {
		ERROR("Could not assign a Workerthread to the SNServiceDiscovery!");
	}
#else
	// kernel compiled without workertask concept
	ERROR("SNServiceDiscovery will not work without Workerthreads!");
#endif

	listenSocket = 0;

	for (int i = 0; i < 10; i++) {
			services[i].address.sa_data = -1;
		}


}

unint1 SNServiceDiscovery::nlookup(const service_name name,
		servicedescriptor* return_socks, unint1 n) {

	unint1 num = 0;
	LOG(KERNEL, TRACE, (KERNEL, TRACE, "SNServiceDiscovery::nlookup() service: '%s' this:%x",name,this));

	// look inside our local database
	for (int i = 0; i < 10; i++) {
		LOG(KERNEL, TRACE, (KERNEL, TRACE, "SNServiceDiscovery::nlookup() service addr: '%x'",services[i].address.sa_data));
		if ((services[i].address.sa_data != (unint4) -1))
		{

			if (strcmp(services[i].address.name_data, name) == 0) {
				// hit
				LOG(KERNEL, TRACE, (KERNEL, TRACE, "SNServiceDiscovery::nlookup() match!"));
				return_socks[num] = services[i];
				num++;
				if (num >= n)
					break;
			}
		}
	}

	if (num == 0)
		LOG(KERNEL, WARN, (KERNEL, WARN, "SNServiceDiscovery::nlookup() no service found!"));
	return num;
}
;

bool SNServiceDiscovery::addService(servicedescriptor* addr, bool local) {

	LOG(KERNEL, TRACE, (KERNEL, TRACE, "SNServiceDiscovery::addService()"));

	if (existsService(addr)) return false;

	// look inside our local database
	for (int i = 0; i < 10; i++) {
		if ( services[i].address.sa_data == (unint4) -1) {
			LOG(KERNEL, INFO, (KERNEL, INFO, "SNServiceDiscovery::addService() i: %d service: %s addr: %x port:%d",i,addr->address.name_data,addr->address.sa_data,addr->address.port_data));

			services[i] = *addr;
			if (local) services[i].type = SNSD_LOCAL;

			if (listenSocket != 0 && local) {
				// local service
				sockaddr target_addr;

				target_addr.port_data = 5; // multicast to port 5
				target_addr.sa_data = IP4ADDR(224, 0, 0, 15); // multicast address for our service discovery
				target_addr.name_data[0] = '\0';

				SNSD_Broadcast sd;
				// broadcast this service since its a local one
				sd.type = SNSD_SERVICE_BROADCAST;
				sd.addr = services[i];

				listenSocket->sendto(&sd, sizeof(SNSD_Broadcast), &target_addr);
			}

			return 1;

		}
	}

	return 0;
}

bool SNServiceDiscovery::removeService(servicedescriptor* addr, bool local)
{

	LOG(KERNEL, INFO, (KERNEL, INFO, "SNServiceDiscovery::removeService() service addr: '%x' name: '%s'",addr->address.sa_data,addr->address.name_data));

	for (int i = 0; i < 10; i++) {
		if (services[i].address.sa_data == addr->address.sa_data)
		{

			if (strcmp(services[i].address.name_data, addr->address.name_data) == 0) {
				// hit
				LOG(KERNEL, INFO, (KERNEL, INFO, "SNServiceDiscovery::removeService() match!"));
				unint4 sa_data = services[i].address.sa_data;
				services[i].address.sa_data = -1;


				if (listenSocket != 0 && local) {
					// local service
					sockaddr target_addr;

					target_addr.port_data = 5; // multicast to port 5
					target_addr.sa_data = IP4ADDR(224, 0, 0, 15); // multicast address for our service discovery
					target_addr.name_data[0] = '\0';

					SNSD_Broadcast sd;
					// broadcast this service since its a local one
					sd.type = SNSD_REMOVE;
					sd.addr = services[i];
					sd.addr.address.sa_data = sa_data;

					listenSocket->sendto(&sd, sizeof(SNSD_Broadcast), &target_addr);
				}


				return 1;
			}
		}
	}

	return 0;
}

void SNServiceDiscovery::broadcastServices() {
	sockaddr addr;

	addr.port_data = 5; // listen on port 5
	addr.sa_data = IP4ADDR(224, 0, 0, 15); // multicast address for our service discovery
	addr.name_data[0] = '\0';

	SNSD_Broadcast sd;

	// look inside our local database
	for (int i = 0; i < 10; i++) {
		if ((int4) services[i].type == SNSD_LOCAL) {

			// broadcast this service since its a local one
			sd.type = SNSD_SERVICE_BROADCAST;
			sd.addr = services[i];

			listenSocket->sendto(&sd, sizeof(SNSD_Broadcast), &addr);
		}
	}

}

void SNServiceDiscovery::handleBroadcast(servicedescriptor* sd,
		sockaddr* sender) {
	// copy the senders address into the service descriptor
	sd->type = SNSD_REMOTE;
	addService(sd,false);
}

void SNServiceDiscovery::handleRemoval(servicedescriptor* sd,
		sockaddr* sender) {
	// remove address
	sd->type = SNSD_REMOTE;
	removeService(sd,false);
}

void SNServiceDiscovery::callbackFunc(void* param) {


	LOG(KERNEL, INFO, (KERNEL, INFO, "SNServiceDiscovery(): Initializing"));

	// socket creation must be done here
	// since it needs a valid thread/task context!

	// initialize our communication socket!
	mysocketbuffer = (char*) theOS->getMemoryManager()->alloc(0x400,true);
	listenSocket = new Socket(cIPv4AddressProtocol, SOCK_DGRAM,
			cUDP, mysocketbuffer, 0x400);

	// bind our socket to some address
	sockaddr addr;

	addr.port_data = 5; // listen on port 5
	addr.sa_data = IP4ADDR(224, 0, 0, 15); // multicast address for our service discovery
	addr.name_data[0] = '\0';

	listenSocket->bind(&addr);


	LOG(KERNEL, INFO, (KERNEL, INFO, "SNServiceDiscovery(): Broadcasting ALIVE message!"));

	// send alive message!
	SNSD_Header sd;
	sd.type = SNSD_ALIVE;

	listenSocket->sendto(&sd, sizeof(SNSD_Header), &addr);
	LOG(KERNEL, INFO, (KERNEL, INFO, "SNServiceDiscovery: Initial Service Broadcast!"));
	broadcastServices();

	char* msgptr;
	sockaddr sender;
	int len;

	LOG(KERNEL, INFO, (KERNEL, INFO, "SNServiceDiscovery: Waiting.."));
	while (1) {
		len = listenSocket->recvfrom(pCurrentRunningThread, &msgptr,
				MSG_WAIT, &sender);

		SNSD_Header* header = (SNSD_Header*) msgptr;
		switch (header->type) {
		case SNSD_SERVICE_BROADCAST:
			LOG(KERNEL, INFO, (KERNEL, INFO, "SNServiceDiscovery: Received Service Broadcast!"));
			handleBroadcast(&((SNSD_Broadcast*) header)->addr, &sender);
			break;
		case SNSD_ALIVE:
			LOG(KERNEL, INFO, (KERNEL, INFO, "SNServiceDiscovery: ALIVE Message received! Broadcasting Services!"));
			broadcastServices();
			break;
		case SNSD_REMOVE:
			LOG(KERNEL, INFO, (KERNEL, INFO, "SNServiceDiscovery: Service removed!"));
			handleRemoval(&((SNSD_Broadcast*) header)->addr, &sender);
			break;
		default:
			LOG(KERNEL, WARN, (KERNEL, WARN, "SNServiceDiscovery: Invalid Packet received!! len: %d",len));
			break;
		};

	}

}
