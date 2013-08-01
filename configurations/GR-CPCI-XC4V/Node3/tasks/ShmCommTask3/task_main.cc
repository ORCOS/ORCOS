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

	char* socketbufferShm = (char*) malloc(1000);
	char* mysocketbuffer = (char*) malloc(800);

	int socketShm = socket(0x800, SOCK_DGRAM,6, socketbufferShm,1000);
	int mysock = socket(0x800,SOCK_DGRAM,6,mysocketbuffer,800);

	unint4 dest_addr = IP4_ADDR(192,168,0,3); // communicate with host with address 2

	// bind our socket to some address
	sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));

	short destaddr = 1; // communicate with Master
	sockaddr* addrShm = (sockaddr*) malloc(sizeof(sockaddr));
	addrShm->port_data = 	80; 			//< the port
	addrShm->sa_data = 	destaddr;

	addr->port_data = 	80; 			//< the port
	addr->sa_data = 	dest_addr;  	//< 2 the address of the other host

	bind(socketShm, addrShm);
	bind(mysock,addr);

	serial_id = fopen("dev/comm/serial0");
	fwriteString("Node2: Task1: bound socket to port 80.\n\r",serial_id);
	fclose(serial_id);


	// set the port to 80 since we want to send some data there
	addr->port_data = 80;

	// sleep so our server thread gets the chance to set up its socket :)
	sleep(3000);

	char data[1];
	data[0] = 1;

	while(1)
	{
		// sendto the other task on localhost


		sleep(1000);

		// finally receive our message again
		//char* msgptr;
		//int msglen = recv(mysock,&msgptr,MSG_WAIT);

		//if (msglen > 0)
		//{
		sendto(socketShm,"Message from Node 2", 19, addrShm);
		//sendto(mysock,data,1,addr);
		//}

		sleep(1000);

	}
}
