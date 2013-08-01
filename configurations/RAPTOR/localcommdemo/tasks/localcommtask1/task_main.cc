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
	// first get some buffer the socket can work on (storing messages and so on)
	char* mysocketbuffer = (char*) malloc(800);

	// then create a socket
	int mysock = socket(0x800,SOCK_STREAM,6,mysocketbuffer,800);

	// bind our socket to some address
	sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));


	addr->port_data = 	80; 			        //< the port
	addr->sa_data = 	IP4_ADDR(127,0,0,1);	//< our address

	bind(mysock,addr);


	printf("Task1: bound socket to port 80.\n\r");


	// set the port to 81 since we want to send some data there
	addr->port_data =  81;
	addr->sa_data =    IP4_ADDR(127,0,0,1);

	// sleep so our server thread gets the chance to set up its socket and listens
	sleep(500);

	printf("Trying to connect\r\n");

	connect(mysock,addr);

	printf("Connected .. sending\r\n");

	while(1)
	{
		// sendto the other task on localhost
		sendto(mysock,"test",5,addr);

		sockaddr sender;

		// finally receive our message again
		char* msgptr;

		//int msglen = recvfrom(mysock,&msgptr,MSG_WAIT,&sender);
		int msglen = recv(mysock,&msgptr,MSG_WAIT);


		if (msglen > 0)
		{
		    printf("Task1 received: '%s' \n\r",msgptr);
		}
	}
}
