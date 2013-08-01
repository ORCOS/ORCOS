
/*  <...> includes
 *********************************************************************/
#include <stringtools.hh>

/*  "..." includes
 *********************************************************************/
#include "lwip/opt.h"
#include "netif/ethar.h"
#include "lwip/memp.h"
#include "netif/etharp.h"
#include "ipv6/ethndp.h"

#if LWIP_ARP

#if AR_QUEUEING
static void free_ethar_q(struct ethar_q_entry *q);
#endif

const struct eth_addr ethbroadcast =
        { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff } };
const struct eth_addr ethzero = { { 0, 0, 0, 0, 0, 0 } };

// this table will be referenced by the arp and the ndp protocol since it
// is used as the translation table storing ipv4 and ipv6 addresses
struct ethar_entry ar_table[AR_TABLE_SIZE];

#if !LWIP_NETIF_HWADDRHINT
static u8_t ethar_cached_entry;
#endif

#define AR_MAXAGE 240

/** the time an AR entry stays pending after first request,
 *  for AR_TMR_INTERVAL = 5000, this is
 *  (2 * 5) seconds = 10 seconds.
 *
 *  @internal Keep this number at least 2, otherwise it might
 *  run out instantly if the timeout occurs directly after a request.
 */
#define AR_MAXPENDING 2

/**
 * Clears expired entries in the ARP table.
 *
 * This function should be called every ETHARP_TMR_INTERVAL microseconds (5 seconds),
 * in order to expire entries in the ARP table.
 */
void ethar_tmr(void) {
    u8_t i;

    // LWIP_DEBUGF(ETHARP_DEBUG, ("etharp_timer\n"));
    /* remove expired entries from the ARP table */
    for (i = 0; i < AR_TABLE_SIZE; ++i)
    {
        ar_table[i].ctime++;
        if (((ar_table[i].state == ETHAR_STATE_STABLE) && (ar_table[i].ctime
                >= AR_MAXAGE)) || ((ar_table[i].state == ETHAR_STATE_PENDING)
                && (ar_table[i].ctime >= AR_MAXPENDING)))
        {
            /* pending or stable entry has become old! */
            LWIP_DEBUGF(ETHARP_DEBUG, ("etharp_timer: expired %s entry %"U16_F".\n",
                            ar_table[i].state == ETHAR_STATE_STABLE ? "stable" : "pending", (u16_t)i));

            // if this is the ar query for the tentative address of the netif set it to valid now!
            if (ar_table[i].netif->ip6_addr_state == IP6_ADDR_STATE_TENTATIVE)
                ar_table[i].netif->ip6_addr_state = IP6_ADDR_STATE_VALID;

            /* clean up entries that have just been expired */
            /* remove from SNMP ARP index tree */
            //snmp_delete_arpidx_tree(ar_table[i].netif, &ar_table[i].ipaddr);
#if AR_QUEUEING
            /* and empty packet queue */
            if (ar_table[i].q != NULL)
            {
                /* remove all queued packets */
                LWIP_DEBUGF(ETHARP_DEBUG, ("etharp_timer: freeing entry %"U16_F", packet queue %p.\n", (u16_t)i, (void *)(ar_table[i].q)));
                free_ethar_q(ar_table[i].q);
                ar_table[i].q = NULL;
            }
#endif
            /* recycle entry for re-use */
            ar_table[i].state = ETHAR_STATE_EMPTY;
        }
#if AR_QUEUEING
        /* still pending entry? (not expired) */
        if (ar_table[i].state == ETHAR_STATE_PENDING)
        {
            /* resend an AR query here */
            LWIP_DEBUGF(ETHARP_DEBUG, ("resending ar query!\n"));

            //u8_t *version = ((u8_t*) &ar_table[i].ctime) - 1;
            //*version = 4 + (ar_table[i].v << 1);

            ethar_query(ar_table[i].netif,
                    &ar_table[i].ipaddr_f, NULL);

            ar_table[i].state = ETHAR_STATE_PENDING;
        }
#endif
    }
}

