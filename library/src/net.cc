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
#include "./string.hh"

extern "C" int socket(int domain, int type, int protocol) {
    return (syscall(cSocketSyscallId, domain, type, protocol));
}

extern "C" int connect(int socket, const sockaddr *toaddress, int timeout) {
    return (syscall(cConnectSyscallId, socket, toaddress, timeout));
}

extern "C" int listen(int socket) {
    return (syscall(cListenSyscallId, socket));
}

extern "C" int bind(int socket, const sockaddr *address) {
    return (syscall(cBindSyscallId, socket, address));
}

extern "C" int4 sendto(int socket, const void *buffer, size_t length, const sockaddr *dest_addr) {
    return (syscall(cSendtoSyscallId, socket, buffer, length, dest_addr));
}

extern "C" size_t recv(int socket, char* data, int len, int flags, unint4 timeout) {
    return (syscall(cRecvFromSyscallId, socket, data, len, flags, 0, timeout));
}

extern "C" size_t recvfrom(int socket, char* data, int len, int flags, sockaddr* sender, unint4 timeout) {
    return (syscall(cRecvFromSyscallId, socket, data, len, flags, sender, timeout));
}

/*%
 * WARNING: Don't even consider trying to compile this on a system where
 * sizeof(int) < 4. sizeof(int) > 4 is fine; all the world's not a VAX.
 */

static char *inet_ntop4(const char *src, char *dst, size_t size);
static char *inet_ntop6(const char *src, char *dst, size_t size);

static char   host_name[40];
static char   host_address[20];
static char*  host_addresses[2];
static struct hostent lookup_host;
static int    host_ip4addr;

extern "C" struct hostent* gethostbyname(char* name) {
    /* todo we might actually port the lwip dns.c file to user space as it only uses
     * udp transfers! we can than safely use the userspace implementation.. */
    strncpy(host_name, name, 40);
    lookup_host.h_name      = host_name;
    lookup_host.h_length    = 1;
    lookup_host.h_addrtype  = AF_INET;
    lookup_host.h_aliases   = 0;
    lookup_host.h_addr_list = (char**) host_addresses;
    host_addresses[0] = host_address;
    host_addresses[1] = 0;

    int error = syscall(cGetHostByNameSyscallId, name, &host_ip4addr);
    if (error == cOk) {
        inet_ntop4((char*)&host_ip4addr, lookup_host.h_addr_list[0], 20);
    } else {
        lookup_host.h_length = 0;
    }

    return (&lookup_host);
}




/* char *
 * inet_ntop(af, src, dst, size)
 * convert a network format address to presentation format.
 * return:
 * pointer to presentation format address (`dst'), or NULL (see errno).
 * author:
 * Paul Vixie, 1996.
 */
extern "C" char* inet_ntop(int af, const char *src, char *dst, size_t size) {
    switch (af) {
    case AF_INET:
        return (inet_ntop4(src, dst, size));
    case AF_INET6:
        return (inet_ntop6(src, dst, size));
    default:
        return (NULL);
    }
    /* NOTREACHED */
}

/* const char *inet_ntop4(src, dst, size)
 *
 * format an IPv4 address
 * return:
 *    `dst' (as a const)
 *
 * notes:
 * (1) uses no statics
 * (2) takes a u_char* not an in_addr as input
 * author:
 * Paul Vixie, 1996.
 */
static char *
inet_ntop4(const char *src, char *dst, size_t size) {
    static const char fmt[] = "%u.%u.%u.%u";
    char tmp[sizeof "255.255.255.255"];
    int l;

    l = sprintf(tmp, fmt, src[0], src[1], src[2], src[3]);
    if (l <= 0 || (size_t) l >= size) {
        return (NULL);
    }
    strncpy(dst, tmp, size);
    return (dst);
}

/* const char *inet_ntop6(src, dst, size)
 *
 * convert IPv6 binary address into presentation (printable) format
 *
 * author:
 * Paul Vixie, 1996.
 */
static char *
inet_ntop6(const char *src, char *dst, size_t size) {
    /*
     * Note that int32_t and int16_t need only be "at least" large enough
     * to contain a value of the specified size. On some systems, like
     * Crays, there is no such thing as an integer variable with 16 bits.
     * Keep this in mind if you think this function should have been coded
     * to use pointer overlays. All the world's not a VAX.
     */
    char tmp[sizeof "ffff:ffff:ffff:ffff:ffff:ffff:255.255.255.255"], *tp;
    struct {
        int base, len;
    } best, cur;
    #define NS_IN6ADDRSZ 16
    #define NS_INT16SZ 2
    unsigned int words[NS_IN6ADDRSZ / NS_INT16SZ];
    int i;

    /*
     * Preprocess:
     * Copy the input (bytewise) array into a wordwise array.
     * Find the longest run of 0x00's in src[] for :: shorthanding.
     */
    memset(words, '\0', sizeof words);
    for (i = 0; i < NS_IN6ADDRSZ; i++)
        words[i / 2] |= (src[i] << ((1 - (i % 2)) << 3));
    best.base = -1;
    best.len = 0;
    cur.base = -1;
    cur.len = 0;
    for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
        if (words[i] == 0) {
            if (cur.base == -1)
                cur.base = i, cur.len = 1;
            else
                cur.len++;
        } else {
            if (cur.base != -1) {
                if (best.base == -1 || cur.len > best.len)
                    best = cur;
                cur.base = -1;
            }
        }
    }
    if (cur.base != -1) {
        if (best.base == -1 || cur.len > best.len)
            best = cur;
    }
    if (best.base != -1 && best.len < 2)
        best.base = -1;

    /*
     * Format the result.
     */
    tp = tmp;
    for (i = 0; i < (NS_IN6ADDRSZ / NS_INT16SZ); i++) {
        /* Are we inside the best run of 0x00's? */
        if (best.base != -1 && i >= best.base && i < (best.base + best.len)) {
            if (i == best.base)
                *tp++ = ':';
            continue;
        }
        /* Are we following an initial run of 0x00s or any real hex? */
        if (i != 0)
            *tp++ = ':';
        /* Is this address an encapsulated IPv4? */
        if (i == 6 && best.base == 0 && (best.len == 6 || (best.len == 7 && words[7] != 0x0001) || (best.len == 5 && words[5] == 0xffff))) {
            if (!inet_ntop4(src + 12, tp, sizeof tmp - (tp - tmp)))
                return (NULL);
            tp += strlen(tp);
            break;
        }
        tp += sprintf(tp, "%x", words[i]);
    }
    /* Was it a trailing run of 0x00's? */
    if (best.base != -1 && (best.base + best.len) == (NS_IN6ADDRSZ / NS_INT16SZ))
        *tp++ = ':';
    *tp++ = '\0';

    /*
     * Check for overflow, copy, and we're done.
     */
    if ((size_t)(tp - tmp) > size) {
        return (NULL);
    }
    strcpy(dst, tmp);
    return (dst);
}

