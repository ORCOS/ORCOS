/*
 * ip.h
 *
 *  Created on: 19.05.2010
 *      Author: dbaldin
 */

#ifndef IP_H_
#define IP_H_

#include "ipv4/ip4_addr.h"
#include "ipv6/ip6_addr.h"
#include "ipv4/ip4.h"
#include "ipv6/ip6.h"
#include "types.hh"

#define IPV4 4
#define IPV6 6

struct netif;

struct ip_hdr {
#if BYTE_ORDER == LITTLE_ENDIAN
    u8_t tclass1 :4, v :4;
#else
u8_t v:4, tclass1:4;
#endif
};

// ip addresses need to be aligned in memory for several functions accessing the arrays using pointers!
/*struct ip_addr {
 u32 addr[4];
 u32 version; // ipv4 or ipv6
 } __attribute__((__aligned__));*/

union ip_address {
    struct ip6_addr ip6addr;
    struct ip4_addr ip4addr;
}__attribute__((aligned(4)));

struct ip_addr {
    u8_t version;  // ipv4 or ipv6
    union ip_address addr;
}__attribute__((aligned(4)));

PACK_STRUCT_BEGIN
struct ip_addr_packed {
  PACK_STRUCT_FIELD(u32_t addr);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

typedef struct ip4_addr ip4_addr_t;
typedef struct ip_addr  ip_addr_t;

typedef struct ip_addr_packed ip_addr_p_t;

#define IP_PCB \
  /* ip addresses in network byte order */ \
  struct ip_addr local_ip; \
  struct ip_addr remote_ip; \
   /* Socket options */  \
  u16_t so_options;      \
   /* Type Of Service */ \
  u8_t tos;              \
  /* Time To Live */     \
  u8_t ttl
  /* link layer address resolution hint */
  //IP_PCB_ADDRHINT

extern const ip_addr_t ip4_addr_any;
extern const ip_addr_t ip4_addr_broadcast;

/** IP_ADDR_ can be used as a fixed IP address
 *  for the wildcard and the broadcast address
 */
#define IP4_ADDR_ANY         ((ip_addr_t *)&ip4_addr_any)
#define IP4_ADDR_BROADCAST   ((ip_addr_t *)&ip4_addr_broadcast)

/** 255.255.255.255 */
#define IP4ADDR_NONE         ((u32_t)0xffffffffUL)
/** 127.0.0.1 */
#define IP4ADDR_LOOPBACK     ((u32_t)0x7f000001UL)
/** 0.0.0.0 */
#define IP4ADDR_ANY          ((u32_t)0x00000000UL)
/** 255.255.255.255 */
#define IP4ADDR_BROADCAST    ((u32_t)0xffffffffUL)

/** IPv4 only: set the IP address given as an u32_t */
#define ip4_addr_set_u32(dest_ipaddr, src_u32) ((dest_ipaddr)->addr = (src_u32))
/** IPv4 only: get the IP address as an u32_t */
#define ip4_addr_get_u32(src_ipaddr) ((src_ipaddr)->addr)



/** MEMCPY-like copying of IP addresses where addresses are known to be
 * 16-bit-aligned if the port is correctly configured (so a port could define
 * this to copying 2 u16_t's) - no NULL-pointer-checking needed. */
#ifndef IPADDR2_COPY
#define IPADDR2_COPY(dest, src) SMEMCPY(dest, src, sizeof(ip_addr_t))
#endif


void ip_addr_set(struct ip_addr *dest, struct ip_addr *src);
#define ip_addr_copy(dest, src) ip_addr_set(&dest, &src)

void ip_addr_set4(struct ip_addr *dest, struct ip4_addr *src);
void ip_addr_set6(struct ip_addr *dest, struct ip6_addr *src);

u8_t ip_addr_netcmp(struct ip_addr *addr1, struct ip_addr *addr2, struct ip_addr *mask);
u8_t ip_addr_cmp(struct ip_addr *addr1, struct ip_addr *addr2);
u8_t ip_addr_isany(struct ip_addr *addr);

void ip_addr_set_any(struct ip_addr *dest);

struct netif* ip_route(struct ip_addr* dest);

u8_t ip_addr_isbroadcast(struct ip_addr*, struct netif *);
u8_t ip_addr_ismulticast(struct ip_addr*);

u32_t IPH_HL(struct ip_hdr* iphdr);
u16_t IPH_PROTO(struct ip_hdr* iphdr);
void IPH_DEST(struct ip_hdr* iphdr, struct ip_addr* addr);
void IPH_SRC(struct ip_hdr* iphdr, struct ip_addr* addr);

int   ip4addr_aton(const char *cp, ip4_addr_t *addr);
u32_t ipaddr_addr(const char *cp);

/** Gets an IP pcb option (SOF_* flags) */
#define ip_get_option(pcb, opt)   ((pcb)->so_options & (opt))
/** Sets an IP pcb option (SOF_* flags) */
#define ip_set_option(pcb, opt)   ((pcb)->so_options |= (opt))
/** Resets an IP pcb option (SOF_* flags) */
#define ip_reset_option(pcb, opt) ((pcb)->so_options &= ~(opt))

#endif /* IP_H_ */