void ethar_init() {
	int i;

#if AR_QUEUEING
	 for (i = 0; i < AR_TABLE_SIZE; i++){
		 ar_table[i].q = NULL;
		 ar_table[i].state = ETHAR_STATE_EMPTY;

	}
#endif
}

#if AR_QUEUEING


/**
 * Free a complete queue of etharp entries
 *
 * @param q a qeueue of etharp_q_entry's to free
 */
static void free_ethar_q(struct ethar_q_entry *q) {
    struct ethar_q_entry *r;
    LWIP_ASSERT("q != NULL", q != NULL);
    LWIP_ASSERT("q->p != NULL", q->p != NULL);
    while (q)
    {
        r = q;
        q = q->next;
        LWIP_ASSERT("r->p != NULL", (r->p != NULL));
        pbuf_free(r->p);
        //LOG(LOG_INFO,("free ar_queue calling!\n"));
        memp_free(MEMP_AR_QUEUE, r);
    }
}
#endif

s8_t find_entry(struct ip_addr *ipaddr, u8_t flags) {
    s8_t old_pending = AR_TABLE_SIZE, old_stable = AR_TABLE_SIZE;
    s8_t empty = AR_TABLE_SIZE;
    u8_t i = 0, age_pending = 0, age_stable = 0;
#if AR_QUEUEING
    /* oldest entry with packets on queue */
    s8_t old_queue = AR_TABLE_SIZE;
    /* its age */
    u8_t age_queue = 0;
#endif

    /* First, test if the last call to this function asked for the
     * same address. If so, we're really fast! */
    if (ipaddr)
    {
        /* ipaddr to search for was given */
#if LWIP_NETIF_HWADDRHINT
        if ((netif != NULL) && (netif->addr_hint != NULL))
        {
            /* per-pcb cached entry was given */
            u8_t per_pcb_cache = *(netif->addr_hint);
            if ((per_pcb_cache < AR_TABLE_SIZE) && ar_table[per_pcb_cache].state == ETHAR_STATE_STABLE)
            {
                /* the per-pcb-cached entry is stable */
                if (ip_addr_cmp(ipaddr, &ar_table[per_pcb_cache].ipaddr))
                {
                    /* per-pcb cached entry was the right one! */
                    //ETHARP_STATS_INC(etharp.cachehit);
                    return per_pcb_cache;
                }
            }
        }
#else /* #if LWIP_NETIF_HWADDRHINT */
        if (ar_table[ethar_cached_entry].state == ETHAR_STATE_STABLE)
        {
            /* the cached entry is stable */
        	if (ip_addr_cmp( ipaddr,&ar_table[ethar_cached_entry].ipaddr_f)) {
        		return ethar_cached_entry;
        	}

            /*if (ar_table[ethar_cached_entry].v == 0)
            {
                if (ip4_addr_cmp( &ipaddr->addr.ip4addr, &ar_table[ethar_cached_entry].ipaddr_f.ip4addr))
                    return ethar_cached_entry;
            }
            else if (ip6_addr_cmp(&ipaddr->addr.ip6addr,
                    &ar_table[ethar_cached_entry].ipaddr_f.ip6addr))
                return ethar_cached_entry;*/

        }
#endif /* #if LWIP_NETIF_HWADDRHINT */
    }

    /**
     * a) do a search through the cache, remember candidates
     * b) select candidate entry
     * c) create new entry
     */

    /* a) in a single search sweep, do all of this
     * 1) remember the first empty entry (if any)
     * 2) remember the oldest stable entry (if any)
     * 3) remember the oldest pending entry without queued packets (if any)
     * 4) remember the oldest pending entry with queued packets (if any)
     * 5) search for a matching IP entry, either pending or stable
     *    until 5 matches, or all entries are searched for.
     */

    for (i = 0; i < AR_TABLE_SIZE; ++i)
    {
        /* no empty entry found yet and now we do find one? */
        if ((empty == AR_TABLE_SIZE)
                && (ar_table[i].state == ETHAR_STATE_EMPTY))
        {
            LWIP_DEBUGF(ETHARP_DEBUG, ("find_entry: found empty entry %"U16_F"\n", (u16_t)i));
            /* remember first empty entry */
            empty = i;
        }
        /* pending entry? */
        else if (ar_table[i].state == ETHAR_STATE_PENDING)
        {
            u8_t match = ip_addr_cmp( ipaddr,&ar_table[i].ipaddr_f);

            /* if given, does IP address match IP address in ARP entry? */
           /* if (ar_table[i].v == 0)
            {
                if (ip4_addr_cmp( &ipaddr->addr.ip4addr, &ar_table[i].ipaddr_f.ip4addr))
                    match = 1;
            }
            else if (ip6_addr_cmp(&ipaddr->addr.ip6addr,
                     &ar_table[i].ipaddr_f.ip6addr))
                match = 1;*/

            if (match == 1)
            {
                LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("find_entry: found matching pending entry %"U16_F"\n", (u16_t)i));
                /* found exact IP address match, simply bail out */
#if LWIP_NETIF_HWADDRHINT
                NETIF_SET_HINT(netif, i);
#else /* #if LWIP_NETIF_HWADDRHINT */
                ethar_cached_entry = i;
#endif /* #if LWIP_NETIF_HWADDRHINT */
                return i;
#if AR_QUEUEING
                /* pending with queued packets? */
            }
            else if (ar_table[i].q != NULL)
            {
                if (ar_table[i].ctime >= age_queue)
                {
                    old_queue = i;
                    age_queue = ar_table[i].ctime;
                }
#endif
                /* pending without queued packets? */
            }
            else
            {
                if (ar_table[i].ctime >= age_pending)
                {
                    old_pending = i;
                    age_pending = ar_table[i].ctime;
                }
            }
        }
        /* stable entry? */
        else if (ar_table[i].state == ETHAR_STATE_STABLE)
        {
            /* if given, does IP address match IP address in ARP entry? */
            u8_t match =  ip_addr_cmp( ipaddr,&ar_table[i].ipaddr_f);

          /*  if (ar_table[i].v == 0)
            {
                if (ip4_addr_cmp(&ipaddr->addr.ip4addr, &ar_table[i].ipaddr_f.ip4addr))
                    match = 1;
            }
            else if (ip6_addr_cmp(&ipaddr->addr.ip4addr,
                     &ar_table[i].ipaddr_f.ip4addr))
                match = 1;*/

            if (match == 1)
            {
                LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("find_entry: found matching stable entry %"U16_F"\n", (u16_t)i));
                /* found exact IP address match, simply bail out */
#if LWIP_NETIF_HWADDRHINT
                NETIF_SET_HINT(netif, i);
#else /* #if LWIP_NETIF_HWADDRHINT */
                ethar_cached_entry = i;
#endif /* #if LWIP_NETIF_HWADDRHINT */
                return i;
                /* remember entry with oldest stable entry in oldest, its age in maxtime */
            }
            else if (ar_table[i].ctime >= age_stable)
            {
                old_stable = i;
                age_stable = ar_table[i].ctime;
            }
        }
    }
    /* { we have no match } => try to create a new entry */

    /* no empty entry found and not allowed to recycle? */
    if (((empty == AR_TABLE_SIZE) && ((flags & ETHAR_TRY_HARD) == 0))
    /* or don't create new entry, only search? */
    || ((flags & ETHAR_FIND_ONLY) != 0))
    {
        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("find_entry: no empty entry found and not allowed to recycle\n"));
        return (s8_t) ERR_MEM;
    }

    /* b) choose the least destructive entry to recycle:
     * 1) empty entry
     * 2) oldest stable entry
     * 3) oldest pending entry without queued packets
     * 4) oldest pending entry with queued packets
     *
     * { ETHARP_TRY_HARD is set at this point }
     */

    /* 1) empty entry available? */
    if (empty < AR_TABLE_SIZE)
    {
        i = empty;
        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("find_entry: selecting empty entry %"U16_F"\n", (u16_t)i));
    }
    /* 2) found recyclable stable entry? */
    else if (old_stable < AR_TABLE_SIZE)
    {
        /* recycle oldest stable*/
        i = old_stable;
        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("find_entry: selecting oldest stable entry %"U16_F"\n", (u16_t)i));
