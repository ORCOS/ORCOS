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


extern "C"int socket(int domain, int type, int protocol)
{
    return syscall(cSocketSyscallId,domain,type,protocol);
}

extern "C"int connect(int socket, const sockaddr *toaddress)
{
    return syscall(cConnectSyscallId,socket,toaddress);
}

extern "C"int listen(int socket)
{
    return syscall(cListenSyscallId,socket);
}

extern "C"int bind(int socket, const sockaddr *address)
{
    return syscall(cBindSyscallId,socket,address);
}

extern "C"int4 sendto(int socket, const void *buffer, size_t length, const sockaddr *dest_addr)
{
    return syscall(cSendtoSyscallId,socket,buffer,length,dest_addr);
}

extern "C"size_t recv(int socket,char* data,int len, int flags)
{
    return syscall(cRecvFromSyscallId,socket,data,len,flags,0);
}

extern "C"size_t recvfrom(int socket,char* data,int len,int flags, sockaddr* sender)
{
    return syscall(cRecvFromSyscallId,socket,data,len,flags,sender);
}

