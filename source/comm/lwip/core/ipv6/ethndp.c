/*  <...> includes
 *********************************************************************/

#include <stringtools.hh>

/*  "..." includes
 *********************************************************************/
#include "lwip/opt.h"
#include "netif/ethar.h"
#include "ip.h"
#include "ipv6/icmp6.h"
#include "ipv6/ethndp.h"
#include "memtools.hh"

#define SIZEOF_NSM_HDR 24
#define SIZEOF_ICMP6_OPTIONS_HDR 2
#define SIZEOF_RSM_HDR 8

#define SIZEOF_ETHNSM_PACKET (SIZEOF_ETH_HDR + IP6_HLEN + SIZEOF_NSM_HDR)
#define SIZEOF_ETHRSM_PACKET (SIZEOF_ETH_HDR + 6 + SIZEOF_ICMP6_OPTIONS_HDR + SIZEOF_RSM_HDR )


/**
 * Send an NDP router solicitation message. This is used for searching the network for routers.
 *
 * @param netif the lwip network interface on which to send the request
 * * @return ERR_OK if the request has been sent
 *         ERR_MEM if the ARP packet couldn't be allocated
 *         any other err_t on failure
 */
/*err_t
ethndp_send_rsm(struct netif *netif)
{
	struct pbuf *p;
	err_t result = ERR_OK;

	struct eth_hdr *ethhdr;
	struct ip6_hdr *hdr;
	struct icmp6_rsm_hdr *icmphdr;

    p = pbuf_alloc(PBUF_RAW, SIZEOF_ETHRSM_PACKET, PBUF_RAM);
    ethhdr = p->payload;
    hdr = (struct ip6_hdr *)((u8_t*)ethhdr + SIZEOF_ETH_HDR);
    icmphdr = (struct icmp6_rsm_hdr *)((u8_t*)ethhdr + SIZEOF_ETH_HDR + IP6_HLEN);

    // multicast address derived from the all routers ipv6 address
    ethhdr->type = htons(ETHTYPE_IPV6);
	ethhdr->dest.addr[0] = 0xFF;
	ethhdr->dest.addr[1] = 0xFF;
	ethhdr->dest.addr[2] = 0x0;
	ethhdr->dest.addr[3] = 0x0;
	ethhdr->dest.addr[4] = 0x0;
	ethhdr->dest.addr[5] = 0x2;

	for (int k = 0; k < 6; k++)
	{
		ethhdr->src.addr[k] = netif->hwaddr[k];
	}

	 hdr->hoplim = 255;
	 hdr->v = 6;
	 hdr->flow1  = 0;
	 hdr->flow2 = 0;
	 hdr->tclass1 = 0;
	 hdr->tclass2 = 0;
	 hdr->nexthdr = IP6_PROTO_ICMP; // ICMPv6
	 hdr->len = htons(SIZEOF_ETHRSM_PACKET - (SIZEOF_ETH_HDR + IP6_HLEN));

	 // create the solicited node multicast address based on the target address
	 hdr->dest.addr[0] = htonl(0xFF010000);
	 hdr->dest.addr[1] = 0x0;
	 hdr->dest.addr[2] = 0x0;
	 hdr->dest.addr[3] = htonl(0x00000002);

	 icmphdr->type = 133;
	 icmphdr->icode = 0;
	 icmphdr->unused = 0;

	 // calculate checksum
	 icmphdr->chksum = 0;
	 pbuf_header(p,-(SIZEOF_ETH_HDR + IP6_HLEN));
	 icmphdr->chksum = inet_chksum_pseudo(p,&hdr->src.addr,&hdr->dest.addr,IPV6,IP6_PROTO_ICMP,ntohs(hdr->len) );
	 pbuf_header(p,SIZEOF_ETH_HDR + IP6_HLEN);

	 LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("ethndp: sending RSM\n"));

	 // send!
	 result = netif->linkoutput(netif, p);
	 pbuf_free(p);
	 return result;
}*/

