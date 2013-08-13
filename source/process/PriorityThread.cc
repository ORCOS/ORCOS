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

#include "process/PriorityThread.hh"
#include "kernel/Kernel.hh"
#include "assembler.h"

extern Kernel* theOS;
extern ThreadCfdCl* pCurrentRunningThread;

PriorityThread::PriorityThread( void* startRoutinePointer, void* exitRoutinePointer, Task* owner,
        MemoryManagerCfdCl* memManager, unint4 stack_size, void* prioThreadAttributes, bool newThread ) :
    Thread( startRoutinePointer, exitRoutinePointer, owner, memManager, stack_size, prioThreadAttributes, newThread )

{
    if ( prioThreadAttributes != 0 ) {
    	taskTable* attr = static_cast< taskTable* > ( prioThreadAttributes );
        // get phase and convert from µs to cycles
        this->phase 			= attr->phase * (CLOCK_RATE / 1000000);
        this->initialPriority 	= attr->initialPriority;
        this->effectivePriority = attr->initialPriority;
    }
    else {
        this->phase = 0;
        this->initialPriority	= cDefaultPriority;
        this->effectivePriority = cDefaultPriority;
    }

}

