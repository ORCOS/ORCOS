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

#include <memtools.hh>
#include <assemblerFunctions.hh>

extern "C" void _wofHandler (void);
extern "C" void _wufHandler (void);
extern "C" void trap_alignment (void);

/**
 * Initializes some critical trap handlers
 */
extern "C" void initTrapHandler()
{
#ifndef MP_CONFIG
	// install window overflow handler
	memcpy((void*) 0x40000050, (void*) &_wofHandler, (size_t)0x10);

	// install window underflow handler
	memcpy((void*) 0x40000060, (void*) &_wufHandler, (size_t)0x10);

	// install alignment handler
	memcpy((void*) 0x40000070, (void*) &trap_alignment, (size_t)0x10);
#else
	int index;
	GET_CPU_INDEX(index);
// install window overflow handler
	memcpy((void*) (0x40000050 + index * NODE_OFFSET), (void*) &_wofHandler, (size_t)0x10);

	// install window underflow handler
	memcpy((void*) (0x40000060 + index * NODE_OFFSET), (void*) &_wufHandler, (size_t)0x10);

	// install alignment handler
	memcpy((void*) (0x40000070 + index * NODE_OFFSET), (void*) &trap_alignment, (size_t)0x10);
#endif
}
