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

#include "AISMemManager.hh"
#include <Alignment.hh>

// areas must be placed inside the first MB of 0x400-----
// aligned on a 4KB page basis
/*protected_mem_area pmem_areas[16] =
{
	{0x40040000,0x40040000}, // mode 0 0 KB
	{0x40040000,0x40040000}, // mode 1 0 KB
	{0x40040000,0x40040000}, // mode 2 0 KB
	{0x40040000,0x40080000}, // mode 3 256 KB
	{0x40080000,0x40084000}, // mode 4 16 KB
	{0x40084000,0x40084000}, // mode 5 0 KB
	{0x40084000,0x40084000}, // mode 6 0 KB
	{0x40084000,0x40084000}, // mode 7 0 KB
	{0x40084000,0x40088000}, // mode 8 16 KB
	{0x40088000,0x40088000}, // mode 9 0 KB
	{0x40088000,0x40088000}, // mode 10 0 KB
	{0x40088000,0x40088000}, // mode 11 0 KB
	{0x40088000,0x40088000}, // mode 12 0 KB
	{0x40088000,0x40088000}, // mode 13 0 KB
	{0x40088000,0x40088000}, // mode 14 0 KB
	{0x40088000,0x4008a000}  // mode 15 16 KB
};*/

protected_mem_area pmem_areas[16] =
{
	{0x46000000,0x46000000}, // mode 0 0 KB
	{0x46000000,0x46000000}, // mode 1 0 KB
	{0x46000000,0x46000000}, // mode 2 0 KB
	{0x45000000,0x45FFFFFF}, // mode 3 256 KB
	{0x46000000,0x46FFFFFF}, // mode 4 16 KB
	{0x45000000,0x45000000}, // mode 5 0 KB
	{0x45000000,0x45000000}, // mode 6 0 KB
	{0x45000000,0x45000000}, // mode 7 0 KB
	{0x47000000,0x47FFFFFF}, // mode 8 16 KB
	{0x45000000,0x45000000}, // mode 9 0 KB
	{0x45000000,0x45000000}, // mode 10 0 KB
	{0x45000000,0x45000000}, // mode 11 0 KB
	{0x45000000,0x45000000}, // mode 12 0 KB
	{0x45000000,0x45000000}, // mode 13 0 KB
	{0x45000000,0x45000000}, // mode 14 0 KB
	{0x48000000,0x48FFFFFF}  // mode 15 16 KB
};

void* AISMemManager::allocp( size_t size, unint1 protection ) {

	register unint4 ptr = (unint4) alignCeil((byte*)protmemptr[protection],8);
	if ( ptr + size <= pmem_areas[protection].end)
	{
		//printf("Allocating %d at 0x%x\n",size,ptr);
		protmemptr[protection] = ptr + size;
		return (void*) ptr;
	}
	else
	{
		printf("ERROR: out of memory for protection mode %d\n",protection);
		while (1);
		return 0;
	}

}

void AISMemManager::setHardwareProtection()
{
	printf("Init Hardware Protection Modes:\n\r");

	// set protection for this area
	char* hw_prot = (char*) 0xCC000100;

	// set all subarea protection and failure injection mode to 15
	int i;
	for (i = 0; i<=0xff; i++) *(hw_prot+i) = 15;


	// set failure injection for client turbo decoder text area
	// client node protection mode
	*( (char*)0xCC000042) = 15;

	// map decoder protection mode
	*( (char*)0xCC000043) = 15;


	hw_prot = (char*) 0xCC000000;
	for (i = 0; i<=0xff; i++) *(hw_prot+i) = 15;




	/*for (i = 0; i < 4; i++)
	{
		if ((pmem_areas[i].end - pmem_areas[i].start) > 0 )
		{
			// calculate sub_area indizes
			int startindex = (pmem_areas[i].start - 0x40000000) / 4096;
			int endindex = (pmem_areas[i].end - 0x40000000) / 4096;

			printf("Area: 0x%x - 0x%x Mode: %d\n\r",pmem_areas[i].start,pmem_areas[i].end,i);

			int i2;
			for (i2 = startindex; i2 < endindex; i2++)
			{
				unint4 reg = (unint4) ((unint4)hw_prot + i2);
				printf("Setting Subarea %d (0x%x-0x%x) by register: 0x%x.\n\r",i2,0x40000000 + (i2 * 4096),0x40000000 + ((i2+1) * 4096),reg );
				*((char*)reg) = 2;
			}
		}

	}*/

	*( (char*)0xCC000040) = 15;
	*( (char*)0xCC000041) = 15;

	*( (char*)0xCC000044) = 15;

	*( (char*)0xCC000045) = 2;
	//*( (char*)0xCC000045) = 15;
	//*( (char*)0xCC000045) = 15;

    *( (char*)0xCC000046) = 15;

	*( (char*)0xCC000047) = 2;
	//*( (char*)0xCC000047) = 15;

	*( (char*)0xCC000048) = 2;
	//*( (char*)0xCC000049) = 15;
}

