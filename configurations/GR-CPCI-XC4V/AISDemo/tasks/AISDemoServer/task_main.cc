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
#include "main.c"
#include "errno.h"

int mysock;

extern "C" int task_main(int instance)
{

	//add_devaddr("dev/comm/eth0",2048,(char*) IP4_ADDR(192,168,1,8));
	//add_devaddr("dev/comm/shm0",2048,(char*) IP4_ADDR(192,168,1,8));

    // first get some buffer the socket can work on (storing messages and so on)
    char* mysocketbuffer = (char*) malloc(500);

    // then create a socket
    mysock = socket(0x800,SOCK_DGRAM,6,mysocketbuffer,500);

    // bind our socket to some address
    sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));

    addr->port_data =   80;    //< the port we will be available on
    addr->sa_data =     0;
    memcpy(addr->name_data,"\0",1);   //< not using a service name

    bind(mysock,addr);
	printf("AISServer: bound socket to port 80.\n");
	//sleep(1000);
    // call the turbo decoder main method

	// create memory mappings
	map_logmemory((const char*) 0x45000000,(const char*) 0x45000000, 0x4000000, 7);

    main();

    printf("Turbo Decoder Exiting.\n\r");
}


