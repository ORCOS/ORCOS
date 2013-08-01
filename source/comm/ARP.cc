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

#include "ARP.hh"
#include <memtools.hh>
#include "kernel/Kernel.hh"

#define ARPHEADERSIZE 8
#define ARP_REQUEST 1
#define ARP_REPLY 2

/*!
 * The maximum length of addresses supported by our system
 *
 * ipv4: 4
 * ipv6: 16
 * ...
 */
#define MAX_ADDR_LEN 4

extern Kernel* theOS;
extern ThreadCfdCl* pCurrentRunningThread;
extern Task* pCurrentRunningTask;

ARP::ARP(Directory* commdevsdir) :
	AddressProtocol(2054, commdevsdir) {
	for (int i = 0; i < MAX_ARP_REQUESTS; i++)
		pending_requests[i].free = true;
}

ARP::~ARP() {
}

ErrorT ARP::lookup(char* dest_addr, int addr_len, AddressProtocol* proto) {
	// dont touch message which may be inisde the buffer at msgstart. just put our
	// header infront and send it on all devices to find the mac-address

    //LOG(KERNEL,INFO,(KERNEL,INFO,"ARP::loookup() looking up 0x%x",* ((int4*) dest_addr)));

	bool int_enabled;
	GET_INTERRUPT_ENABLE_BIT(int_enabled);

	// disable interrupts here since this is a ciritical section
	_disableInterrupts();

	// find a free request slot
	int i = 0;
	while (i < MAX_ARP_REQUESTS && !pending_requests[i].free)
		i++;
	if (i < MAX_ARP_REQUESTS) {

		int retval = cError;

		// before broadcasting store the request in our pending requests array so
		// we can finish the send after the addresse has been resolved and may resend the arppacket if we dont get an answer
		pending_requests[i].dest_addr = dest_addr;
		pending_requests[i].dest_addr_len = addr_len;
		pending_requests[i].free = false;
		pending_requests[i].count = 0;
		pending_requests[i].maxcount = 4; // try 5 times
		pending_requests[i].proto = proto;
		pending_requests[i].thread = pCurrentRunningThread;
		pending_requests[i].retval = &retval; // variable lying on the stack!

		// for now we dont activate interrupts
		// since the arp reply may arrive even before the workerthread gets activated

		// if ( int_enabled ) { _enableInterrupts(); }

		// now send the packet over all communication devices
		sendARPRequest(dest_addr, addr_len, proto);

#if USE_WORKERTASK
		// create a periodicfunctioncall job so we can count the timeout and or resend the request
		// this structure is placed on the stack. this is no problem here since
		// this method will not be returned and the workerthread can access this structure until the
		// request is finished
		PeriodicFunctionCall* jobparam = new PeriodicFunctionCall;
		jobparam->functioncall.objectptr = this; // call this object
		jobparam->functioncall.parameterptr = (void*) i; // store the index of the request
		jobparam->functioncall.time = theOS->getClock()->getTimeSinceStartup()
				+ 5000 ms ; // call the first time in 5 seconds
		jobparam->period = 5000 ms ; // set to 5 seconds

		// we got a workertask inside the kernel so we can assign the periodic send  job to a workerthread
		pending_requests[i].pWThread = theOS->getWorkerTask()->addJob(
				PeriodicFunctionCallJob, pCurrentRunningTask->getId(),
				jobparam, 10000);
#else
		// kernel compiled without workertask concept. set assigned workerthread to 0
		pending_requests[ i ].pWThread = 0;
#endif

		// the currently running thread is executing this code (the same thread that wanted to send a message)
		// block this thread now. sending will be finalized by the worker thread if the address could be resolved

		pCurrentRunningThread->block();
		// since we got unblocked the ARP Request has finished! Either the address is known now or it could not be resolved!
		return retval;
	}

	if (int_enabled) {
		_enableInterrupts();
	}

	return cError;

}

