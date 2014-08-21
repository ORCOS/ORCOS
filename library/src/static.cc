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


#include "types.h"
#include "defines.h"
#include "orcos.hh"

// must always be linked into the user application
extern "C" void thread_exit(int exitCode)
{
    syscall(cThread_ExitSysCallId, exitCode);
}


extern "C" unint8 getCycles()
{
	unint8 time;
	syscall(cGetTimeSyscallId,&time);
	return time;
}

extern "C" unint4 getTime()
{
    return syscall(cGetDateTimeSyscallId);
}
