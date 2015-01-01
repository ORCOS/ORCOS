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

#include "CommDeviceDriver.hh"
#include <kernel/Kernel.hh>

/* user library defines */
#include "inc/defines.h"
#include "syscalls/syscalls.hh"

extern Kernel* theOS;

//int2 CommDeviceDriver::globalCommDeviceIdCounter;

//TODO: create configurable burst mode comm device driver using a
// period/timed functional call to send packets
// since this takes alot more memory this has to be configureable!

CommDeviceDriver::CommDeviceDriver(const char* p_name) :
        CharacterDevice(cCommDevice, false, p_name) {
}

CommDeviceDriver::~CommDeviceDriver() {
}

ErrorT CommDeviceDriver::ioctl(int request, void* args) {

    switch (request) {
    case cNETIF_SETIPV4: {
        struct ip4_addr ipAddr;
        ipAddr.addr = (u32_t) args;
        netif_set_ipaddr(&st_netif, &ipAddr);
        break;
    }
    case cNETIF_SETGWIPV4: {
        struct ip4_addr ipAddr;
        ipAddr.addr = (u32_t) args;
        netif_set_gw(&st_netif, &ipAddr);
        break;
    }
    case cNETIF_SETNETMASK: {
        struct ip4_addr ipAddr;
        ipAddr.addr = (u32_t) args;
        netif_set_netmask(&st_netif, &ipAddr);
        break;
    }
    case cNETIF_SET_DOWN: {
        netif_set_down(&st_netif);
        break;
    }
    case cNETIF_SET_UP: {
        netif_set_up(&st_netif);
        break;
    }
    case cNETIF_GET_STATS: {
        VALIDATE_IN_PROCESS(args);
        netif_stat_t* stats = reinterpret_cast<netif_stat_t*>(args);
        memset(stats,0,sizeof(netif_stat_t));
        stats->flags        = st_netif.flags;
        stats->errors       = st_netif.txerrors;
        stats->ipv4addr     = ntohl(st_netif.ip4_addr.addr);
        stats->gwipv4addr   = ntohl(st_netif.ip4_gw.addr);
        stats->ipv4netmask  = ntohl(st_netif.ip4_netmask.addr);
        stats->mtu          = st_netif.mtu;
        stats->rxbytes      = st_netif.rxbytes;
        stats->rxpackets    = st_netif.rxpackets;
        stats->txpackets    = st_netif.txpackets;
        stats->txbytes      = st_netif.txbytes;
        memcpy(stats->hwaddr,st_netif.hwaddr,st_netif.hwaddr_len);
        memcpy(stats->ipv6addr,st_netif.ip6_addr.addr,16);
        break;
    }
    default:
        return (cInvalidArgument);
    }

    return (cOk );
}
