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
#include ThreadCfd_hh
#include "assembler.h"


/*******************************************************************
 *				SOCKET Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_socketSyscallCfd
int socketSyscall( int4 int_sp ) {
    int domain;
    int type;
    int protocol;
    char* buffer;
    int buffersize;

    SYSCALLGETPARAMS5(int_sp,domain,type,protocol,buffer,buffersize);

    // create new Socket
    Socket* s = new Socket( domain, type, protocol, buffer, buffersize );
    pCurrentRunningTask->aquiredResources.addTail( (DatabaseItem*) s );

    LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: Socket created with id %d",s->getId()));

    // return the id of this new resource (socket)
    return s->getId();
}
#endif



/*******************************************************************
 *				CONNECT Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_connectSyscallCfd
int connectSyscall(int4 int_sp) {
	 int socketid;
	 sockaddr* addr;
	 int retval;

	 SYSCALLGETPARAMS2(int_sp,socketid,addr);

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

	 return retval;
}
#endif



/*******************************************************************
 *				LISTEN Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_listenSyscallCfd
int listenSyscall(int4 int_sp) {
	int socketid;
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
		else
		retval = cError;

	}
	// maybe resource not owned or resource doesn't exist

	else
	retval = cError;

	return retval;
}
#endif



/*******************************************************************
 *				BIND Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_bindSyscallCfd
int bindSyscall( int4 int_sp ) {
    int socketid;
    sockaddr* addr;
    int retval;

    SYSCALLGETPARAMS2(int_sp,socketid,addr);

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( socketid );
    if ( res != 0 ) {

        LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: bind valid"));

        // resource valid and owned
        // check if resource is a socket
        if ( res->getType() == cSocket ) {
            retval = ( (Socket*) res )->bind( addr );

        }
        else
        retval = cError;

    }
    // maybe resource not owned or resource doesnt exist

    else
    retval = cError;

    return retval;
}
#endif



/*******************************************************************
 *				SENDTO Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_sendtoSyscallCfd
int sendtoSyscall( int4 int_sp ) {
    int socket;
    const void* buffer;
    size_t length;
    const sockaddr *dest_addr;
    int retval;

    SYSCALLGETPARAMS4(int_sp,socket,buffer,length,dest_addr);

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( socket );
    if ( res != 0 ) {

        LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: sendto valid"));

        // resource valid and owned
        // check if resource is a socket
        if ( res->getType() == cSocket ) {
            retval = ( (Socket*) res )->sendto( buffer, length, dest_addr );

        }
        else
        retval = cError;

    }
    // maybe resource not owned or resource doesnt exist

    else {
        LOG(SYSCALLS,ERROR,(SYSCALLS,ERROR,"Syscall: sendto invalid! Resource %d not owned",socket));
        retval = cError;
    }

    return retval;
}
#endif



/*******************************************************************
 *				RECV Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_recvSyscallCfd
int recvSyscall( int4 int_sp) {
    int socketid;
    char** addressofptrtomsg;
    int flags;
    int retval;
    sockaddr* sender;

    SYSCALLGETPARAMS4(int_sp,socketid,addressofptrtomsg,flags,sender);

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById( socketid );
    if ( res != 0 ) {

        LOG(SYSCALLS,TRACE,(SYSCALLS,TRACE,"Syscall: recv valid"));

        // resource valid and owned
        // check if resource is a socket
        if ( res->getType() == cSocket ) {
            retval = ( (Socket*) res )->recvfrom( pCurrentRunningThread, addressofptrtomsg, flags, sender );
        }
        else
        retval = cError;

    }
    // maybe resource not owned or resource doesnt exist

    else
    retval = cError;

    return retval;
}
#endif