#if AR_QUEUEING
        /* no queued packets should exist on stable entries */
        LWIP_ASSERT("arp_table[i].q == NULL", ar_table[i].q == NULL);
#endif
        /* 3) found recyclable pending entry without queued packets? */
    }
    else if (old_pending < AR_TABLE_SIZE)
    {
        /* recycle oldest pending */
        i = old_pending;
        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("find_entry: selecting oldest pending entry %"U16_F" (without queue)\n", (u16_t)i));
#if AR_QUEUEING
        /* 4) found recyclable pending entry with queued packets? */
    }
    else if (old_queue < AR_TABLE_SIZE)
    {
        /* recycle oldest pending */
        i = old_queue;
        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("find_entry: selecting oldest pending entry %"U16_F", freeing packet queue %p\n", (u16_t)i, (void *)(ar_table[i].q)));
        free_ethar_q(ar_table[i].q);
        ar_table[i].q = NULL;
#endif
        /* no empty or recyclable entries found */
    }
    else
    {
        return (s8_t) ERR_MEM;
    }

    /* { empty or recyclable entry found } */
    LWIP_ASSERT("i < AR_TABLE_SIZE", i < AR_TABLE_SIZE);

    /* if (ar_table[i].state != ETHAR_STATE_EMPTY)
     {
     snmp_delete_arpidx_tree(arp_table[i].netif, &arp_table[i].ipaddr);
     } */

