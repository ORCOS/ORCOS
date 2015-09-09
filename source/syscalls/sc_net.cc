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

#include "syscalls.hh"
#include Kernel_Thread_hh
#include "assemblerFunctions.hh"
#include "lwip/dns.h"

#if ENABLE_NETWORKING

/*******************************************************************
 *                SOCKET Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_socketCfd
/*****************************************************************************
 * Method: sc_socket(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
int sc_socket(intptr_t int_sp) {
    unint2 domain;
    SOCK_TYPE type;
    unint2 protocol;

    SYSCALLGETPARAMS3(int_sp, domain, type, protocol);

    /* create new Socket */
    Socket* s = new Socket(domain, type, protocol);
    if (s->isValid()) {
        pCurrentRunningTask->aquiredResources.addTail(s);
        LOG(SYSCALLS, TRACE, "Syscall: Socket created with id %d", s->getId());
        /* return the id of this new resource (socket) */
        return (s->getId());
    } else {
        delete (s);
        return (cError );
    }
}
#endif

/*******************************************************************
 *                CONNECT Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_connectCfd
/*****************************************************************************
 * Method: sc_connect(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
int sc_connect(intptr_t int_sp) {
    ResourceIdT socketid;
    sockaddr* addr;
    int timeout_ms;
    int retval;

    SYSCALLGETPARAMS3(int_sp, socketid, addr, timeout_ms);
    VALIDATE_IN_PROCESS(addr);

    if (timeout_ms <= 0) {
        return (cInvalidArgument);
    }

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById(socketid);
    if (res != 0) {
        LOG(SYSCALLS, TRACE, "Syscall: connect valid");

        if (res->getType() == cSocket) {
            Socket* sock = static_cast<Socket*>(res);
            retval = sock->connect(pCurrentRunningThread, addr, timeout_ms);
        } else {
            retval = cWrongResourceType;
        }
    } else {
        retval = cResourceNotOwned;
    }

    return (retval);
}
#endif

/*******************************************************************
 *                LISTEN Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_listenCfd
/*****************************************************************************
 * Method: sc_listen(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
int sc_listen(intptr_t int_sp) {
    ResourceIdT socketid;
    unint4 backlog_size;
    int retval;

    SYSCALLGETPARAMS2(int_sp, socketid, backlog_size);
    if (backlog_size > 50 || backlog_size == 0) {
        LOG(SYSCALLS, ERROR, "Syscall: listen. Invalid backlog size %u.", backlog_size);
        return (cInvalidArgument);
    }

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById(socketid);
    if (res != 0) {
        LOG(SYSCALLS, TRACE, "Syscall: listen ");

        if (res->getType() == cSocket) {
            Socket* sock = static_cast<Socket*>(res);
            retval = sock->listen(pCurrentRunningThread, backlog_size);
        } else {
            retval = cWrongResourceType;
            LOG(SYSCALLS, ERROR, "Syscall: listen invalid.. Resource is not a Socket. ");
        }
    } else {
        retval = cResourceNotOwned;
        LOG(SYSCALLS, ERROR, "Syscall: listen invalid.. Resource not owned.. ");
    }

    return (retval);
}
#endif

/*******************************************************************
 *                BIND Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_bindCfd
/*****************************************************************************
 * Method: sc_bind(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
int sc_bind(intptr_t int_sp) {
    ResourceIdT socketid;
    sockaddr* addr;
    int retval;

    SYSCALLGETPARAMS2(int_sp, socketid, addr);
    VALIDATE_IN_PROCESS(addr);

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById(socketid);
    if (res != 0) {
        LOG(SYSCALLS, TRACE, "Syscall: bind valid");

        /* resource valid and owned
         * check if resource is a socket */
        if (res->getType() == cSocket) {
            Socket* sock = static_cast<Socket*>(res);
            retval = sock->bind(addr);

        } else {
            retval = cWrongResourceType;
            LOG(SYSCALLS, ERROR, "Syscall: bind invalid.. Resource is not a Socket. ");
        }

    } else {
        retval = cResourceNotOwned;
        LOG(SYSCALLS, ERROR, "Syscall: bind invalid.. Resource not owned.. ");
    }
    return (retval);
}
#endif

/*******************************************************************
 *                SENDTO Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_sendtoCfd
/*****************************************************************************
 * Method: sc_sendto(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
int sc_sendto(intptr_t int_sp) {
    ResourceIdT socket;
    const void* buffer;
    size_t length;
    const sockaddr *dest_addr;
    int retval;

    SYSCALLGETPARAMS4(int_sp, socket, buffer, length, dest_addr);

    VALIDATE_IN_PROCESS(buffer);
    VALIDATE_IN_PROCESS((unint4) buffer + length);
    if (dest_addr != 0) {
        VALIDATE_IN_PROCESS(dest_addr);
    }

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById(socket);
    if (res != 0) {
        LOG(SYSCALLS, TRACE, "Syscall: sendto valid");

        /* resource valid and owned
         check if resource is a socket */
        if (res->getType() == cSocket) {
            Socket* sock = static_cast<Socket*>(res);
            retval = sock->sendto(buffer, (unint2) length, dest_addr);
        } else {
            retval = cWrongResourceType;
        }
    } else {
        LOG(SYSCALLS, ERROR, "Syscall: sendto invalid! Resource %d not owned", socket);
        retval = cResourceNotOwned;
    }

    return (retval);
}
#endif

/*******************************************************************
 *                RECV Syscall
 *******************************************************************/

