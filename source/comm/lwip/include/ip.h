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
  u8_t tclass1:4, v:4;
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
	struct ip4_addr ip4addr;
	struct ip6_addr ip6addr;
} __attribute__((aligned(4))); ;

struct ip_addr {
    union ip_address addr;
    u8_t version; // ipv4 or ipv6
} __attribute__((aligned(4)));

/*struct ip_addr {
    u32 addr[4];
    u8 version; // ipv4 or ipv6
} __attribute__((__packed__));*/

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
  ///* link layer address resolution hint */
  //IP_PCB_ADDRHINT


void ip_addr_set(struct ip_addr *dest, struct ip_addr *src);
u8_t ip_addr_netcmp(struct ip_addr *addr1, struct ip_addr *addr2, struct ip_addr *mask);
u8_t ip_addr_cmp(struct ip_addr *addr1, struct ip_addr *addr2);
u8_t ip_addr_isany( struct ip_addr *addr);

struct netif* ip_route(struct ip_addr* dest);

u8_t ip_addr_isbroadcast(struct ip_addr*, struct netif *);
u8_t ip_addr_ismulticast(struct ip_addr*);

u32_t IPH_HL(struct ip_hdr* iphdr);
u16_t IPH_PROTO(struct ip_hdr* iphdr);
void IPH_DEST(struct ip_hdr* iphdr,struct ip_addr* addr);
void IPH_SRC(struct ip_hdr* iphdr,struct ip_addr* addr);

#endif /* IP_H_ */
