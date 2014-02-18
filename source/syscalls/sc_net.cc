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

#if ENABLE_NETWORKING

/*******************************************************************
 *				SOCKET Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_socketCfd
int socketSyscall( int4 int_sp ) {
    unint2 domain;
    SOCK_TYPE type;
    unint2 protocol;

    SYSCALLGETPARAMS3(int_sp,domain,type,protocol);


    // create new Socket
    Socket* s = new Socket( domain, type, protocol );
    pCurrentRunningTask->aquiredResources.addTail( (DatabaseItem*) s );

    LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: Socket created with id %d",s->getId()));

    // return the id of this new resource (socket)
    return (s->getId());
}
#endif



/*******************************************************************
 *				CONNECT Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_connectCfd
int connectSyscall(int4 int_sp) {
	 ResourceIdT socketid;
	 sockaddr* addr;
	 int retval;

	 SYSCALLGETPARAMS2(int_sp,socketid,addr);

	 VALIDATE_IN_PROCESS(addr);

	 Resource* res;
	 res = pCurrentRunningTask->getOwnedResourceById( socketid );
	 if ( res != 0 ) {

		LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: connect valid"));

		// resource valid and owned
		// check if resource is a socket
		if ( res->getType() == cSocket ) {
			retval = ( (Socket*) res )->connect(pCurrentRunningThread, addr );
		}
		else
		retval = cError;
	 }
	 // maybe resource not owned or resource doesnt exist

	 else
	    retval = cError;

	 return (retval);
}
#endif



/*******************************************************************
 *				LISTEN Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_listenCfd
int listenSyscall(int4 int_sp) {
	ResourceIdT socketid;
	int retval;

	SYSCALLGETPARAMS1(int_sp,socketid);

	Resource* res;
	res = pCurrentRunningTask->getOwnedResourceById( socketid );
	if ( res != 0 ) {

		LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: listen "));

		// resource valid and owned
		// check if resource is a socket
		if ( res->getType() == cSocket ) {
			retval = ( (Socket*) res )->listen(pCurrentRunningThread);
		}
		else {
			retval = cError;
			LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: listen invalid.. Resource is not a Socket. "));
		}

	}
	// maybe resource not owned or resource doesn't exist
	else {
		retval = cError;
		LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: listen invalid.. Resource not owned.. "));
	}

	return (retval);
}
#endif



/*******************************************************************
 *				BIND Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_bindCfd
int bindSyscall( int4 int_sp ) {
	ResourceIdT socketid;
    sockaddr* addr;
    int retval;

    SYSCALLGETPARAMS2(int_sp,socketid,addr);

    VALIDATE_IN_PROCESS(addr);

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( socketid );
    if ( res != 0 ) {

        LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: bind valid"));

        // resource valid and owned
        // check if resource is a socket
        if ( res->getType() == cSocket ) {
            retval = ( (Socket*) res )->bind( addr );

        }
        else {
			retval = cError;
			LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: bind invalid.. Resource is not a Socket. "));
		}

    }
    // maybe resource not owned or resource doesnt exist

    else{
		retval = cError;
		LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: bind invalid.. Resource not owned.. "));
	}
    return (retval);
}
#endif



/*******************************************************************
 *				SENDTO Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_sendtoCfd
int sendtoSyscall( int4 int_sp ) {
	ResourceIdT socket;
    const void* buffer;
    size_t length;
    const sockaddr *dest_addr;
    int retval;

    SYSCALLGETPARAMS4(int_sp,socket,buffer,length,dest_addr);

    VALIDATE_IN_PROCESS(buffer);
    VALIDATE_IN_PROCESS((unint4) buffer + length);
    if (dest_addr != 0) {
    	VALIDATE_IN_PROCESS(dest_addr);
    }

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( socket );
    if ( res != 0 ) {

        LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: sendto valid"));

        // resource valid and owned
        // check if resource is a socket
        if ( res->getType() == cSocket ) {
            retval = ( (Socket*) res )->sendto( buffer, (unint2) length, dest_addr );

        }
        else
        retval = cError;

    }
    // maybe resource not owned or resource doesnt exist

    else {
        LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: sendto invalid! Resource %d not owned",socket));
        retval = cError;
    }

    return (retval);
}
#endif



/*******************************************************************
 *				RECV Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_recvCfd
int recvSyscall( int4 int_sp) {

	ResourceIdT socketid;
    char* data_addr;
    size_t data_len;
    int flags;
    int retval;
    sockaddr* sender;

    SYSCALLGETPARAMS5(int_sp,socketid,data_addr,data_len,flags,sender);

    VALIDATE_IN_PROCESS(data_addr);
    //VALIDATE_IN_PROCESS(sender);

    LOG(SYSCALLS,DEBUG,(SYSCALLS,DEBUG,"Syscall: recv: socketid %d, msg_addr: 0x%x, flags: %x, sockaddr_ptr: 0x%x",socketid,data_addr,flags,sender));

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( socketid );
    if ( res != 0 ) {

        LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: recv valid"));

        // resource valid and owned
        // check if resource is a socket
        if ( res->getType() == cSocket ) {
            retval = ( (Socket*) res )->recvfrom( pCurrentRunningThread, data_addr,data_len, flags, sender );
        }
        else
        retval = cError;

    }
    // maybe resource not owned or resource doesnt exist
    else
    	retval = cError;

    return (retval);
}
#endif


#endif // networking enabled


