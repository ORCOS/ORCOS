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

#include "inc/types.hh"
#include "inc/error.hh"
#include <sprintf.hh>
#include <kernel/Kernel.hh>

#include Kernel_Thread_hh

extern Kernel* theOS;

/*
 * Kernel wait function. Uses the Board Clock to wait for a given time
 * in milliseconds. This is a blocking wait if we can not be preempted.
 *
 */
void kwait(int milliseconds) {
	/* check if we have a board and clock
	   if not we are probably initializing them */
	if ((theOS == 0) || (theOS->board == 0) || (theOS->getClock() == 0)) {
		/* fallback if the clock is not set */
		volatile unint4 i = 0;  /* volatile so this is not optimized out  */
		while ( i < 100000) i++;
	} else {
		volatile unint8 now = theOS->getClock()->getTimeSinceStartup();
		while (theOS->getClock()->getTimeSinceStartup() < (now + (milliseconds * 33))) {};
	}

}




