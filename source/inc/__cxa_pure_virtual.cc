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

#include "kernel/Kernel.hh"

extern "C" Kernel* theOS;

void* __stack_chk_guard = (void*) 0xdeadbeaf;

extern "C" void __stack_chk_fail() {
    while (1)
        ;
}

extern "C" void __cxa_pure_virtual() {

    while (1)
        ;
}