#if AR_QUEUEING
    ar_table[i].q = NULL;
#endif

    /* recycle entry (no-op for an already empty entry) */
    ar_table[i].state = ETHAR_STATE_EMPTY;

    /* IP address given? */
    if (ipaddr != NULL)
    {
        /* set IP address */
        //ar_table[i].v = (ipaddr->version - 4) >> 1;
       // SMEMCPY(&ar_table[i].ipaddr_f,&ipaddr->addr, (4 << (ar_table[i].v << 1)) );
    	ar_table[i].ipaddr_f = *ipaddr;

        //ip_addr_set(&ar_table[i].ipaddr, ipaddr);
    }
    ar_table[i].ctime = 0;
#if LWIP_NETIF_HWADDRHINT
    NETIF_SET_HINT(netif, i);
#else /* #if LWIP_NETIF_HWADDRHINT */
    ethar_cached_entry = i;
#endif /* #if LWIP_NETIF_HWADDRHINT */
    return (err_t) i;
}

/**
 * Send an IP packet on the network using netif->linkoutput
 * The ethernet header is filled in before sending.
 *
 * @params netif the lwIP network interface on which to send the packet
 * @params p the packet to send, p->payload pointing to the (uninitialized) ethernet header
 * @params src the source MAC address to be copied into the ethernet header
 * @params dst the destination MAC address to be copied into the ethernet header
 * @return ERR_OK if the packet was sent, any other err_t on failure
 */
err_t ethar_send_ip(struct netif *netif, struct pbuf *p, struct eth_addr *src,
        struct eth_addr *dst, u8_t ip_version) {
    struct eth_hdr *ethhdr = p->payload;
    u8_t k;

    LWIP_ASSERT("netif->hwaddr_len must be the same as ETHAR_HWADDR_LEN for ethar!",
            (netif->hwaddr_len == ETHAR_HWADDR_LEN));
    k = ETHAR_HWADDR_LEN;
    while (k > 0)
    {
        k--;
        ethhdr->dest.addr[k] = dst->addr[k];
        ethhdr->src.addr[k] = src->addr[k];
    }
    if (ip_version == IPV4)
        ethhdr->type = htons(ETHTYPE_IPV4);
    else
        ethhdr->type = htons(ETHTYPE_IPV6);
    LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("etharp_send_ip: sending packet %p\n", (void *)p));
    /* send the packet */
    return netif->linkoutput(netif, p);
}

