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

extern "C" int task_main()
{

	int serial_id;

	// first get some buffer the socket can work on (storing messages and so on)
	char* mysocketbuffer = (char*) malloc(500);

	// then create a socket
	// using DATAGRAMS
	// using the buffer created above
	int mysock = socket(0x800,SOCK_DGRAM,6,mysocketbuffer,500);

	// bind our socket to some address
	sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));

	short dest_addr = 2; // communicate with host with address 2

	//addr->name_data = 	"\0        "; 			//< the service name.. just a number
	addr->port_data = 	80; 			//< the port
	addr->sa_data = 	dest_addr;  	//< 2 the address of the other host

	bind(mysock,addr);

	// sleep so our server thread gets the chance to set up its socket :)
	sleep(500);

	char data[1];
	data[0] = 1;

	while(1)
	{
		// sendto the other task on localhost
		sendto(mysock,data,1,addr);

		sleep(1000);
	}
}
