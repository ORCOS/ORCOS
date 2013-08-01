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

	int i = 0;

	int cd_id;
	short led2on = 2;
	int leds;

	sleep(5);

	while(1)
	{
		//in handle = fopen("led");
		//cd_id = fopen("dev/led0");
		//leds = fgetc(cd_id);
		//leds = leds | led2on;
		//fputc(leds,cd_id);

		sleep(2000);
		printf("Hello from Task 2! \r\n");

		//leds = fgetc(cd_id);
		//leds = leds & ~led2on;
		//fputc(leds,cd_id);
		//fclose(cd_id);

		//sleep(1500);

		i++;
	}
}