#ifdef HAS_SyscallManager_recvCfd
/*****************************************************************************
 * Method: sc_recv(intptr_t sp_int)
 *
 * @description
 *
 * @params
 *  sp_int:     The stack pointer at time of system call instruction execution
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
int sc_recv(intptr_t int_sp) {
    ResourceIdT socketid;
    char* data_addr;
    size_t data_len;
    int flags;
    int retval;
    sockaddr* sender;
    unint4 timeout;

    SYSCALLGETPARAMS6(int_sp, socketid, data_addr, data_len, flags, sender, timeout);

    VALIDATE_IN_PROCESS(data_addr);
    if (sender != 0)
        VALIDATE_IN_PROCESS(sender);

    LOG(SYSCALLS, DEBUG, "Syscall: recv: socketid %d, msg_addr: 0x%x, flags: %x, sockaddr_ptr: 0x%x", socketid, data_addr, flags, sender);

    Resource* res;
    res = pCurrentRunningTask->getOwnedResourceById(socketid);
    if (res != 0) {
        LOG(SYSCALLS, TRACE, "Syscall: recv valid");

        /* resource valid and owned
         check if resource is a socket */
        if (res->getType() == cSocket) {
            Socket* sock = static_cast<Socket*>(res);
            retval = sock->recvfrom(pCurrentRunningThread, data_addr, data_len, flags, sender, timeout);
        } else {
            retval = cWrongResourceType;
        }
    } else {
        /* maybe resource not owned or resource doesnt exist */
        retval = cResourceNotOwned;
    }

    return (retval);
}
#endif

#ifdef HAS_SyscallManager_gethostbynameCfd
void dns_lookup_found_callback(const char *name, ip_addr_t *ipaddr, void *callback_arg) {
    if (callback_arg != 0) {
        Thread* p = (Thread*) callback_arg;
        p->unblock();
    }
};

/*****************************************************************************
 * Method: sc_gethostbyname(intptr_t int_sp)
 *
 * @description
 * Performs a host name lookup if DNS is supported.
 *******************************************************************************/
int sc_gethostbyname(intptr_t int_sp) {
#ifndef LWIP_DNS
    return (cNotImplemented);
#else
   char*  host_name;
   int*   host_ip4addr;
   SYSCALLGETPARAMS2(int_sp, host_name, host_ip4addr);
   VALIDATE_IN_PROCESS(host_name);
   VALIDATE_IN_PROCESS(host_ip4addr);

   ip_addr_t res_addr;
   err_t result = dns_gethostbyname(host_name, &res_addr, dns_lookup_found_callback, pCurrentRunningThread);
   if (result == ERR_ARG) {
       LOG(SYSCALLS, WARN, "Syscall: dns_gethostbyname returned error %d", result);
       *host_ip4addr = 0;
       return (cError);
   }
   if (result == ERR_INPROGRESS) {
       /* lookup was not found in cache. wait for
        * lookup for at most 5 seconds */
       pCurrentRunningThread->block(5000 ms);
       if (pCurrentRunningThread->blockTimeout == 0) {
           LOG(SYSCALLS, ERROR, "Syscall: gethostbyname lookup for %s timed out.", host_name);
           return (cError);
       }
   }
   *host_ip4addr = res_addr.addr.ip4addr.addr;
   return (cOk);
#endif
}
#endif


#else
/*****************************************************************************
 * Method: sc_recv(intptr_t sp_int)
 *
 * @description
 *  Dummy if networking is disabled.
 *******************************************************************************/
#ifdef HAS_SyscallManager_recvCfd
int sc_recv(intptr_t int_sp) {
 return (cNotImplemented);
}
#endif

/*****************************************************************************
 * Method: sc_sendto(intptr_t int_sp)
 *
 * @description
 *  Dummy if networking is disabled.
 *******************************************************************************/
#ifdef HAS_SyscallManager_sendtoCfd
int sc_sendto(intptr_t int_sp) {
 return (cNotImplemented);
}
#endif

/*****************************************************************************
 * Method: sc_bind(intptr_t int_sp)
 *
 * @description
 *  Dummy if networking is disabled.
 *******************************************************************************/
#ifdef HAS_SyscallManager_bindCfd
int sc_bind(intptr_t int_sp) {
 return (cNotImplemented);
}
#endif

/*****************************************************************************
 * Method: sc_socket(intptr_t int_sp)
 *
 * @description
 *  Dummy if networking is disabled.
 *******************************************************************************/
#ifdef HAS_SyscallManager_socketCfd
int sc_socket(intptr_t int_sp) {
 return (cNotImplemented);
}
#endif

/*****************************************************************************
 * Method: sc_connect(intptr_t int_sp)
 *
 * @description
 *  Dummy if networking is disabled.
 *******************************************************************************/
#ifdef HAS_SyscallManager_connectCfd
int sc_connect(intptr_t int_sp) {
 return (cNotImplemented);
}
#endif

/*****************************************************************************
 * Method: sc_listen(intptr_t int_sp)
 *
 * @description
 *  Dummy if networking is disabled.
 *******************************************************************************/
#ifdef HAS_SyscallManager_listenCfd
int sc_listen(intptr_t int_sp) {
 return (cNotImplemented);
}
#endif

/*****************************************************************************
 * Method: sc_gethostbyname(intptr_t int_sp)
 *
 * @description
 *  Dummy if networking is disabled.
 *******************************************************************************/
#ifdef HAS_SyscallManager_gethostbynameCfd
int sc_gethostbyname(intptr_t int_sp) {
 return (cNotImplemented);
}
#endif

#endif // networking enabled