/**
 * Send an AR request for the given IP address and/or queue a packet.
 * ARP for ipv4 and NDP for ipv6.
 *
 * If the IP address was not yet in the cache, a pending AR cache entry
 * is added and an ARP request is sent for the given address. The packet
 * is queued on this entry.
 *
 * If the IP address was already pending in the cache, a new AR request
 * is sent for the given address. The packet is queued on this entry.
 *
 * If the IP address was already stable in the cache, and a packet is
 * given, it is directly sent and no ARP request is sent out.
 *
 * If the IP address was already stable in the cache, and no packet is
 * given, an ARP request is sent out.
 *
 * @param netif The lwIP network interface on which ipaddr
 * must be queried for.
 * @param ipaddr The IP address to be resolved.
 * @param q If non-NULL, a pbuf that must be delivered to the IP address.
 * q is not freed by this function.
 *
 * @note q must only be ONE packet, not a packet queue!
 *
 * @return
 * - ERR_BUF Could not make room for Ethernet header.
 * - ERR_MEM Hardware address unknown, and no more ARP entries available
 *   to query for address or queue the packet.
 * - ERR_MEM Could not queue packet due to memory shortage.
 * - ERR_RTE No route to destination (no gateway to external networks).
 * - ERR_ARG Non-unicast address given, those will not appear in ARP cache.
 *
 */
err_t ethar_query(struct netif *netif, struct ip_addr *ipaddr, struct pbuf *q) {
    struct eth_addr * srcaddr = (struct eth_addr *) netif->hwaddr;
    err_t result = ERR_MEM;
    s8_t i; /* ARP entry index */

    /* non-unicast address? */
    if (ip_addr_isbroadcast(ipaddr, netif) || ip_addr_ismulticast(ipaddr)
            || ip_addr_isany(ipaddr))
    {
        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("ethar_query: will not add non-unicast IP address to AR cache\n"));
        return ERR_ARG;
    }

    /* find entry in ARP cache, ask to create entry if queueing packet */
#if LWIP_NETIF_HWADDRHINT
    i = find_entry(ipaddr, ETHAR_TRY_HARD, netif);
#else /* LWIP_NETIF_HWADDRHINT */
    i = find_entry(ipaddr, ETHAR_TRY_HARD);
