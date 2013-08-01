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

#include "Leon3_IRQMP.hh"
#include <assembler.h>
#include <assemblerFunctions.hh>


Leon3_IRQMP::Leon3_IRQMP() {

	// get cpu index
	GET_CPU_INDEX(cpuOffset);

	// register offset for cpu n = 4*n
	cpuOffset *= 4;

	// clear all pending interrupts
	OUTW(LEON3_IRQMP_BASE + LEON3_IRQMP_CLEAR_OFFSET, 0);
	OUTW(LEON3_IRQMP_BASE + LEON3_IRQMP_BROADCAST, 1 << 14);
}

Leon3_IRQMP::~Leon3_IRQMP() {
}



