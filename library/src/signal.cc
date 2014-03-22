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

#include "./types.h"
#include "./defines.h"
#include "./orcos.hh"

int signal_wait( void* sig, bool memAddrAsSig ) {
    return syscall( cSignal_WaitSyscallId, sig, memAddrAsSig );
}

void signal_signal( void* sig,int value, bool memAddrAsSig ) {
    syscall( cSignal_SignalSyscallId, sig, value, memAddrAsSig );
}
