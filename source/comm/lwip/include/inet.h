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
#ifndef __LWIP_INET_H__
#define __LWIP_INET_H__

#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "ip.h"

struct ip4_addr;
struct ip6_addr;
struct ip_addr;

#ifdef __cplusplus
extern "C"
{
#endif

    u16_t inet_chksum(void *data, u16_t len);
    u16_t inet_chksum_pbuf(struct pbuf *p);

    u16_t inet_chksum_pseudo(struct pbuf *p,
            void *src_ip, void *dest_ip, u8_t ip_version,
            register u8_t proto,register u16_t proto_len);

//static u16_t chksum(void *dataptr, u16_t len);
    u32_t chksum(u16_t *data, u16_t len);

//u32_t inet_addr(const char *cp);
//s8_t inet_aton(const char *cp, struct in_addr *addr);

#ifdef htons
#undef htons
#endif /* htons */
#ifdef htonl
#undef htonl
#endif /* htonl */
#ifdef ntohs
#undef ntohs
#endif /* ntohs */
#ifdef ntohl
#undef ntohl
#endif /* ntohl */

#ifndef LWIP_PLATFORM_BYTESWAP
#define LWIP_PLATFORM_BYTESWAP 0
#endif

#if BYTE_ORDER == BIG_ENDIAN
#define htons(x) (x)
#define ntohs(x) (x)
#define htonl(x) (x)
#define ntohl(x) (x)
#define ntohl_24(x) (x)

#define PP_HTONS(x) (x)
#define PP_NTOHS(x) (x)
#define PP_HTONL(x) (x)
#define PP_NTOHL(x) (x)

#else /* BYTE_ORDER != BIG_ENDIAN */
#ifdef LWIP_PREFIX_BYTEORDER_FUNCS
    /* workaround for naming collisions on some platforms */
#define htons lwip_htons
#define ntohs lwip_ntohs
#define htonl lwip_htonl
#define ntohl lwip_ntohl
#endif /* LWIP_PREFIX_BYTEORDER_FUNCS */
#if LWIP_PLATFORM_BYTESWAP
#define htons(x) LWIP_PLATFORM_HTONS(x)
#define ntohs(x) LWIP_PLATFORM_HTONS(x)
#define htonl(x) LWIP_PLATFORM_HTONL(x)
#define ntohl(x) LWIP_PLATFORM_HTONL(x)
#else /* LWIP_PLATFORM_BYTESWAP */

    static inline u16_t htons(u16_t x) { return (__builtin_bswap16(x)); }
    static inline u32_t htonl(u32_t x) { return (__builtin_bswap32(x)); }
    static inline u32_t ntohl_24(u32_t x) { return (htonl(x << 8)); }

#define ntohl htonl
#define ntohs htons
#endif /* LWIP_PLATFORM_BYTESWAP */

    /* These macros should be calculated by the preprocessor and are used
       with compile-time constants only (so that there is no little-endian
       overhead at runtime). */
    #define PP_HTONS(x) ((((x) & 0xff) << 8) | (((x) & 0xff00) >> 8))
    #define PP_NTOHS(x) PP_HTONS(x)
    #define PP_HTONL(x) ((((x) & 0xff) << 24) | \
                         (((x) & 0xff00) << 8) | \
                         (((x) & 0xff0000UL) >> 8) | \
                         (((x) & 0xff000000UL) >> 24))
    #define PP_NTOHL(x) PP_HTONL(x)

#endif /* BYTE_ORDER == BIG_ENDIAN */

#ifndef IP4_ADDR2INT
#define IP4_ADDR2INT(a,b,c,d) \
                         PP_HTONL(((u32_t)((a) & 0xff) << 24) | \
                               ((u32_t)((b) & 0xff) << 16) | \
                               ((u32_t)((c) & 0xff) << 8) | \
                                (u32_t)((d) & 0xff))
#endif

#define IP_ADDR_INIT_IPV4(a, b, c, d) { IPV4, {{{ IP4_ADDR2INT(a, b, c ,d) ,0 ,0 ,0}}}}

#ifdef __cplusplus
}
#endif

#endif /* __LWIP_INET_H__ */