/**
 * Send an NDP request packet asking for ipaddr. Will send a Neighbor Solicitation Message
 *
 * @param netif the lwip network interface on which to send the request
 * @param ipaddr the IP address for which to ask
 * @return ERR_OK if the request has been sent
 *         ERR_MEM if the ARP packet couldn't be allocated
 *         any other err_t on failure
 */
err_t
ethndp_request(struct netif *netif, struct ip6_addr *ipaddr)
{

   struct pbuf *p;
   err_t result = ERR_OK;
   u8_t k; /* ARP entry index */
   struct eth_hdr *ethhdr;
   struct ip6_hdr *hdr;
   struct icmp6_nsm_hdr *icmphdr;

   u16_t len = SIZEOF_ETHNSM_PACKET;
   if (netif->ip6_addr_state == IP6_ADDR_STATE_VALID) {
      // if weve got a valid ip6 address we will send the source link layer address inside an additional
      // option frame
    len = (u16_t) (len + SIZEOF_ICMP6_OPTIONS_HDR + 6);
   }

     /* allocate a pbuf for the outgoing ARP request packet */
     p = pbuf_alloc(PBUF_RAW, len, PBUF_RAM);
     /* could allocate a pbuf for an ARP request? */
     if (p == NULL) {
       LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS,
         ("etharp_raw: could not allocate pbuf for ARP request.\n"));
       //ETHARP_STATS_INC(etharp.memerr);
       return ERR_MEM;
     }

     LWIP_ASSERT("check that first pbuf can hold struct etharp_hdr",
                  (p->len >= len));

     ethhdr = p->payload;
     hdr = (struct ip6_hdr *)((u8_t*)ethhdr + SIZEOF_ETH_HDR);
     icmphdr = (struct icmp6_nsm_hdr *)((u8_t*)ethhdr + SIZEOF_ETH_HDR + IP6_HLEN);

     ethhdr->type = htons(ETHTYPE_IPV6);
     ethhdr->dest.addr[0] = 0xFF;
     ethhdr->dest.addr[1] = 0xFF;
     ethhdr->dest.addr[2] = (u8_t) ((ipaddr->addr[3] >> 24) & 0xFF);
     ethhdr->dest.addr[3] = (u8_t) ((ipaddr->addr[3] >> 16) & 0xFF);
     ethhdr->dest.addr[4] = (u8_t) ((ipaddr->addr[3] >> 8) & 0xFF);
     ethhdr->dest.addr[5] = (u8_t) (ipaddr->addr[3] & 0xFF);

     for (k = 0; k < 6; k++)
     {
         ethhdr->src.addr[k] = netif->hwaddr[k];
     }

     hdr->hoplim = 255;
     hdr->v = 6;
     hdr->flow1  = 0;
     hdr->flow2 = 0;
     hdr->tclass1 = 0;
     hdr->tclass2 = 0;
     hdr->nexthdr = IP6_PROTO_ICMP; // ICMPv6
     hdr->len = htons((u16_t) (len - (SIZEOF_ETH_HDR + IP6_HLEN)));

     // create the solicited node multicast address based on the target address
     hdr->dest.addr[0] = htonl(0xFF020000);
     hdr->dest.addr[1] = 0x0;
     hdr->dest.addr[2] = htonl(0x1);
     hdr->dest.addr[3] = htonl(0xFF000000 | (htonl(ipaddr->addr[3]) & 0xFFFFFF));

     icmphdr->type = 135;
     icmphdr->icode = 0;
     icmphdr->unused = 0;
     ip6_addr_set(&icmphdr->target_addr,ipaddr);

     // fill source address field
     if (netif->ip6_addr_state == IP6_ADDR_STATE_VALID) {
       ip6_addr_set(&hdr->src,&netif->ip6_addr);

       // place the options header behind
       struct icmp6_options_hdr *opthdr = (struct icmp6_options_hdr *)((u8_t*)ethhdr + SIZEOF_ETHNSM_PACKET);
       opthdr->type = ICMP6_OPTIONS_SOURCEADDR;
       opthdr->length = 1;

       SMEMCPY(((u8_t*)ethhdr + SIZEOF_ETHNSM_PACKET+SIZEOF_ICMP6_OPTIONS_HDR),&netif->hwaddr[0],6);

     }
     else
     {
         hdr->src.addr[0] = 0;
         hdr->src.addr[1] = 0;
         hdr->src.addr[2] = 0;
         hdr->src.addr[3] = 0;
     }


     // calculate checksum
     icmphdr->chksum = 0;
     pbuf_header(p,-(SIZEOF_ETH_HDR + IP6_HLEN));
     icmphdr->chksum = inet_chksum_pseudo(p,&hdr->src.addr,&hdr->dest.addr,IPV6,IP6_PROTO_ICMP,ntohs(hdr->len) );
     pbuf_header(p,SIZEOF_ETH_HDR + IP6_HLEN);

     LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("ethndp: sending NSM\n"));

     // send!
     result = netif->linkoutput(netif, p);
     pbuf_free(p);
     return result;
}

