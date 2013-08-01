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

/* Some ICMP messages should be passed to the transport protocols. This
 is not implemented. */
#include "lwipopts.h"

#if LWIP_ICMP /* don't build if not configured for use in lwipopts.h */

#include "stringtools.hh"
#include "memtools.hh"
#include "ipv6/icmp6.h"
#include "inet.h"
//#include "ipv6/ip6.h"
#include "lwip/def.h"
#include "lwip/stats.h"
#include "netif/ethar.h"
#include "ip.h"

void icmp6_input(struct pbuf *p, struct netif *inp) {
    u8_t type;
    struct icmp6_echo_hdr *iecho;
    struct ip6_hdr *iphdr;
    struct ip6_addr tmpaddr;
    struct icmp6_nsm_hdr *ps_nsm;
    struct icmp6_nad_hdr *ps_nad;
    struct eth_hdr *ethhdr;

    ICMP_STATS_INC(icmp.recv);

    /* TODO: check length before accessing payload! */
    if(pbuf_header(p, -IP6_HLEN)) {
        LWIP_ASSERT("icmpv6: Can't move over ip6 header in packet", 0);
        return;
      }

    type = ((u8_t *) p->payload)[0];

    // TODO: Make implementation configurable!

    switch (type)
    {
        case ICMP6_ECHO:
            LWIP_DEBUGF(ICMP_DEBUG, ("icmp_input: ping\n"));

            if (p->tot_len < sizeof(struct icmp6_echo_hdr))
            {
                LWIP_DEBUGF(ICMP_DEBUG, ("icmp_input: bad ICMP echo received\n"));

                pbuf_free(p);
                ICMP_STATS_INC(icmp.lenerr);
                return;
            }
            iecho = p->payload;
            iphdr = (struct ip6_hdr *) ((u8_t *) p->payload - IP6_HLEN);

            if (inet_chksum_pseudo(p, &iphdr->src.addr, &iphdr->dest.addr,
                    IPV6, IP6_PROTO_ICMP, ntohs(iphdr->len)) != 0)
            {
                LWIP_DEBUGF(ICMP_DEBUG, ("icmp_input: checksum failed!\n"));
                //LWIP_DEBUGF(ICMP_DEBUG, ("icmp_input: checksum failed for received ICMP echo (%"X16_F")\n", inet_chksum_pseudo(p, &(iphdr->src), &(iphdr->dest), IP6_PROTO_ICMP, p->tot_len)));
                ICMP_STATS_INC(icmp.chkerr);
                /*      return;*/
            }
            LWIP_DEBUGF(ICMP_DEBUG, ("icmp: p->len %"S16_F" p->tot_len %"S16_F"\n", p->len, p->tot_len));
            ip6_addr_set(&tmpaddr, &(iphdr->src));
            ip6_addr_set(&(iphdr->src), &(iphdr->dest));
            ip6_addr_set(&(iphdr->dest), &tmpaddr);
            iecho->type = ICMP6_ER;
            /* adjust the checksum */
            /* if (iecho->chksum >= htons(0xffff - (ICMP6_ECHO << 8)))
             {
             iecho->chksum += htons(ICMP6_ECHO << 8) + 1;
             }
             else
             {
             iecho->chksum += htons(ICMP6_ECHO << 8);
             }*/
            iecho->chksum -= htons(0x100);
            //LWIP_DEBUGF(ICMP_DEBUG, ("icmp_input: checksum failed for received ICMP echo (%"X16_F")\n", inet_chksum_pseudo(p, &(iphdr->src), &(iphdr->dest), IP_PROTO_ICMP, p->tot_len)));
            ICMP_STATS_INC(icmp.xmit);

            /*    LWIP_DEBUGF("icmp: p->len %"U16_F" p->tot_len %"U16_F"\n", p->len, p->tot_len);*/
            // calling the next commented line will crash the ubuntu linux kernel v2.6.32-21 - 32
            /* ip6_output_if (p, &(iphdr->src), IP_HDRINCL,
             iphdr->hoplim, IP6_PROTO_ICMP, inp);*/

            pbuf_header(p, IP6_HLEN + SIZEOF_ETH_HDR);
            // modify eth header
            ethhdr = p->payload;
            SMEMCPY(&ethhdr->dest,&ethhdr->src,6);
            SMEMCPY(&ethhdr->src,&inp->hwaddr,6);

            LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("icmpv6: sending echo reply\n"));

            // send!
            inp->linkoutput(inp, p);

            break;
        case ICMP6_NSM:
            // Neighborhood Solicitation Message
            ps_nsm = p->payload;
            if (ip6_addr_cmp(&ps_nsm->target_addr, &inp->ip6_addr)
                    && (inp->ip6_addr_state == IP6_ADDR_STATE_VALID))
            {
                // we are the target
                // send answer. in place modify the packet to use it as a NAD packet
                LWIP_DEBUGF(ICMP_DEBUG, ("icmp: received NSM for our address!\n"));
                iphdr = (struct ip6_hdr *) ((u8_t *) p->payload - IP6_HLEN);

                if (p->len < 32)
                {
                    LWIP_DEBUGF(ICMP_DEBUG, ("icmpv6: not enough room in packet for options frame!\n"));
                    break;
                }

                ip6_addr_set(&iphdr->dest, &iphdr->src);
                ip6_addr_set(&iphdr->src, &inp->ip6_addr);
                iphdr->hoplim = 255;
                ps_nsm->type = 136;
                ps_nsm->icode = 0;
                ps_nsm->unused = htonl(0x3UL << 29);

                struct icmp6_options_hdr *opthdr =
                        (struct icmp6_options_hdr *) ((u8_t*) ps_nsm + 24);
                opthdr->type = ICMP6_OPTIONS_TARGETADDR;
                opthdr->length = 1;

                SMEMCPY(((u8_t*)opthdr + 2),&inp->hwaddr[0],6);

                pbuf_header(p, IP6_HLEN + SIZEOF_ETH_HDR);
                // modify eth header
                ethhdr = p->payload;
                SMEMCPY(&ethhdr->dest,&ethhdr->src,6);
                SMEMCPY(&ethhdr->src,&inp->hwaddr,6);

                ps_nsm->chksum = 0;
                pbuf_header(p, -(SIZEOF_ETH_HDR + IP6_HLEN));
                ps_nsm->chksum = inet_chksum_pseudo(p, &iphdr->src.addr,
                        &iphdr->dest.addr, IPV6, IP6_PROTO_ICMP, ntohs(
                                iphdr->len));
                pbuf_header(p, SIZEOF_ETH_HDR + IP6_HLEN);

                LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("icmpv6: sending NAD on NSM\n"));

                // send!
                inp->linkoutput(inp, p);

            }

            break;

        case ICMP6_NAD:
            // Handle Neighborhood Advertisements
            ps_nad = p->payload;

            if (p->len < 32)
            {
                LWIP_DEBUGF(ICMP_DEBUG, ("icmpv6: nad link layer address missing. ignoring!\n"));
                break;
            }
            iphdr = (struct ip6_hdr *) ((u8_t *) p->payload - IP6_HLEN);

            // add the target / link layer translation to the ar cache
            struct ip_addr ipaddr;
            ipaddr.version = IPV6;
            ip6_addr_set(&ipaddr.addr.ip6addr, &iphdr->src);

            struct icmp6_options_hdr *opthdr =
                    (struct icmp6_options_hdr *) ((u8_t*) ps_nad + 24);

            update_ar_entry(inp, &ipaddr, (struct eth_addr*) ((u8_t*) opthdr
                    + 2), ETHAR_TRY_HARD);
            // check if this advertisement is for us

            if (ip6_addr_cmp(&ps_nad->target_addr, &inp->ip6_addr)
                    && (inp->ip6_addr_state == IP6_ADDR_STATE_TENTATIVE))
            {
                // Duplicate Address Detected!
                // we are screwed up. IP6 address needs to be set manually!
                //LOG(LOG_WARNING,("IP6 Address invalid: Duplicate Address detected!"));
                inp->ip6_addr_state = IP6_ADDR_STATE_INVALID;
            }

            break;
        default:
            LWIP_DEBUGF(ICMP_DEBUG, ("icmp_input: ICMP type %"S16_F" not supported.\n", (s16_t)type));
            ICMP_STATS_INC(icmp.proterr);
            ICMP_STATS_INC(icmp.drop);
    }

    pbuf_free(p);
}

