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

#include "FailureMonitor.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;
extern ThreadCfdCl* pCurrentRunningThread;
extern Task* pCurrentRunningTask;

FailureMonitor::FailureMonitor(  )
{
    lasterror = *( (volatile unsigned int*) FAIL_ADDR );
	failure_threshold = 21000;
}

void FailureMonitor::callbackFunc( void* param ) {
   // check the failure hardware register again
	volatile unsigned int curerror = *((unsigned int*)FAIL_ADDR);
	volatile unsigned int diff = curerror - lasterror;
	lasterror = curerror;

	printf("\nFM: diff %d, total errors %d \n\n",diff,lasterror);

	if (diff > failure_threshold)
	{
		// handle the error by doing some fancy stuff like migrating!
		// for now we just throw a message!
		printf("FM: Failure counter reached threshold! %d > %d\n",diff,failure_threshold);
	}
}
