/**
 * @file
 * Address Resolution Protocol module for IP over Ethernet
 *
 * Functionally, ARP is divided into two parts. The first maps an IP address
 * to a physical address when sending a packet, and the second part answers
 * requests from other machines for our physical address.
 *
 * This implementation complies with RFC 826 (Ethernet ARP). It supports
 * Gratuitious ARP from RFC3220 (IP Mobility Support for IPv4) section 4.6
 * if an interface calls etharp_gratuitous(our_netif) upon address change.
 */

/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * Copyright (c) 2003-2004 Leon Woestenberg <leon.woestenberg@axon.tv>
 * Copyright (c) 2003-2004 Axon Digital Design B.V., The Netherlands.
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
 */

#include "lwip/opt.h"

#if LWIP_ARP /* don't build if not configured for use in lwipopts.h */

#include "inet.h"
#include "ipv4/ip4.h"
#include "lwip/stats.h"
#include "lwip/snmp.h"
#include "lwip/dhcp.h"
#include "ipv4/autoip.h"
#include "netif/etharp.h"

#if PPPOE_SUPPORT
#include "netif/ppp_oe.h"
#endif /* PPPOE_SUPPORT */

#include "stringtools.hh"
#include "memtools.hh"

/** the time an ARP entry stays valid after its last update,
 *  for ARP_TMR_INTERVAL = 5000, this is
 *  (240 * 5) seconds = 20 minutes.
 */
#define ARP_MAXAGE 240
/** the time an ARP entry stays pending after first request,
 *  for ARP_TMR_INTERVAL = 5000, this is
 *  (2 * 5) seconds = 10 seconds.
 *
 *  @internal Keep this number at least 2, otherwise it might
 *  run out instantly if the timeout occurs directly after a request.
 */
#define ARP_MAXPENDING 2

#define HWTYPE_ETHERNET 1

#define ARPH_HWLEN(hdr) (ntohs((hdr)->_hwlen_protolen) >> 8)
#define ARPH_PROTOLEN(hdr) (ntohs((hdr)->_hwlen_protolen) & 0xff)

#define ARPH_HWLEN_SET(hdr, len) (hdr)->_hwlen_protolen = htons(ARPH_PROTOLEN(hdr) | ((len) << 8))
#define ARPH_PROTOLEN_SET(hdr, len) (hdr)->_hwlen_protolen = htons((len) | (ARPH_HWLEN(hdr) << 8))

/*
 enum etharp_state {
 ETHARP_STATE_EMPTY = 0,
 ETHARP_STATE_PENDING,
 ETHARP_STATE_STABLE
 };


 struct etharp_entry {
 #if ARP_QUEUEING

 struct etharp_q_entry *q;
 #endif
 struct ip4_addr ipaddr;
 struct eth_addr ethaddr;
 enum etharp_state state;
 u8_t ctime;
 struct netif *netif;
 };*/

//const struct eth_addr ethbroadcast = {{0xff,0xff,0xff,0xff,0xff,0xff}};
//const struct eth_addr ethzero = {{0,0,0,0,0,0}};
//static struct etharp_entry arp_table[ARP_TABLE_SIZE];
#if !LWIP_NETIF_HWADDRHINT
//static u8_t etharp_cached_entry;
#endif

/**
 * Try hard to create a new entry - we want the IP address to appear in
 * the cache (even if this means removing an active entry or so). */
//#define ETHARP_TRY_HARD 1
//#define ETHARP_FIND_ONLY  2
#if LWIP_NETIF_HWADDRHINT
#define NETIF_SET_HINT(netif, hint)  if (((netif) != NULL) && ((netif)->addr_hint != NULL))  \
                                      *((netif)->addr_hint) = (hint);
static s8_t find_entry(struct ip4_addr *ipaddr, u8_t flags, struct netif *netif);
#else /* LWIP_NETIF_HWADDRHINT */
//static s8_t find_entry(struct ip4_addr *ipaddr, u8_t flags);
#endif /* LWIP_NETIF_HWADDRHINT */

//static err_t update_arp_entry(struct netif *netif, struct ip_addr *ipaddr, struct eth_addr *ethaddr, u8_t flags);

/* Some checks, instead of etharp_init(): */
#if (LWIP_ARP && (ARP_TABLE_SIZE > 0x7f))
#error "If you want to use ARP, ARP_TABLE_SIZE must fit in an s8_t, so, you have to reduce it in your lwipopts.h"
#endif

