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
#ifndef __LWIP_ICMP_H__
#define __LWIP_ICMP_H__

#include "lwip/opt.h"

#include "lwip/pbuf.h"
#include "lwip/netif.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ICMP6_DUR  1
#define ICMP6_TE   3
#define ICMP6_ECHO 128  /* Echo */
#define ICMP6_ER   129  /* Echo Reply */

#define ICMP6_NSM  135  /* Neighborhood Solicitation Message */
#define ICMP6_NAD  136  /* Neighborhood Advertisement */

#define ICMP6_OPTIONS_SOURCEADDR    1
#define ICMP6_OPTIONS_TARGETADDR    2
#define ICMP6_OPTIONS_PREFIX        3
#define ICMP6_OPTIONS_REDIRECT      4
#define ICMP6_OPTIONS_MTU           5

enum icmp6_dur_type {
  ICMP_DUR_NET = 0,    /* net unreachable */
  ICMP_DUR_HOST = 1,   /* host unreachable */
  ICMP_DUR_PROTO = 2,  /* protocol unreachable */
  ICMP_DUR_PORT = 3,   /* port unreachable */
  ICMP_DUR_FRAG = 4,   /* fragmentation needed and DF set */
  ICMP_DUR_SR = 5      /* source route failed */
};

enum icmp6_te_type {
  ICMP_TE_TTL = 0,     /* time to live exceeded in transit */
  ICMP_TE_FRAG = 1     /* fragment reassembly time exceeded */
};

void icmp6_input(struct pbuf *p, struct netif *inp);

void icmp6_dest_unreach(struct pbuf *p, enum icmp6_dur_type t);
void icmp6_time_exceeded(struct pbuf *p, enum icmp6_te_type t);

struct icmp6_echo_hdr {
  u8_t type;
  u8_t icode;
  u16_t chksum;
  u16_t id;
  u16_t seqno;
} PACK_STRUCT_STRUCT;

// Neighbor Solicitation Message Header
struct icmp6_nsm_hdr {
  u8_t type;
  u8_t icode;
  u16_t chksum;
  u32_t unused;
  struct ip6_addr target_addr;
} PACK_STRUCT_STRUCT;

struct icmp6_nad_hdr {
  u8_t type;
  u8_t icode;
  u16_t chksum;
#if BYTE_ORDER == LITTLE_ENDIAN
    u32_t reserved :29;
    u8_t flag_O :1, flag_S :1, flag_R: 1;
#else
    u8_t flag_R:1, flag_S:1, flag_O:1;
    u32_t reserved:29;
#endif
  struct ip6_addr target_addr;
} PACK_STRUCT_STRUCT;

struct icmp6_dur_hdr {
  u8_t type;
  u8_t icode;
  u16_t chksum;
  u32_t unused;
} PACK_STRUCT_STRUCT;

struct icmp6_options_hdr {
    u8_t type;
    u8_t length;
} PACK_STRUCT_STRUCT;


struct icmp6_te_hdr {
  u8_t type;
  u8_t icode;
  u16_t chksum;
  u32_t unused;
} PACK_STRUCT_STRUCT;

#ifdef __cplusplus
}
#endif


#endif /* __LWIP_ICMP_H__ */

