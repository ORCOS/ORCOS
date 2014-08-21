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
#include "assemblerFunctions.hh"


/*******************************************************************
 *				SIGNAL_WAIT Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_signal_waitCfd
int signal_wait( int4 int_sp ) {
    void* sig;
    bool memAddrAsSig;
    SYSCALLGETPARAMS2( int_sp, sig, memAddrAsSig );

    /* signal null (0) is not allowed as it stands for "no" signal */
    if (sig == 0) return (cInvalidArgument);

#ifdef HAS_Board_HatLayerCfd
    // if we want to use a Memory Address as the signal, we have to get the physical Address
    if (memAddrAsSig) {
        sig = (void*) theOS->getHatLayer()->getPhysicalAddress( sig );
    }
#endif //HAS_Board_HatLayerCfd

#ifdef ORCOS_SUPPORT_SIGNALS
    pCurrentRunningThread->sigwait( sig );

    return (cOk);
#else
    return (cError);
#endif
}
#endif


/*******************************************************************
 *				SIGNAL_SIGNAL Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_signal_signalCfd
int signal_signal( int4 int_sp ) {
    void* sig;
    int value;
    bool memAddrAsSig;
    SYSCALLGETPARAMS3( int_sp, sig,value, memAddrAsSig );

    /* signal null (0) is not allowed as it stands for "no" signal */
     if (sig == 0) return (cInvalidArgument);

#ifdef HAS_Board_HatLayerCfd
    // if we want to use a Memory Address as the signal, we have to get the physical Address
    if (memAddrAsSig) {
        sig = (void*) theOS->getHatLayer()->getPhysicalAddress( sig );
    }
#endif //HAS_Board_HatLayerCfd

#ifdef ORCOS_SUPPORT_SIGNALS
    theOS->getDispatcher()->signal( sig, value );

    /* If we have priorities we may need to dispatch .. check this now! */
#ifdef HAS_PRIORITY
        DISABLE_IRQS(status);
        SET_RETURN_VALUE((void*)int_sp,(void*)cOk);
        // we may have unblocked a higher priority thread so we need to reschedule now!
        theOS->getDispatcher()->dispatch(  );
#endif

    return (cOk);
#else
   return cError;
#endif
}
#endif