#if ARP_QUEUEING
/**
 * Free a complete queue of etharp entries
 *
 * @param q a qeueue of etharp_q_entry's to free
 */
/*
 static void
 free_etharp_q(struct etharp_q_entry *q)
 {
 struct etharp_q_entry *r;
 LWIP_ASSERT("q != NULL", q != NULL);
 LWIP_ASSERT("q->p != NULL", q->p != NULL);
 while (q) {
 r = q;
 q = q->next;
 LWIP_ASSERT("r->p != NULL", (r->p != NULL));
 pbuf_free(r->p);
 memp_free(MEMP_ARP_QUEUE, r);
 }
 }*/
#endif

/**
 * Responds to ARP requests to us. Upon ARP replies to us, add entry to cache
 * send out queued IP packets. Updates cache with snooped address pairs.
 *
 * Should be called for incoming ARP packets. The pbuf in the argument
 * is freed by this function.
 *
 * @param netif The lwIP network interface on which the ARP packet pbuf arrived.
 * @param ethaddr Ethernet address of netif.
 * @param p The ARP packet that arrived on netif. Is freed by this function.
 *
 * @return NULL
 *
 * @see pbuf_free()
 */
void etharp_arp_input(struct netif *netif, struct eth_addr *ethaddr, struct pbuf *p) {
    struct etharp_hdr *hdr;
    struct eth_hdr *ethhdr;
    /* these are aligned properly, whereas the ARP header fields might not be */
    struct ip4_addr sipaddr, dipaddr;
    u8_t i;
    u8_t for_us;
#if LWIP_AUTOIP
    const u8_t * ethdst_hwaddr;
#endif /* LWIP_AUTOIP */

    LWIP_ERROR("netif != NULL", (netif != NULL), return;);

    /* drop short ARP packets: we have to check for p->len instead of p->tot_len here
     since a struct etharp_hdr is pointed to p->payload, so it musn't be chained! */
    if (p->len < SIZEOF_ETHARP_PACKET) {
        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_WARNING, ("etharp_arp_input: packet dropped, too short (%"S16_F"/%"S16_F")"NEWLINE, p->tot_len, (s16_t)SIZEOF_ETHARP_PACKET));ETHARP_STATS_INC(etharp.lenerr);ETHARP_STATS_INC(etharp.drop);
        pbuf_free(p);
        return;
    }

    ethhdr = p->payload;
    hdr = (struct etharp_hdr *) ((u8_t*) ethhdr + SIZEOF_ETH_HDR);
#if ETHARP_SUPPORT_VLAN
    if (ethhdr->type == ETHTYPE_VLAN) {
        hdr = (struct etharp_hdr *)(((u8_t*)ethhdr) + SIZEOF_ETH_HDR + SIZEOF_VLAN_HDR);
    }
#endif /* ETHARP_SUPPORT_VLAN */

    /* RFC 826 "Packet Reception": */
    if ((hdr->hwtype != htons(HWTYPE_ETHERNET)) || (hdr->_hwlen_protolen != htons((ETHARP_HWADDR_LEN << 8) | sizeof(struct ip4_addr)))
            || (hdr->proto != htons(ETHTYPE_IPV4)) || (ethhdr->type != htons(ETHTYPE_ARP))) {
        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_WARNING, ("etharp_arp_input: packet dropped, wrong hw type, hwlen, proto, protolen or ethernet type (%"U16_F"/%"U16_F"/%"U16_F"/%"U16_F"/%"U16_F")"NEWLINE, hdr->hwtype, ARPH_HWLEN(hdr), hdr->proto, ARPH_PROTOLEN(hdr), ethhdr->type));ETHARP_STATS_INC(etharp.proterr);ETHARP_STATS_INC(etharp.drop);
        pbuf_free(p);
        return;
    }ETHARP_STATS_INC(etharp.recv);

#if LWIP_AUTOIP
    /* We have to check if a host already has configured our random
     * created link local address and continously check if there is
     * a host with this IP-address so we can detect collisions */
    autoip_arp_reply(netif, hdr);