#endif /* LWIP_NETIF_HWADDRHINT */

    /* could not find or create entry? */
    if (i < 0)
    {
        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("ethar_query: could not create AR entry\n"));
        if (q)
        {
            LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("ethar_query: packet dropped\n"));
            //ETHARP_STATS_INC(etharp.memerr);
        }
        return (err_t) i;
    }

    /* mark a fresh entry as pending (we just sent a request) */
    if (ar_table[i].state == ETHAR_STATE_EMPTY)
    {
        ar_table[i].state = ETHAR_STATE_PENDING;
    }

    /* { i is either a STABLE or (new or existing) PENDING entry } */
    LWIP_ASSERT("ar_table[i].state == PENDING or STABLE",
            ((ar_table[i].state == ETHAR_STATE_PENDING) ||
                    (ar_table[i].state == ETHAR_STATE_STABLE)));

    /* do we have a pending entry? or an implicit query request? */
    if ((ar_table[i].state == ETHAR_STATE_PENDING) || (q == NULL))
    {

        ar_table[i].netif = netif;

        if (ipaddr->version == IPV4)
            /* try to resolve it; send out ARP request */
            result = etharp_request(netif,  &ipaddr->addr.ip4addr);
        else
            result = ethndp_request(netif,  &ipaddr->addr.ip6addr);
    }

    /* packet given? */
    if (q != NULL)
    {
        /* stable entry? */
        if (ar_table[i].state == ETHAR_STATE_STABLE)
        {
            /* we have a valid IP->Ethernet address mapping */
            /* send the packet */
            result = ethar_send_ip(netif, q, srcaddr, &(ar_table[i].ethaddr),
                    ipaddr->version);
            /* pending entry? (either just created or already pending */
        }
        else if (ar_table[i].state == ETHAR_STATE_PENDING)
        {
#if AR_QUEUEING /* queue the given q packet */
            struct pbuf *p;
            int copy_needed = 0;
            /* IF q includes a PBUF_REF, PBUF_POOL or PBUF_RAM, we have no choice but
             * to copy the whole queue into a new PBUF_RAM (see bug #11400)
             * PBUF_ROMs can be left as they are, since ROM must not get changed. */
            p = q;
            while (p)
            {
                LWIP_ASSERT("no packet queues allowed!", (p->len != p->tot_len) || (p->next == 0));
                if (p->type != PBUF_ROM)
                {
                    copy_needed = 1;
                    break;
                }
                p = p->next;
            }
            if (copy_needed)
            {
                /* copy the whole packet into new pbufs */
                p = pbuf_alloc(PBUF_RAW, p->tot_len, PBUF_RAM);
                if (p != NULL)
                {
                    if (pbuf_copy(p, q) != ERR_OK)
                    {
                        pbuf_free(p);
                        p = NULL;
                    }
                }
            }
            else
            {
                /* referencing the old pbuf is enough */
                p = q;
                pbuf_ref(p);
            }
            /* packet could be taken over? */
            if (p != NULL)
            {
                /* queue packet ... */
                struct ethar_q_entry *new_entry;
                /* allocate a new arp queue entry */
                new_entry = memp_malloc(MEMP_AR_QUEUE);
                if (new_entry != NULL)
                {
                    new_entry->next = 0;
                    new_entry->p = p;
                    if (ar_table[i].q != NULL)
                    {
                        /* queue was already existent, append the new entry to the end */
                        struct ethar_q_entry *r;
                        r = ar_table[i].q;
                        while (r->next != NULL)
                        {
                            r = r->next;
                        }
                        r->next = new_entry;
                    }
                    else
                    {
                        /* queue did not exist, first item in queue */
                        ar_table[i].q = new_entry;
                    }
                    LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("ethar_query: queued packet %p on AR entry %"S16_F"\n", (void *)q, (s16_t)i));
                    result = ERR_OK;
                }
                else
                {
                    /* the pool MEMP_ARP_QUEUE is empty */
                    pbuf_free(p);
                    LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("ethar_query: could not queue a copy of PBUF_REF packet %p (out of memory)\n", (void *)q));
                    /* { result == ERR_MEM } through initialization */
                }
            }
            else
            {
                //ETHARP_STATS_INC(etharp.memerr);
                LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("ethar_query: could not queue a copy of PBUF_REF packet %p (out of memory)\n", (void *)q));
                /* { result == ERR_MEM } through initialization */
            }
#else /* ARP_QUEUEING == 0 */
            /* q && state == PENDING && ARP_QUEUEING == 0 => result = ERR_MEM */
            /* { result == ERR_MEM } through initialization */
            LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("ethar_query: Ethernet destination address unknown, queueing disabled, packet %p dropped\n", (void *)q));
#endif
        }
    }
    return result;
}

