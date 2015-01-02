/*
 * ethernet.c
 *
 *  Created on: 18.05.2010
 *      Author: dbaldin
 */

#include "lwip/opt.h"
#include "netif/etharp.h"
#include "lwip/stats.h"
#include "lwip/err.h"
#include "lwip/netif.h"
#include "netif/ethar.h"

extern void* comStackMutex;
extern void  acquireMutex(void* mutex);
extern void  releaseMutex(void* mutex);

/**
 * Process received ethernet frames. Using this function instead of directly
 * calling ip_input and passing ARP frames through etharp in ethernetif_input,
 * the ARP cache is protected from concurrent access.
 *
 * @param p the recevied packet, p->payload pointing to the ethernet header
 * @param netif the network interface on which the packet was received
 */
err_t ethernet_input(struct pbuf *p, struct netif *netif) {
    struct eth_hdr* ethhdr;
    u16_t type;

    /* points to packet payload, which starts with an Ethernet header */
    ethhdr = p->payload;
    LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("ethernet_input: dest:%02x:%02x:%02x:%02x:%02x:%02x, src:%02x:%02x:%02x:%02x:%02x:%02x, type:%2x"NEWLINE, (unsigned )ethhdr->dest.addr[0], (unsigned )ethhdr->dest.addr[1], (unsigned )ethhdr->dest.addr[2], (unsigned )ethhdr->dest.addr[3], (unsigned )ethhdr->dest.addr[4], (unsigned )ethhdr->dest.addr[5], (unsigned )ethhdr->src.addr[0], (unsigned )ethhdr->src.addr[1], (unsigned )ethhdr->src.addr[2], (unsigned )ethhdr->src.addr[3], (unsigned )ethhdr->src.addr[4], (unsigned )ethhdr->src.addr[5], (unsigned )htons(ethhdr->type)));

    type = htons(ethhdr->type);
#if ETHARP_SUPPORT_VLAN
    if (type == ETHTYPE_VLAN) {
        struct eth_vlan_hdr *vlan = (struct eth_vlan_hdr*)(((char*)ethhdr) + SIZEOF_ETH_HDR);
#ifdef ETHARP_VLAN_CHECK /* if not, allow all VLANs */
        if (VLAN_ID(vlan) != ETHARP_VLAN_CHECK) {
            /* silently ignore this packet: not for our VLAN */
            pbuf_free(p);
            return ERR_OK;
        }
#endif /* ETHARP_VLAN_CHECK */
        type = htons(vlan->tpid);
    }
#endif /* ETHARP_SUPPORT_VLAN */

    //acquireMutex(comStackMutex);

    switch (type) {
    /* IP packet? */
    case ETHTYPE_IPV4:
#if ETHARP_TRUST_IP_MAC
        /* update ARP table */
        ethar_ip_input(netif, p);
#endif /* ETHARP_TRUST_IP_MAC */
        /* skip Ethernet header */
        if (pbuf_header(p, -(s16_t) SIZEOF_ETH_HDR)) {
            LWIP_ASSERT("Can't move over header in packet", 0);
            pbuf_free(p);
            p = NULL;
        } else {
            /* pass to IP layer */
            ip4_input(p, netif);
        }
        break;

    case ETHTYPE_IPV6:

#if ETHARP_TRUST_IP_MAC
        /* update AR table */
        ethar_ip_input(netif, p);
#endif /* ETHARP_TRUST_IP_MAC */

        if (pbuf_header(p, -(s16_t) SIZEOF_ETH_HDR)) {
            LWIP_ASSERT("Can't move over header in packet", 0);
            pbuf_free(p);
            p = NULL;
        } else {
            ip6_input(p, netif);
        }
        break;

#if LWIP_ARP
    case ETHTYPE_ARP:
        /* pass p to ARP module */
        etharp_arp_input(netif, (struct eth_addr*) (netif->hwaddr), p);
        break;
#endif
#if PPPOE_SUPPORT
        case ETHTYPE_PPPOEDISC: /* PPP Over Ethernet Discovery Stage */
        pppoe_disc_input(netif, p);
        break;

        case ETHTYPE_PPPOE: /* PPP Over Ethernet Session Stage */
        pppoe_data_input(netif, p);
        break;
#endif /* PPPOE_SUPPORT */

    default:
        ETHARP_STATS_INC(etharp.proterr);
        ETHARP_STATS_INC(etharp.drop);
        pbuf_free(p);
        break;
    }

    //releaseMutex(comStackMutex);

    /* This means the pbuf is freed or consumed,
     so the caller doesn't have to free it again */
    return (ERR_OK);
}