#endif /* LWIP_AUTOIP */

    /* Copy struct ip_addr2 to aligned ip_addr, to support compilers without
     * structure packing (not using structure copy which breaks strict-aliasing rules). */
    SMEMCPY(&sipaddr, &hdr->sipaddr, sizeof(sipaddr));
    SMEMCPY(&dipaddr, &hdr->dipaddr, sizeof(dipaddr));

    /* this interface is not configured? */
    if (netif->ip4_addr.addr == 0) {
        for_us = 0;
    } else {
        /* ARP packet directed to us? */
        //for_us = ip4_addr_cmp(&dipaddr,(struct ip4_addr*) &(netif->ip4_addr.addr[0]));
        for_us = ip4_addr_cmp(&dipaddr, &(netif->ip4_addr));
    }

    struct ip_addr ipaddr;
    ipaddr.version = IPV4;
    ipaddr.addr.ip4addr.addr = sipaddr.addr;

    /* ARP message directed to us? */
    if (for_us) {
        /* add IP address in ARP cache; assume requester wants to talk to us.
         * can result in directly sending the queued packets for this host. */
        update_ar_entry(netif, &ipaddr, &(hdr->shwaddr), ETHAR_TRY_HARD);
        /* ARP message not directed to us? */
    } else {
        /* update the source IP address in the cache, if present */
        update_ar_entry(netif, &ipaddr, &(hdr->shwaddr), 0);
    }

    /* now act on the message itself */
    switch (htons(hdr->opcode)) {
    /* ARP request? */
    case ARP_REQUEST:
        /* ARP request. If it asked for our address, we send out a
         * reply. In any case, we time-stamp any existing ARP entry,
         * and possiby send out an IP packet that was queued on it. */

        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("etharp_arp_input: incoming ARP request"NEWLINE));
        /* ARP request for our address? */
        if (for_us) {

            LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("etharp_arp_input: replying to ARP request for our IP address"NEWLINE));
            /* Re-use pbuf to send ARP reply.
             Since we are re-using an existing pbuf, we can't call etharp_raw since
             that would allocate a new pbuf. */
            hdr->opcode = htons(ARP_REPLY);

            hdr->dipaddr = hdr->sipaddr;
            // SMEMCPY(&hdr->sipaddr, &netif->ip4_addr.addr[0], sizeof(hdr->sipaddr));
            SMEMCPY(&hdr->sipaddr, &netif->ip4_addr, sizeof(hdr->sipaddr));

            LWIP_ASSERT("netif->hwaddr_len must be the same as ETHARP_HWADDR_LEN for etharp!", (netif->hwaddr_len == ETHARP_HWADDR_LEN));
            i = ETHARP_HWADDR_LEN;
#if LWIP_AUTOIP
            /* If we are using Link-Local, ARP packets must be broadcast on the
             * link layer. (See RFC3927 Section 2.5) */
            ethdst_hwaddr = ((netif->autoip != NULL) && (netif->autoip->state != AUTOIP_STATE_OFF)) ? (u8_t*)(ethbroadcast.addr) : hdr->shwaddr.addr;
#endif /* LWIP_AUTOIP */

            while (i > 0) {
                i--;
                hdr->dhwaddr.addr[i] = hdr->shwaddr.addr[i];
#if LWIP_AUTOIP
                ethhdr->dest.addr[i] = ethdst_hwaddr[i];
#else  /* LWIP_AUTOIP */
                ethhdr->dest.addr[i] = hdr->shwaddr.addr[i];
#endif /* LWIP_AUTOIP */
                hdr->shwaddr.addr[i] = ethaddr->addr[i];
                ethhdr->src.addr[i]  = ethaddr->addr[i];
            }

            /* hwtype, hwaddr_len, proto, protolen and the type in the ethernet header
             are already correct, we tested that before */

            /* return ARP reply */
            netif->linkoutput(netif, p);
            /* we are not configured? */
        } else if (netif->ip4_addr.addr == 0) {
            /* { for_us == 0 and netif->ip_addr.addr == 0 } */
            LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("etharp_arp_input: we are unconfigured, ARP request ignored."NEWLINE));
            /* request was not directed to us */
        } else {
            /* { for_us == 0 and netif->ip_addr.addr != 0 } */
            LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("etharp_arp_input: ARP request was not for us."NEWLINE));
        }
        break;
    case ARP_REPLY:
        /* ARP reply. We already updated the ARP cache earlier. */
        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("etharp_arp_input: incoming ARP reply"NEWLINE));
#if (LWIP_DHCP && DHCP_DOES_ARP_CHECK)
        /* DHCP wants to know about ARP replies from any host with an
         * IP address also offered to us by the DHCP server. We do not
         * want to take a duplicate IP address on a single network.
         * @todo How should we handle redundant (fail-over) interfaces? */
        dhcp_arp_reply(netif, &sipaddr);
#endif
        break;
    default:
        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("etharp_arp_input: ARP unknown opcode type %"S16_F""NEWLINE, htons(hdr->opcode)));
        ETHARP_STATS_INC(etharp.err);
        break;
    }
    /* free ARP packet */
    pbuf_free(p);
}

