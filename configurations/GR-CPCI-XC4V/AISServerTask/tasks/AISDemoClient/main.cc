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

decoder_attr args[10]  __attribute__ ((aligned (8)));

extern "C" int main(int i,char** b)
{
	int block_id = ((map_thread_data*) args[i].msgptr)->block_id;
//	printf("TurboDecoder: Decoding job %d, block_id=%d.\n",(int) i, block_id);
	printf("TurboDecoder: Decoding job");

	map_decoder_t(args[i].msgptr);

	printf("TurboDecoder: Done\n");
	// send reply
	sendto(mysock,&block_id,sizeof(int),&args[i].sender);

}