ErrorT ARP::recv(char* packetstart, int packetlength,
		CommDeviceDriver* fromDevice) {
	// get the ARP header
	ARPPacketHeader* header;
	header = (ARPPacketHeader*) packetstart;

	LOG(COMM,INFO,(COMM,INFO,"ARP packet received."));
	// incoming address resolution request
	// look if we are the host which wants to be looked up
	// first find the address protocol specified by the packet
	AddressProtocol* aproto = theOS->getProtocolPool()->getAddressProtocolbyId(
			header->ar_pro);
	if (aproto != 0) {
		LOG(COMM,INFO,(COMM,INFO,"ARP: Protocol %d known.",header->ar_pro));

		unint4 localaddr;
		aproto->getLocalDeviceAddress(fromDevice, localaddr);

		//char* dest_addr = (char*) ( ((int) packetstart) + packetlength - header->ar_pln );
		//char* from_addr = (char*) ( ((int) packetstart) + packetlength - 2 * header->ar_pln - header->ar_hln );
		//char* from_mac_addr = (char*) ( ((int) packetstart) + packetlength - 2 * header->ar_pln - 2 * header->ar_hln );

		char* dest_addr =
				(char*) (((int) packetstart) + sizeof(ARPPacketHeader)
						+ header->ar_pln + 2 * header->ar_hln);
		char* from_addr = (char*) (((int) packetstart)
				+ sizeof(ARPPacketHeader) + header->ar_hln);
		char* from_mac_addr = (char*) (((int) packetstart)
				+ sizeof(ARPPacketHeader));

		LOG(KERNEL,TRACE,(KERNEL,TRACE,"ARP local_addr: %x, dest_addr: %x, from_addr: %x.",localaddr,* ((unint4*)dest_addr),* ((unint4*)from_addr)));

		unint4 from;
		memcpy(&from, from_addr, header->ar_pln);

		// tell protocol that address 'from_addr' can be found on the device 'fromDevice'
		aproto->addNeighborInfo(from, fromDevice, from_mac_addr, header->ar_hln);

		if (memcmp(dest_addr, &localaddr, header->ar_pln) == 0) {

			// we are the target/destination host
			// look at the operation code

			if (header->ar_op == ARP_REQUEST) {
				LOG(COMM,INFO,(COMM,INFO,"ARP Request!"));
				// create a reply message
				// we can use the old buffer and overwrite the message
				// since all address lengths will be the same
				header->ar_op = ARP_REPLY;

				// copy the address of the sender into the destination address field first
				memcpy(dest_addr, from_addr, header->ar_pln);

				// then copy the mac address of the sender into the destination mac address field
				memcpy((void*) (((int) dest_addr) - header->ar_hln),
						from_mac_addr, header->ar_hln);

				// now fill the from address and mac addres fields with our mac-/address.
				memcpy((void*) (((int) dest_addr) - header->ar_hln
						- header->ar_pln), &localaddr, header->ar_pln);
				memcpy((void*) (((int) dest_addr) - header->ar_hln
						- header->ar_pln - header->ar_hln),
						fromDevice->getMacAddr(), header->ar_hln);

				packet_layer mylayer;
				mylayer.bytes = (char*) packetstart;
				mylayer.size = packetlength;
				mylayer.total_size = packetlength;
				mylayer.next = 0;

				// ready to send back the message
				//fromDevice->broadcast( packetstart, packetlength, this->getId() );
				fromDevice->broadcast(&mylayer, this->getId());
				// done. return to worker thread
			} else if (header->ar_op == ARP_REPLY) {
				LOG(COMM,INFO,(COMM,INFO,"ARP Reply!"));
				// we got a reply here for us
				// we can now finish the corresponding send request
				// if still alive
				// first find the pending request for the remote hosts address
				for (int i = 0; i < MAX_ARP_REQUESTS; i++) {
					if ((int) pending_requests[i].dest_addr_len
							== header->ar_pln) {
						unint4 pid;
						GETPID(pid);
						ThreadCfdCl* thread =
								(ThreadCfdCl*) pending_requests[i].thread;

						// set pid to calling process so we can access the variable on the stack
						SETPID(thread->getOwner()->getId());

						// this entry has the same address length so check if addresses are the same
						if (memcmp(pending_requests[i].dest_addr, from_addr,
								header->ar_pln) == 0) {
							// we found a pending request of a thread. go set the pid of the worker thread
							// who is executing this code here to that pid so it can access the memory of the thread
							*(pending_requests[i].retval) = cOk;
							SETPID(pid);

							LOG(KERNEL,INFO,(KERNEL,INFO,"ARP lookup succeeded. tried %d times.",++pending_requests[ i ].count ));

							// stop the workerthread assigned to this ARPRequest
							if (pending_requests[i].pWThread != 0)
								pending_requests[i].pWThread->stop();

							// marks as free again
							pending_requests[i].free = true;

							LOG(COMM,INFO,(COMM,INFO,"ARP unblocking thread %d .",thread->getId() ));

							// finally unblock the thread again since it finished sending
							thread->unblock();

						}
					}
				}

				// we are done
			}
		}
	}
	return cOk;
}

