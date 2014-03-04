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
#include <string.hh>

#define SCREEN_WIDTH 1024
char* fb;

unint4 fb_size = 0;

#define FRAMEBUFFER_SIZE 1024 * 768 * 1 * 2

void drawRect(int x, int y ,int width) {

	int r = 255;
	int g = 0;
	int b = 0;

	for (int i = x*3; i < x*3+width*3; i+=3) {

		fb[i + y*SCREEN_WIDTH*3] = b;
		fb[i+1 + y*SCREEN_WIDTH*3] = g;
		fb[i+2 + y*SCREEN_WIDTH*3] = r;
	}

}

void* memsetlong( void* ptr, int c, size_t n ) {
    int* p = (int*) ptr;

    void* save = ptr;

    while ( n-- ) {
        *p++ = c;
    }

    return save;
}

static int val = 0;

void* videomain(void* instance) {

	val += 0x11301130;

	//val = ~val;

	//printf("inst:%d\r",inst);

	// fill the whole framebuffer
	memsetlong(fb, val, FRAMEBUFFER_SIZE / 4 );
}

extern "C" int task_main()
{
	val = 0;
	printf("Hello from Framebuffer Task!\r\n ");

	unint4 fb_address = 0;

	int error = shm_map("/dev/fb0",&fb_address,&fb_size);
	if (error != 0) {
		printf("Can not open Framebuffer 'fb0'. Error: %d\r",error);
	}

	printf("Mapped Framebuffer at: 0x%x, size: %d\r",fb_address,fb_size);

	// if we did not acquire the framebuffer or it was not mapped
	// we will get data abort errors.
	// this is intended to test the error handling of orcos

	// write to backbuffer
	fb = (char*) (fb_address);


	thread_attr_t attr;
	attr.deadline = 300000;
	attr.period = 300000;
	attr.executionTime = 0;
	attr.phase = 0;
	attr.stack_size = 2048;

	int id;

	thread_create(&id,&attr,&videomain,0);
	thread_run(id);


	while(1) { sleep(5000); }

/*	int x = 0;
	int y = 0;

	while(1)
	{
		sleep(1000);
		drawRect(x,y,40);
		x+= 20;
		y += 20;

	}*/

	thread_exit();
}
