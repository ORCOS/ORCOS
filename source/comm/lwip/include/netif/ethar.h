/*
 * ethar.h
 *
 *  Created on: 25.06.2010
 *      Author: ipstackdev
 */

#ifndef ETHAR_H_
#define ETHAR_H_

#include "lwip/opt.h"

#include "lwip/pbuf.h"
#include "ipv4/ip4_addr.h"
#include "ipv6/ip6_addr.h"
#include "lwip/netif.h"
#include "ipv4/ip4.h"
#include "ipv6/ip6.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ETH_PAD_SIZE
#define ETH_PAD_SIZE          0
#endif

#ifndef ETHAR_HWADDR_LEN
#define ETHAR_HWADDR_LEN     6
#endif


/**
 * Try hard to create a new entry - we want the IP address to appear in
 * the cache (even if this means removing an active entry or so). */
#define ETHAR_TRY_HARD 1
#define ETHAR_FIND_ONLY  2


#define ETHTYPE_IPV4      0x0800
#define ETHTYPE_IPV6      0x86dd

//#define AR_TABLE_SIZE 10

PACK_STRUCT_BEGIN
struct eth_addr {
  PACK_STRUCT_FIELD(u8_t addr[ETHAR_HWADDR_LEN]);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

enum ethar_state {
    ETHAR_STATE_EMPTY = 0, ETHAR_STATE_PENDING, ETHAR_STATE_STABLE
};


#if AR_QUEUEING
/** struct for queueing outgoing packets for unknown address
  * defined here to be accessed by memp.h
  */
struct ethar_q_entry {
  struct ethar_q_entry *next;
  struct pbuf *p;
};
#endif /* ARP_QUEUEING */

struct ethar_entry {
#if AR_QUEUEING
    /**
     * Pointer to queue of pending outgoing packets on this ARP entry.
     */
    struct ethar_q_entry *q;
#endif
    struct ip_addr ipaddr_f;
    enum ethar_state state;
    u8_t ctime;
    struct eth_addr ethaddr;
    struct netif *netif;
} PACK_STRUCT_STRUCT;




PACK_STRUCT_BEGIN
struct eth_hdr {
#if ETH_PAD_SIZE
  PACK_STRUCT_FIELD(u8_t padding[ETH_PAD_SIZE]);
#endif
  PACK_STRUCT_FIELD(struct eth_addr dest);
  PACK_STRUCT_FIELD(struct eth_addr src);
  PACK_STRUCT_FIELD(u16_t type);
} PACK_STRUCT_STRUCT;
PACK_STRUCT_END

#define SIZEOF_ETH_HDR (14 + ETH_PAD_SIZE)

union u_ip_hdr {
    struct ip4_hdr ip4hdr;
    struct ip6_hdr ip6hdr;
};


extern struct ethar_entry ar_table[AR_TABLE_SIZE];

void ethar_init(void);

s8_t find_entry(struct ip_addr *ipaddr, u8_t flags);

err_t update_ar_entry(struct netif *netif, struct ip_addr *ipaddr,
        struct eth_addr *ethaddr, u8_t flags);

s8_t ethar_find_addr(struct netif *netif, struct ip_addr *ipaddr,
        struct eth_addr **eth_ret, struct ip_addr **ip_ret);

void ethar_ip_input(struct netif *netif, struct pbuf *p);

err_t
ethar_query(struct netif *netif, struct ip_addr *ipaddr, struct pbuf *q);

err_t
ethar_send_ip(struct netif *netif, struct pbuf *p, struct eth_addr *src, struct eth_addr *dst, u8_t ip_version);

void ethar_tmr(void);

#ifdef __cplusplus
}
#endif

#endif /* ETHAR_H_ */
