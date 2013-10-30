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

#include <orcos.hh>
#include "string.hh"

extern "C" int task_main()
{
	// the very first thing to do is to give our device an addr in the domain we are going to use
	// normally this is done by some DHCP Server, but an address can be added manually
	// the next line adds the address 1 to the device serial0 in the simpleaddress domain
	// our device will afterwards use this address to identify itself to other hosts when communicating in that domain

	//add_devaddr("dev/comm/eth0",2048,(char*) IP4_ADDR(192,168,1,8));

	char* mysocketbuffer = (char*) malloc(800);
	// then create a socket

	int mysock = socket(0x800,SOCK_DGRAM,6,mysocketbuffer,800);

	// bind our socket to some address
	sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));

	addr->port_data = 	80; 			        //< the port
	addr->sa_data = 	0;
	memcpy(addr->name_data,"PrintTask\0",10);   //< register using this service name

	bind(mysock,addr);

	printf("Echo-Server bound and waiting for messages.");

	sleep(500);

	sockaddr sender;

	while(1)
	{
		char* msgptr;
		int msglen = recvfrom(mysock,&msgptr,MSG_WAIT,&sender);
		msgptr[msglen-1] = '\0';

		if (msglen > 0)
		{
		    printf("Echo-Server received: '%s' from addr: 0x%x, port: %d\n\r",msgptr,sender.sa_data,sender.port_data);

		    // echo back
			sendto(mysock,msgptr,msglen,&sender);
		}

	}
}
