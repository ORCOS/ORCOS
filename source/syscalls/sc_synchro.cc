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

 Author: dbaldin
 */

#include "handle_syscalls.hh"
#include Kernel_Thread_hh
#include "assembler.h"


/*******************************************************************
 *				SIGNAL_WAIT Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_signal_waitCfd
int signal_wait( int4 int_sp ) {
    void* sig;
    bool memAddrAsSig;
    SYSCALLGETPARAMS2( int_sp, sig, memAddrAsSig );

#ifdef HAS_Board_HatLayerCfd
    // if we want to use a Memory Address as the signal, we have to get the physical Address
    if (memAddrAsSig) {
        sig = (void*) theOS->getHatLayer()->getPhysicalAddress( sig );
    }
#endif //HAS_Board_HatLayerCfd

    pCurrentRunningThread->sigwait( sig );

    return cOk;
}
#endif


/*******************************************************************
 *				SIGNAL_SIGNAL Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_signal_signalCfd
int signal_signal( int4 int_sp ) {
    void* sig;
    bool memAddrAsSig;
    SYSCALLGETPARAMS2( int_sp, sig, memAddrAsSig );

#ifdef HAS_Board_HatLayerCfd
    // if we want to use a Memory Address as the signal, we have to get the physical Address
    if (memAddrAsSig) {
        sig = (void*) theOS->getHatLayer()->getPhysicalAddress( sig );
    }
#endif //HAS_Board_HatLayerCfd

    theOS->getCPUDispatcher()->signal( sig, cOk );

    return cOk;
}
#endif
