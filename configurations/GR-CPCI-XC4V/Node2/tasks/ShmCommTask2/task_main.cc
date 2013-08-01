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

	char* mysocketbuffer = (char*) malloc(1000);
	// then create a socket
	// using DATAGRAMS
	// let the kernel create its own buffer to work with with size 500
	// only works without virtual memory!
	int mysock = socket(0x800,SOCK_DGRAM,6, mysocketbuffer,1000);

	// bind our socket to some address
	sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));

	short dest_addr = 1; // communicate with Master
	addr->port_data = 	80; 			//< the port
	addr->sa_data = 	dest_addr;

	bind(mysock,addr);

	serial_id = fopen("dev/comm/serial0");
	fwriteString("Node2: Task1: bound socket to port 80.\n\r",serial_id);
	fclose(serial_id);


	// set the port to 80 since we want to send some data there
	addr->port_data = 80;

	// sleep so our server thread gets the chance to set up its socket :)
	sleep(3000);

	while(1)
	{
		// sendto the other task on localhost


		sleep(1000);

		// finally receive our message again
		//char* msgptr;
		//int msglen = recv(mysock,&msgptr,MSG_WAIT);

		//if (msglen > 0)
		//{
		sendto(mysock,"Message from Node 1", 19, addr);
		//}

		sleep(1000);

	}
}