/**
 * Resolve and fill-in Ethernet address header for outgoing IP packet.
 *
 * For IP multicast and broadcast, corresponding Ethernet addresses
 * are selected and the packet is transmitted on the link.
 *
 * For unicast addresses, the packet is submitted to etharp_query(). In
 * case the IP address is outside the local network, the IP address of
 * the gateway is used.
 *
 * @param netif The lwIP network interface which the IP packet will be sent on.
 * @param q The pbuf(s) containing the IP packet to be sent.
 * @param ipaddr The IP address of the packet destination.
 *
 * @return
 * - ERR_RTE No route to destination (no gateway to external networks),
 * or the return type of either etharp_query() or etharp_send_ip().
 */
err_t etharp_output(struct netif *netif, struct pbuf *q, struct ip4_addr *ipaddr) {
    struct eth_addr *dest, mcastaddr;

    /* make room for Ethernet header - should not fail */
    if (pbuf_header(q, sizeof(struct eth_hdr)) != 0) {
        /* bail out */
        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS, ("etharp_output: could not allocate room for header."NEWLINE));LINK_STATS_INC(link.lenerr);
        return ERR_BUF;
    }

    /* assume unresolved Ethernet address */
    dest = NULL;
    /* Determine on destination hardware address. Broadcasts and multicasts
     * are special, other IP addresses are looked up in the ARP table. */

    /* broadcast destination IP address? */
    if (ip4_addr_isbroadcast(ipaddr, netif)) {
        /* broadcast on Ethernet also */
        dest = (struct eth_addr *) &ethbroadcast;
        /* multicast destination IP address? */
    } else if (ip4_addr_ismulticast(ipaddr)) {
        /* Hash IP multicast address to MAC address.*/
        mcastaddr.addr[0] = 0x01;
        mcastaddr.addr[1] = 0x00;
        mcastaddr.addr[2] = 0x5e;
        mcastaddr.addr[3] = ip4_addr2(ipaddr) & 0x7f;
        mcastaddr.addr[4] = ip4_addr3(ipaddr);
        mcastaddr.addr[5] = ip4_addr4(ipaddr);
        /* destination Ethernet address is multicast */
        dest = &mcastaddr;
        /* unicast destination IP address? */
    } else {
        /* outside local network? */
        // if (!ip4_addr_netcmp(ipaddr,(struct ip4_addr*) &(netif->ip4_addr.addr[0]),(struct ip4_addr*) &(netif->ip4_netmask.addr[0]))) {
        if (!ip4_addr_netcmp(ipaddr, &(netif->ip4_addr), &(netif->ip4_netmask))) {
            /* interface has default gateway? */
            if (netif->ip4_gw.addr != 0) {
                /* send to hardware address of default gateway IP address */
                // ipaddr = (struct ip4_addr*) &(netif->ip4_gw.addr[0]);
                ipaddr = &(netif->ip4_gw);
                LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS, ("etharp_output: sending to gateway %x."NEWLINE,netif->ip4_gw.addr));
            } else {
                /* no route to destination error (default gateway missing) */
                return ERR_RTE;
            }
        }

        struct ip_addr ip4addr;
        ip4addr.version = IPV4;
        ip4addr.addr.ip4addr.addr = ipaddr->addr;
        /* queue on destination Ethernet address belonging to ipaddr */
        return ethar_query(netif, &ip4addr, q);
    }

    /* continuation for multicast/broadcast destinations */
    /* obtain source Ethernet address of the given interface */
    /* send packet directly on the link */
    return ethar_send_ip(netif, q, (struct eth_addr*) (netif->hwaddr), dest, IPV4);
}

/**
 * Send a raw ARP packet (opcode and all addresses can be modified)
 *
 * @param netif the lwip network interface on which to send the ARP packet
 * @param ethsrc_addr the source MAC address for the ethernet header
 * @param ethdst_addr the destination MAC address for the ethernet header
 * @param hwsrc_addr the source MAC address for the ARP protocol header
 * @param ipsrc_addr the source IP address for the ARP protocol header
 * @param hwdst_addr the destination MAC address for the ARP protocol header
 * @param ipdst_addr the destination IP address for the ARP protocol header
 * @param opcode the type of the ARP packet
 * @return ERR_OK if the ARP packet has been sent
 *         ERR_MEM if the ARP packet couldn't be allocated
 *         any other err_t on failure
 */