/**
 * Update (or insert) a IP/MAC address pair in the ARP cache.
 *
 * If a pending entry is resolved, any queued packets will be sent
 * at this point.
 *
 * @param ipaddr IP address of the inserted ARP entry.
 * @param ethaddr Ethernet address of the inserted ARP entry.
 * @param flags Defines behaviour:
 * - ETHARP_TRY_HARD Allows ARP to insert this as a new item. If not specified,
 * only existing ARP entries will be updated.
 *
 * @return
 * - ERR_OK Succesfully updated ARP cache.
 * - ERR_MEM If we could not add a new ARP entry when ETHARP_TRY_HARD was set.
 * - ERR_ARG Non-unicast address given, those will not appear in ARP cache.
 *
 * @see pbuf_free()
 */
err_t update_ar_entry(struct netif *netif, struct ip_addr *ipaddr,
        struct eth_addr *ethaddr, u8_t flags) {
    s8_t i;
    u8_t k;
    LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("update_ar_entry() ip_version: %d\n",ipaddr->version));
    LWIP_ASSERT("netif->hwaddr_len == ETHARP_HWADDR_LEN", netif->hwaddr_len == ETHAR_HWADDR_LEN);

    /*LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("update_ar_entry: %"U16_F".%"U16_F".%"U16_F".%"U16_F" - %02"X16_F":%02"X16_F":%02"X16_F":%02"X16_F":%02"X16_F":%02"X16_F"\n",
     ip4_addr1((struct ip4_addr*)&ipaddr->addr[0]), ip4_addr2((struct ip4_addr*)&ipaddr->addr[0]), ip4_addr3((struct ip4_addr*)&ipaddr->addr[0]), ip4_addr4((struct ip4_addr*)&ipaddr->addr[0]),
     ethaddr->addr[0], ethaddr->addr[1], ethaddr->addr[2],
     ethaddr->addr[3], ethaddr->addr[4], ethaddr->addr[5])); */

    /* non-unicast address? */
    if (ip_addr_isany(ipaddr) || ip_addr_isbroadcast(ipaddr, netif)
            || ip_addr_ismulticast(ipaddr))
    {
        LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("update_ar_entry: will not add non-unicast IP address to AR cache\n"));
        return ERR_ARG;
    }
    /* find or create ARP entry */
#if LWIP_NETIF_HWADDRHINT
    i = find_entry(ipaddr, flags, netif);
#else /* LWIP_NETIF_HWADDRHINT */
    i = find_entry(ipaddr, flags);
#endif /* LWIP_NETIF_HWADDRHINT */
    /* bail out if no entry could be found */
    if (i < 0)
        return (err_t) i;

    /* mark it stable */
    ar_table[i].state = ETHAR_STATE_STABLE;
    /* record network interface */
    ar_table[i].netif = netif;

    /* insert in SNMP ARP index tree */
    //snmp_insert_arpidx_tree(netif, &ar_table[i].ipaddr);

    LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("update_ar_entry: updating stable entry %"S16_F"\n", (s16_t)i));
    /* update address */
    k = ETHAR_HWADDR_LEN;
    while (k > 0)
    {
        k--;
        ar_table[i].ethaddr.addr[k] = ethaddr->addr[k];
    }
    /* reset time stamp */
    ar_table[i].ctime = 0;
#if AR_QUEUEING
    /* this is where we will send out queued packets! */
    while (ar_table[i].q != NULL)
    {
        struct pbuf *p;
        /* remember remainder of queue */
        struct ethar_q_entry *q = ar_table[i].q;
        /* pop first item off the queue */
        ar_table[i].q = q->next;
        /* get the packet pointer */
        p = q->p;
        /* now queue entry can be freed */
        //LOG(LOG_INFO,("free ar_queue calling!\n"));
        memp_free(MEMP_AR_QUEUE, q);
        /* send the queued IP packet */
        ethar_send_ip(netif, p, (struct eth_addr*) (netif->hwaddr), ethaddr, ar_table[i].ipaddr_f.version);
        /* free the queued IP packet */
        pbuf_free(p);
    }
#endif
    return ERR_OK;
}

