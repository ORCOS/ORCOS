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
#include "TestClass.hh"


void* thread_main(void* arg)
{
	int serial_id;
	serial_id = fopen("dev/comm/serial0");
	printf("Serial id: %d\n\r", serial_id);

	//while ()
	{
		fwrite("Task1 Thread2 says Hello World\n\r",32,1,serial_id);
		sleep(2000);
		fwrite("Task1 Thread2 slept well\n\r",26,1,serial_id);
	}

	fclose(serial_id);
	// return here so we are destroyed
}


extern "C" int task_main()
{

	int cd_id;
	int serial_id;
	short led1on = 1;
	char theInput [10];
	int commandEnd = 0;
	char theCommand [10];

	TestClass* t = new TestClass();
	int leds;

	// create a new thread just for fun
	int threadid;
	thread_attr_t attr;
	attr.stack_size = 3200;
	attr.phase = 0;
	attr.priority = 0;
	thread_create(&threadid,&attr,&thread_main,0);
	thread_run(threadid);


	while(1)
	{
		printf("Hello from Task 1!\r\n");

		cd_id = fopen("dev/led0");
		leds = fgetc(cd_id);
		leds = leds | led1on;
		fputc(leds,cd_id);

		sleep(1000);

		leds = fgetc(cd_id);
		leds = leds & ~led1on;
		fputc(leds,cd_id);
		fclose(cd_id);

		sleep(1000);

	}
}