#if !LWIP_AUTOIP
static
#endif /* LWIP_AUTOIP */
err_t etharp_raw(struct netif *netif, const struct eth_addr *ethsrc_addr, const struct eth_addr *ethdst_addr, const struct eth_addr *hwsrc_addr, const struct ip4_addr *ipsrc_addr, const struct eth_addr *hwdst_addr, const struct ip4_addr *ipdst_addr, const u16_t opcode) {
    struct pbuf *p;
    err_t result = ERR_OK;
    u8_t k; /* ARP entry index */
    struct eth_hdr *ethhdr;
    struct etharp_hdr *hdr;
#if LWIP_AUTOIP
    const u8_t * ethdst_hwaddr;
#endif /* LWIP_AUTOIP */

    /* allocate a pbuf for the outgoing ARP request packet */
    p = pbuf_alloc(PBUF_RAW, SIZEOF_ETHARP_PACKET, PBUF_RAM);
    /* could allocate a pbuf for an ARP request? */
    if (p == NULL) {
        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS, ("etharp_raw: could not allocate pbuf for ARP request."NEWLINE));ETHARP_STATS_INC(etharp.memerr);
        return ERR_MEM;
    }
    LWIP_ASSERT("check that first pbuf can hold struct etharp_hdr", (p->len >= SIZEOF_ETHARP_PACKET));

    ethhdr = p->payload;
    hdr = (struct etharp_hdr *) ((u8_t*) ethhdr + SIZEOF_ETH_HDR);
    LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("etharp_raw: sending raw ARP packet."NEWLINE));
    hdr->opcode = htons(opcode);

    LWIP_ASSERT("netif->hwaddr_len must be the same as ETHARP_HWADDR_LEN for etharp!", (netif->hwaddr_len == ETHARP_HWADDR_LEN));
    k = ETHARP_HWADDR_LEN;
#if LWIP_AUTOIP
    /* If we are using Link-Local, ARP packets must be broadcast on the
     * link layer. (See RFC3927 Section 2.5) */
    ethdst_hwaddr = ((netif->autoip != NULL) && (netif->autoip->state != AUTOIP_STATE_OFF)) ? (u8_t*)(ethbroadcast.addr) : ethdst_addr->addr;
#endif /* LWIP_AUTOIP */
    /* Write MAC-Addresses (combined loop for both headers) */
    while (k > 0) {
        k--;
        /* Write the ARP MAC-Addresses */
        hdr->shwaddr.addr[k] = hwsrc_addr->addr[k];
        hdr->dhwaddr.addr[k] = hwdst_addr->addr[k];
        /* Write the Ethernet MAC-Addresses */
#if LWIP_AUTOIP
        ethhdr->dest.addr[k] = ethdst_hwaddr[k];
#else  /* LWIP_AUTOIP */
        ethhdr->dest.addr[k] = ethdst_addr->addr[k];
#endif /* LWIP_AUTOIP */
        ethhdr->src.addr[k] = ethsrc_addr->addr[k];
    }
    hdr->sipaddr = *(struct ip4_addr2 *) ipsrc_addr;
    hdr->dipaddr = *(struct ip4_addr2 *) ipdst_addr;

    hdr->hwtype = htons(HWTYPE_ETHERNET);
    hdr->proto = htons(ETHTYPE_IPV4);
    /* set hwlen and protolen together */
    hdr->_hwlen_protolen = htons((ETHARP_HWADDR_LEN << 8) | sizeof(struct ip4_addr));

    ethhdr->type = htons(ETHTYPE_ARP);
    /* send ARP query */
    result = netif->linkoutput(netif, p);
    ETHARP_STATS_INC(etharp.xmit);
    /* free ARP query packet not needed..*/
    pbuf_free(p);
    p = NULL;
    /* could not allocate pbuf for ARP request */

    return result;
}

/**
 * Send an ARP request packet asking for ipaddr.
 *
 * @param netif the lwip network interface on which to send the request
 * @param ipaddr the IP address for which to ask
 * @return ERR_OK if the request has been sent
 *         ERR_MEM if the ARP packet couldn't be allocated
 *         any other err_t on failure
 */
err_t etharp_request(struct netif *netif, struct ip4_addr *ipaddr) {
    LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("etharp_request: sending ARP request."NEWLINE));

    return etharp_raw(netif, (struct eth_addr *) netif->hwaddr, &ethbroadcast, (struct eth_addr *) netif->hwaddr, &netif->ip4_addr, &ethzero, ipaddr, ARP_REQUEST);
}

#endif /* LWIP_ARP */