/**
 * Finds (stable) ethernet/IP address pair from AR table
 * using interface and IP address index.
 * @note the addresses in the AR table are in network order!
 *
 * @param netif points to interface index
 * @param ipaddr points to the (network order) IP address index
 * @param eth_ret points to return pointer
 * @param ip_ret points to return pointer
 * @return table index if found, -1 otherwise
 */
#if 0
s8_t ethar_find_addr(struct netif *netif, struct ip_addr *ipaddr,
        struct eth_addr **eth_ret, struct ip_addr **ip_ret)
{
    s8_t i;

    LWIP_UNUSED_ARG(netif);

#if LWIP_NETIF_HWADDRHINT
    i = find_entry(ipaddr, ETHAR_FIND_ONLY, NULL);
#else /* LWIP_NETIF_HWADDRHINT */
    i = find_entry(ipaddr, ETHAR_FIND_ONLY);
#endif /* LWIP_NETIF_HWADDRHINT */
    if ((i >= 0) && ar_table[i].state == ETHAR_STATE_STABLE)
    {
        *eth_ret = &ar_table[i].ethaddr;
        *ip_ret = &ar_table[i].ipaddr;
        return i;
    }
    return -1;
}
#endif
/**
 * Updates the AR table using the given IP packet.
 *
 * Uses the incoming IP packet's source address to update the
 * AR cache for the local network. The function does not alter
 * or free the packet. This function must be called before the
 * packet p is passed to the IP layer.
 *
 * @param netif The lwIP network interface on which the IP packet pbuf arrived.
 * @param p The IP packet that arrived on netif.
 *
 * @return NULL
 *
 * @see pbuf_free()
 */
void ethar_ip_input(struct netif *netif, struct pbuf *p) {
    struct eth_hdr *ethhdr;
    union u_ip_hdr *iphdr;
    struct ip_addr ipaddr;

    LWIP_ERROR("netif != NULL", (netif != NULL), return;);
    /* Only insert an entry if the source IP address of the
     incoming IP packet comes from a host on the local network. */
    ethhdr = p->payload;

    iphdr = (union u_ip_hdr *) ((u8_t*) ethhdr + SIZEOF_ETH_HDR);

#if ETHAR_SUPPORT_VLAN
    if (ethhdr->type == ETHTYPE_VLAN)
    {
        iphdr = (struct ip_hdr *)((u8_t*)ethhdr + SIZEOF_ETH_HDR + SIZEOF_VLAN_HDR);
    }
#endif /* ETHARP_SUPPORT_VLAN */

    if (htons(ethhdr->type) == ETHTYPE_IPV4)
    {
        /* source is not on the local network? */
        if (!ip4_addr_netcmp(&(iphdr->ip4hdr.src), &(netif->ip4_addr),
                &(netif->ip4_netmask)))
            /* do nothing */
            return;

        ipaddr.version = IPV4;
        ipaddr.addr.ip4addr = iphdr->ip4hdr.src;
    }
    else
    {
        /* source is not on the local network? */
        if (!ip6_addr_netcmp(&(iphdr->ip6hdr.src), &(netif->ip6_addr),
                &(netif->ip6_netmask)))
            /* do nothing */
            return;

        ipaddr.version = IPV6;
        ipaddr.addr.ip6addr.addr[0] = iphdr->ip6hdr.src.addr[0];
        ipaddr.addr.ip6addr.addr[1] = iphdr->ip6hdr.src.addr[1];
        ipaddr.addr.ip6addr.addr[2] = iphdr->ip6hdr.src.addr[2];
        ipaddr.addr.ip6addr.addr[3] = iphdr->ip6hdr.src.addr[3];
    }

    LWIP_DEBUGF(ETHARP_DEBUG | LWIP_DBG_TRACE, ("ethar_ip_input: updating ETHAR table.\n"));

    /* update AR table */
    /* @todo We could use ETHAR_TRY_HARD if we think we are going to talk
     * back soon (for example, if the destination IP address is ours. */
    update_ar_entry(netif, &ipaddr, &(ethhdr->src), 0);
}

#endif
