/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

#include "lwip/opt.h"
#include "ipv6/ip6_addr.h"
#include "inet.h"
#include "stringtools.hh"
#include "memtools.hh"

u8_t ip6_addr_netcmp(struct ip6_addr *addr1, struct ip6_addr *addr2, struct ip6_addr *mask) {
    return ((addr1->addr[0] & mask->addr[0]) == (addr2->addr[0] & mask->addr[0])
            && (addr1->addr[1] & mask->addr[1])
            == (addr2->addr[1] & mask->addr[1])
            && (addr1->addr[2] & mask->addr[2])
            == (addr2->addr[2] & mask->addr[2])
            && (addr1->addr[3] & mask->addr[3])
            == (addr2->addr[3] & mask->addr[3]));

}

u8_t ip6_addr_cmp(struct ip6_addr *addr1, struct ip6_addr *addr2) {
    return (addr1->addr[0] == addr2->addr[0] && addr1->addr[1] == addr2->addr[1]
            && addr1->addr[2] == addr2->addr[2]
            && addr1->addr[3] == addr2->addr[3]);
}

void ip6_addr_set(struct ip6_addr *dest, struct ip6_addr *src) {
    SMEMCPY(dest, src, sizeof(struct ip6_addr));
}

u8_t ip6_addr_isany(struct ip6_addr *addr) {
    if (addr == 0)
        return 1;
    return ((addr->addr[0] | addr->addr[1] | addr->addr[2] | addr->addr[3]) == 0);
}
