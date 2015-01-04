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

#include <Leon3InterruptHandler.hh>
#include <memtools.hh>
#include <SCLConfig.hh>
#include <assemblerFunctions.hh>


extern "C" Leon3InterruptHandler* theLeon3InterruptHandler;
extern "C" void trap_timer (void);
extern "C" void trap_irq (void);
extern "C" void trap_alignment (void);
extern "C" void trap_syscall (void);




Leon3InterruptHandler::Leon3InterruptHandler() {
    // set ourself to the IRQHandler
    theLeon3InterruptHandler = this;

#ifdef MP_CONFIG
    int offset;
    GET_CPU_INDEX(offset);
    offset *= NODE_OFFSET;

    //memcpy((void*) (0x40000070 + offset), (void*) &trap_alignment,(size_t) 0x10);

    // uart irq
    memcpy((void*) (0x40000120 + offset), (void*) &trap_irq,(size_t) 0x10);

    // shared memory
    memcpy((void*) (0x40000140 + offset), (void*) &trap_irq,(size_t) 0x10);

    // greth interrupt
    memcpy((void*) (0x400001e0 + offset), (void*) &trap_irq,(size_t) 0x10);

    // timer traps
    memcpy((void*) (0x40000180 + offset), (void*) &trap_timer,(size_t) 0x10);
    memcpy((void*) (0x40000190 + offset), (void*) &trap_timer,(size_t) 0x10);
    memcpy((void*) (0x400001A0 + offset), (void*) &trap_timer,(size_t) 0x10);
    memcpy((void*) (0x400001B0 + offset), (void*) &trap_timer,(size_t) 0x10);
    memcpy((void*) (0x400001C0 + offset), (void*) &trap_timer,(size_t) 0x10);

    // syscall trap
    memcpy((void*) (0x40000800 + offset), (void*) &trap_syscall,(size_t) 0x10);
#else
    //memcpy((void*) 0x40000070, (void*) &trap_alignment,(size_t) 0x10);

    // uart irq
    memcpy((void*) 0x40000120, (void*) &trap_irq,(size_t) 0x10);

    // shared memory
    memcpy((void*) 0x40000140, (void*) &trap_irq,(size_t) 0x10);

    // greth interrupt
    memcpy((void*) 0x400001e0, (void*) &trap_irq,(size_t) 0x10);

    // timer traps
    memcpy((void*) 0x40000180, (void*) &trap_timer,(size_t) 0x10);
    memcpy((void*) 0x40000190, (void*) &trap_timer,(size_t) 0x10);
    memcpy((void*) 0x400001A0, (void*) &trap_timer,(size_t) 0x10);
    memcpy((void*) 0x400001C0, (void*) &trap_timer,(size_t) 0x10);

    // syscall trap
    memcpy((void*) 0x40000800, (void*) &trap_syscall,(size_t) 0x10);
#endif
}

Leon3InterruptHandler::~Leon3InterruptHandler() {
}
