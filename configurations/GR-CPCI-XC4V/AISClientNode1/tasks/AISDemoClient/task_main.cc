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
#include "map.h"
#include "main.h"

int mysock  __attribute__ ((aligned (8)));;

/*
 * Enable interrupts by setting the interrupt level to 00
 */
#define _enableInterrupts() asm volatile( \
								"mov 	%%psr, %%l0;"	\
								"mov 	0x00000F00, %%l1;"	\
								"not	%%l1;"\
								"and 	%%l0, %%l1, %%l0;"	\
								"mov 	%%l0, %%psr;"	\
								"nop; nop; nop;" \
								: \
								: \
								: "%l0", "%l1" \
							)


/*
 * disable interrupts by setting the interrupt level to FF
 * ( Note: IRL 15 is unmaskable )
 */
#define _disableInterrupts() asm volatile( \
								"mov 	%%psr, %%l0;"	\
								"mov 	0x00000F00, %%l1;"	\
								"or 	%%l0, %%l1, %%l0;"	\
								"mov 	%%l0, %%psr;"	\
								"nop; nop; nop;" \
								: \
								: \
								: "%l0", "%l1" \
							)


void* thread_main(void* arg)
{
	int i = (int) arg;
	//_disableInterrupts();
	main(i,0);
	//_enableInterrupts();
	//printf("TurboDecoder: decoder thread stopping!");
}

extern "C" int task_main()
{

	//add_devaddr("dev/comm/eth0",2048,(char*) IP4_ADDR(192,168,1,9));
	//add_devaddr("dev/comm/shm0",2048,(char*) IP4_ADDR(192,168,1,9));

    // first get some buffer the socket can work on (storing messages and so on)
    char* mysocketbuffer = (char*) malloc(1024);

    // then create a socket
    mysock = socket(0x800,SOCK_DGRAM,6,mysocketbuffer,1024);

    // bind our socket to some address
    sockaddr* addr = (sockaddr*) malloc(sizeof(sockaddr));

    addr->port_data =   80;    //< the port we will be available on
    addr->sa_data =     0; 	   // do not specify let the system fill it out IP4_ADDR(192,168,1,9);
    memcpy(&addr->name_data,"TurboDeco\0",10);

    bind(mysock,addr);
	printf("TurboDecoder: bound socket to port 80.\n");

	sockaddr sender;
	int arg = 0;

	// create memory mappings for the decoder
	map_logmemory((const char*) 0x45000000,(const char*) 0x45000000, 0x4000000, 7);

	while(1)
	{
		char* msgptr;
		int msglen = recvfrom(mysock,&msgptr,MSG_WAIT,&sender);

		int threadid;
		thread_attr_t attr;
		attr.stack_size = 2048;
		attr.phase = 0;
		attr.priority = 0;
		// deadline in µs
		attr.deadline = 200000;
		attr.period = 0;

		args[arg].sender = sender;
		args[arg].msgptr = msgptr;

		thread_create(&threadid,&attr,&thread_main,(void*)arg);

		arg += 2;
		if (arg >= 10) arg = 0;

		//unint4 timestart = (unint4) getTime();

		thread_run(threadid);


        // got a new job. decode the block now!
		//map_decoder_t((void*) msgptr);

		//unint4 timeend = (unint4) getTime();
		//printf("End: %d\n",timeend);
		//printf("Duration: %d\n",timeend-timestart);

		// sendto(mysock,"1",1,&sender);
	}
}
