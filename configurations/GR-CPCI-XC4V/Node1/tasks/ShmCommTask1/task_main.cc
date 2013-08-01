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

	// shm communication
	char* socketbufferShm = (char*) malloc(1000);
	int socketShm = socket(0x800, SOCK_DGRAM,6, socketbufferShm,1000);

	short destAddr = 2;
	sockaddr* addrShm = (sockaddr*) malloc(sizeof(sockaddr));
	addrShm->port_data = 80;
	addrShm->sa_data   = destAddr;

	bind(socketShm, addrShm);
	serial_id = fopen("dev/comm/serial0");
	fwriteString("Master: Task1: bound socket to port 80.\n\r",serial_id);
	fclose(serial_id);


	sleep(2000);

	while(1)
	{
		sleep(100);

		char* msgptr;
		int msglen = recv(socketShm,&msgptr,MSG_WAIT);

		serial_id = fopen("dev/comm/serial0");

		if (msglen > 0)
		{
			fwriteString("Master received: ",serial_id);
			fwrite(msgptr,msglen,1,serial_id);
			fwriteString("\n\r",serial_id);
		}

		fclose(serial_id);

		sleep(100);

	}
}
