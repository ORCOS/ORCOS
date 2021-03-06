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
#ifndef __LWIP_IP6_ADDR_H__
#define __LWIP_IP6_ADDR_H__

#include "lwip/opt.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define IP6_ADDR_ANY 0

    typedef enum
    {
        IP6_ADDR_STATE_INVALID, IP6_ADDR_STATE_TENTATIVE, IP6_ADDR_STATE_VALID
    }T_IP6_ADDR_STATE;

#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/bpstruct.h"
#endif
    PACK_STRUCT_BEGIN
    struct ip6_addr
    {
        PACK_STRUCT_FIELD(u32_t addr[4]);
    }PACK_STRUCT_STRUCT;
    PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/epstruct.h"
#endif

    /*
     * struct ipaddr2 is used in the definition of the ARP packet format in
     * order to support compilers that don't have structure packing.
     */
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/bpstruct.h"
#endif
    PACK_STRUCT_BEGIN
    struct ip6_addr2
    {
        PACK_STRUCT_FIELD(u16_t addrw[2]);
    }PACK_STRUCT_STRUCT;
    PACK_STRUCT_END
#ifdef PACK_STRUCT_USE_INCLUDES
#  include "arch/epstruct.h"
#endif

#define IP6_ADDR(ipaddr, a,b,c,d,e,f,g,h) do { (ipaddr)->addr[0] = htonl((u32_t)((a & 0xffff) << 16) | (b & 0xffff)); \
                                               (ipaddr)->addr[1] = htonl(((c & 0xffff) << 16) | (d & 0xffff)); \
                                               (ipaddr)->addr[2] = htonl(((e & 0xffff) << 16) | (f & 0xffff)); \
                                               (ipaddr)->addr[3] = htonl(((g & 0xffff) << 16) | (h & 0xffff)); } while(0)

    u8_t ip6_addr_netcmp(struct ip6_addr *addr1, struct ip6_addr *addr2, struct ip6_addr *mask);
    u8_t ip6_addr_cmp(struct ip6_addr *addr1, struct ip6_addr *addr2);
    void ip6_addr_set(struct ip6_addr *dest, struct ip6_addr *src);
    u8_t ip6_addr_isany(struct ip6_addr *addr);

#define ip6_addr_ismulticast(addr1) (((addr1)->addr[0] & ntohl(0xFF000000UL)) == ntohl(0xFF000000UL))

#define ip6_addr_isallnodes(addr1) ( (addr1)->addr[0]  == ntohl(0xFF020000UL) && (addr1)->addr[3] == ntohl(0x00000001UL) )

#define ip6_addr_issolicitedmulticast(addr1,netif_addr) ( ((addr1)->addr[0]  == htonl(0xFF020000UL)) && \
                                                         ((addr1)->addr[1]  == 0x0) && \
                                                         ((addr1)->addr[2]  == htonl(0x1UL)) && \
                                                         ((addr1)->addr[3]  == (htonl(0xFF000000 | (htonl ((netif_addr)->addr[3]) & 0xFFFFFF)))) )
#define ip6_addr_debug_print(debug, ipaddr) \
        LWIP_DEBUGF(debug, ("%"X32_F":%"X32_F":%"X32_F":%"X32_F":%"X32_F":%"X32_F":%"X32_F":%"X32_F"\n", \
         (ntohl(ipaddr->addr[0]) >> 16) & 0xffff, \
         ntohl(ipaddr->addr[0]) & 0xffff, \
         (ntohl(ipaddr->addr[1]) >> 16) & 0xffff, \
         ntohl(ipaddr->addr[1]) & 0xffff, \
         (ntohl(ipaddr->addr[2]) >> 16) & 0xffff, \
         ntohl(ipaddr->addr[2]) & 0xffff, \
         (ntohl(ipaddr->addr[3]) >> 16) & 0xffff, \
         ntohl(ipaddr->addr[3]) & 0xffff));

#ifdef __cplusplus
}
#endif

#endif /* __LWIP_IP_ADDR_H__ */