void icmp6_dest_unreach(struct pbuf *p, enum icmp6_dur_type t) {
    struct pbuf *q;
    struct ip6_hdr *iphdr;
    struct icmp6_dur_hdr *idur;

    /* @todo: can this be PBUF_LINK instead of PBUF_IP? */
    q = pbuf_alloc(PBUF_IP, 8 + IP6_HLEN + 8, PBUF_RAM);
    /* ICMP header + IP header + 8 bytes of data */
    if (q == NULL)
    {
        LWIP_DEBUGF(ICMP_DEBUG, ("icmp_dest_unreach: failed to allocate pbuf for ICMP packet.\n"));
        pbuf_free(p);
        return;
    }
    LWIP_ASSERT("check that first pbuf can hold icmp message",
            (q->len >= (8 + IP6_HLEN + 8)));

    iphdr = p->payload;

    idur = q->payload;
    idur->type = (u8_t) ICMP6_DUR;
    idur->icode = (u8_t) t;

    SMEMCPY((u8_t *)q->payload + 8, p->payload, IP6_HLEN + 8);

    /* calculate checksum */
    idur->chksum = 0;
    idur->chksum = inet_chksum(idur, q->len);
    ICMP_STATS_INC(icmp.xmit);

    ip6_output(q, NULL,  &(iphdr->src), ICMP_TTL,
            IP6_PROTO_ICMP);
    pbuf_free(q);
}

void icmp6_time_exceeded(struct pbuf *p, enum icmp6_te_type t) {
    struct pbuf *q;
    struct ip6_hdr *iphdr;
    struct icmp6_te_hdr *tehdr;

    LWIP_DEBUGF(ICMP_DEBUG, ("icmp_time_exceeded\n"));

    /* @todo: can this be PBUF_LINK instead of PBUF_IP? */
    q = pbuf_alloc(PBUF_IP, 8 + IP6_HLEN + 8, PBUF_RAM);
    /* ICMP header + IP header + 8 bytes of data */
    if (q == NULL)
    {
        LWIP_DEBUGF(ICMP_DEBUG, ("icmp_dest_unreach: failed to allocate pbuf for ICMP packet.\n"));
        pbuf_free(p);
        return;
    }
    LWIP_ASSERT("check that first pbuf can hold icmp message",
            (q->len >= (8 + IP6_HLEN + 8)));

    iphdr = p->payload;

    tehdr = q->payload;
    tehdr->type = (u8_t) ICMP6_TE;
    tehdr->icode = (u8_t) t;

    /* copy fields from original packet */
    SMEMCPY((u8_t *)q->payload + 8, (u8_t *)p->payload, IP6_HLEN + 8);

    /* calculate checksum */
    tehdr->chksum = 0;
    tehdr->chksum = inet_chksum(tehdr, q->len);
    ICMP_STATS_INC(icmp.xmit);
    ip6_output(q, NULL,  &(iphdr->src), ICMP_TTL,
            IP6_PROTO_ICMP);
    pbuf_free(q);
}

#endif /* LWIP_ICMP */
