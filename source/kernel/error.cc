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

#include ThreadCfd_hh


extern Kernel* theOS;

// handle a software error
// at the moment the error is only reported
ErrorT handleError( ErrorT status ) {
    switch ( status ) {
        case cNotImplemented:
            LOG(KERNEL,ERROR,(KERNEL,ERROR,"Method is not implemented"));
            break;
        case cError:
            LOG(KERNEL,ERROR,(KERNEL,ERROR,"unspecified error occurred"));
            break;
        case cHeapMemoryExhausted:
            LOG(KERNEL,ERROR,(KERNEL,ERROR,"no more heap memory available"));
            break;
    }
    return status;
}

// handle a software error occuring in a single thread
// at the moment the error is only reported
ErrorT handleError( ErrorT status, ThreadCfdCl* curT ) {

#ifdef HAS_Kernel_LoggerCfd
    int curThreadId = curT->getId();
#endif

    switch ( status ) {
        case cNotImplemented:
            LOG(KERNEL,ERROR,(KERNEL,ERROR,"Method is not implemented in Thread: %d",curThreadId));
        case cError:
            LOG(KERNEL,ERROR,(KERNEL,ERROR,"Unspecified error occurred in Thread: %d",curThreadId));
        case cHeapMemoryExhausted:
            LOG(KERNEL,ERROR,(KERNEL,ERROR,"No more heap memory available in Thread: %d",curThreadId));
            break;
    }
    return status;
}