void ARP::sendARPRequest(char* dest_addr, int addr_len, AddressProtocol* proto) {

	if (addr_len == 4) {
		LOG(KERNEL,INFO,(KERNEL,INFO,"ARP asking for address 0x%x.",* ((int4*) dest_addr)));
	} else {
		if (addr_len == 2)
			LOG(COMM,INFO,(COMM,INFO,"ARP asking for address 0x%x.",* ((int2*) dest_addr)));
	}

	// ok enough space to send our message
	// for all devices go on and create a packet
	LinkedListDatabase* devices =
			const_cast<LinkedListDatabase*> (this->commdevsdir->getContent());
	LinkedListDatabaseItem* litem = devices->getHead();
	while (litem != 0) {
		CommDeviceDriver* comm_dev = (CommDeviceDriver*) litem->getData();

		char buffer[ARPHEADERSIZE + MAX_ADDR_LEN * 4 + 20];
		int bufferend = ((int) &buffer) + ARPHEADERSIZE + MAX_ADDR_LEN * 4 + 20;

		ARPPacketHeader header;
		header.ar_hln = comm_dev->getMacAddrSize();
		header.ar_pln = addr_len;
		header.ar_op = ARP_REQUEST;
		header.ar_pro = proto->getId();
		header.ar_hrd = comm_dev->getHardwareAddressSpaceId();
		// store addresses in buffer
		// first target address we want to lookup
		int pos = (int) bufferend - header.ar_pln;
		memcpy((void*) pos, dest_addr, header.ar_pln);

		// now store mac address we dont know (great protocol *g*)
		// we just dont care what to write
		pos = pos - header.ar_hln;

		// now store my addr
		// get address from addressprotocol
		pos = pos - header.ar_pln;
		unint4 localaddr;
		proto->getLocalDeviceAddress(comm_dev, localaddr);
		memcpy((void*) pos, &localaddr, header.ar_pln);

		// last store my mac address
		pos = pos - header.ar_hln;
		memcpy((void*) pos, comm_dev->getMacAddr(), header.ar_hln);

		// finally put header infront
		pos = pos - ARPHEADERSIZE;
		memcpy((void*) pos, &header, ARPHEADERSIZE);

		packet_layer mylayer;
		mylayer.bytes = (char*) pos;
		mylayer.size = ARPHEADERSIZE + 2 * header.ar_pln + 2 * header.ar_hln;
		mylayer.total_size = ARPHEADERSIZE + 2 * header.ar_pln + 2
				* header.ar_hln;
		mylayer.next = 0;

		// send broadcast
		comm_dev->broadcast(&mylayer, this->getId());

		// get next device and broadcast there
		litem = litem->getSucc();
	}

}

void ARP::callbackFunc(void* param) {
	ARPRequest* request = &pending_requests[(int) param];
	// send the request again
	sendARPRequest(request->dest_addr, request->dest_addr_len, request->proto);
	// increase the send counter
	request->count++;

	// finish if we cant resolve it
	if (request->count >= request->maxcount) {
		LOG(COMM,ERROR,(COMM,ERROR,"ARP lookup failed. tried %d times.",request->count ));
		// marks as free again
		request->free = true;

		*(request->retval) = cError;

		// finally unblock the thread again
		request->thread->unblock();

		// block this workerthread since lookup failed
		request->pWThread->stop();
	}

}
