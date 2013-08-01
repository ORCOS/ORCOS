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
	int newsockid;

	// first get some buffer the socket can work on (storing messages and so on)
	char* mysocketbuffer = (char*) malloc(800);

	// then create a socket
	int mysock = socket(0x800,SOCK_STREAM,6,mysocketbuffer,800);

	// bind our socket to some address
	sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));


	addr->port_data =   81; 						//< the port we will be available on
	addr->sa_data = 	IP4_ADDR(127,0,0,1); 	 	//< 1 is one of the addresses for our host/localhost (setup by the other task before)
	addr->name_data[0] = '\0';


	bind(mysock,addr);
	printf("Task2: bound socket to port 81.\n\r");

	newsockid = listen(mysock);


	while(1)
	{

		// receive something
		char* msgptr;

		int msglen = recv(newsockid,&msgptr,MSG_WAIT);

		if (msglen > 0)
		{
			printf("Task2 received: '%s' \n\r",msgptr);
			// just echo back the data
			sendto(newsockid,msgptr,msglen,addr);
		}

		sleep(1000);

	}
}