/**
 * Output method for ipv6 packets.
 *
 * @param netif the lwip network interface on which to send the request
 * @param ipaddr the IP address for which to ask
 * @return ERR_OK if the request has been sent
 *         ERR_MEM if the ARP packet couldn't be allocated
 *         any other err_t on failure
 */
err_t ethndp_output(struct netif *netif, struct pbuf *q,
        struct ip6_addr *ipaddr) {
    struct eth_addr *dest, mcastaddr;

    /* make room for Ethernet header - should not fail */
    if (pbuf_header(q, sizeof(struct eth_hdr)) != 0)
    {
        /* bail out */
        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE | LWIP_DBG_LEVEL_SERIOUS,
                ("ethndp_output: could not allocate room for header.\n"));
        //LINK_STATS_INC(link.lenerr);
        return ERR_BUF;
    }

    /* assume unresolved Ethernet address */
    dest = NULL;
    /* Determine on destination hardware address. Broadcasts and multicasts
     * are special, other IP addresses are looked up in the ARP table. */

    if (ip6_addr_ismulticast(ipaddr))
    {
        /* Hash IP multicast address to MAC address.*/
        // See RFC 2464 Chapter 7
        mcastaddr.addr[0] = 0x33;
        mcastaddr.addr[1] = 0x33;
        mcastaddr.addr[2] = (u8_t) (((ipaddr->addr[3]) >> 24) & 0xFF);
        mcastaddr.addr[3] = (ipaddr->addr[3] >> 16) & 0xFF;
        mcastaddr.addr[4] = (ipaddr->addr[3] >> 8) & 0xFF;
        mcastaddr.addr[5] = ipaddr->addr[3] & 0xFF;
        /* destination Ethernet address is multicast */
        dest = &mcastaddr;

        /* obtain source Ethernet address of the given interface */
        /* send packet directly on the link */
        return ethar_send_ip(netif, q, (struct eth_addr*) (netif->hwaddr), dest, IPV6);
    }
    else
    {
        /* outside local network? */
        if (!ip6_addr_netcmp(ipaddr, &(netif->ip6_addr), &(netif->ip6_netmask)))
        {
            /* interface has default gateway? */

        	/*if (!ip6_addr_isany(&netif->ip6_gw)) {
        		// we have a gateway .. send packet there

        	}*/

            /* no route to destination error (default gateway missing) */
            return ERR_RTE;
        }

        struct ip_addr ip6addr;
        ip6addr.version = IPV6;
        ip6addr.addr.ip6addr.addr[0] = ipaddr->addr[0];
        ip6addr.addr.ip6addr.addr[1] = ipaddr->addr[1];
        ip6addr.addr.ip6addr.addr[2] = ipaddr->addr[2];
        ip6addr.addr.ip6addr.addr[3] = ipaddr->addr[3];

        /* query on destination Ethernet address belonging to ipaddr */
        return ethar_query(netif, &ip6addr, q);
    }

    /* continuation for multicast/broadcast destinations */
    /* obtain source Ethernet address of the given interface */
    /* send packet directly on the link */
    return ethar_send_ip(netif, q, (struct eth_addr*) (netif->hwaddr), dest, IPV6);
}


