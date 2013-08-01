/*
 * ip.c
 *
 *  Created on: 19.05.2010
 *      Author: dbaldin
 */

#include "ip.h"
#include "ipv4/ip4.h"
#include "ipv6/ip6.h"
#include "lwip/def.h"

void ip_addr_set(struct ip_addr *dest, struct ip_addr *src)
{
	dest->version = src->version;
	if (src->version == IPV4) {
		//ip4_addr_set((struct ip4_addr*) &(dest->addr[0]), (struct ip4_addr*) &src->addr[0]);
		ip4_addr_set(&dest->addr.ip4addr, &src->addr.ip4addr);
	} else
	{
		//ip6_addr_set((struct ip6_addr*) &dest->addr[0], (struct ip6_addr*) &src->addr[0]);
		ip6_addr_set( &dest->addr.ip6addr,  &src->addr.ip6addr);
	}

}

u8_t ip_addr_netcmp(struct ip_addr *addr1, struct ip_addr *addr2, struct ip_addr *mask)
{
	if (addr1 == 0 || addr2 == 0) return 0;

	if (addr1->version == IPV4) {
		//return ip4_addr_netcmp((struct ip4_addr*) &(addr1->addr[0]), (struct ip4_addr*) &(addr2->addr[0]),(struct ip4_addr*) &(mask->addr[0]));
		return ip4_addr_netcmp( &addr1->addr.ip4addr, &addr2->addr.ip4addr, &mask->addr.ip4addr);
	} else
	{
		//return ip6_addr_netcmp((struct ip6_addr*) &(addr1->addr[0]), (struct ip6_addr*) &(addr2->addr[0]),(struct ip6_addr*) &(mask->addr[0]));
		return ip6_addr_netcmp(&addr1->addr.ip6addr, &addr2->addr.ip6addr, &mask->addr.ip6addr);
	}

}

struct netif* ip_route(struct ip_addr* dest) {

	if (dest->version == IPV4)
			return ip4_route( &dest->addr.ip4addr);
		else
			return ip6_route( &dest->addr.ip6addr);


}


u8_t ip_addr_cmp(struct ip_addr *addr1, struct ip_addr *addr2)
{
	if (addr1 == 0 || addr2 == 0) return 0;

    if (addr1->version != addr2->version) return 0;
	if (addr1->version == IPV4) {
		//return ip4_addr_cmp((struct ip4_addr*) &(addr1->addr[0]), (struct ip4_addr*) &(addr2->addr[0]));
		return ip4_addr_cmp( &addr1->addr.ip4addr,  &addr2->addr.ip4addr);
	} else
	{
		//return ip6_addr_cmp((struct ip6_addr*) &(addr1->addr[0]), (struct ip6_addr*) &(addr2->addr[0]));
		return ip6_addr_cmp( &addr1->addr.ip6addr,  &addr2->addr.ip6addr);
	}
}


u8_t ip_addr_isany( struct ip_addr *addr) {
	if (addr == 0) return 1;

	if (addr->version == IPV4) {
			//return ip4_addr_isany((struct ip4_addr*) &(addr->addr[0]));
			return ip4_addr_isany( &addr->addr.ip4addr);
		} else
		{
			//return ip6_addr_isany((struct ip6_addr*) &(addr->addr[0]));
			return ip6_addr_isany(&addr->addr.ip6addr);
		}
}

u8_t ip_addr_isbroadcast(struct ip_addr* addr, struct netif* nif)
{
	if (addr == 0) return 0;

	if (addr->version == IPV4) {
			//return ip4_addr_isbroadcast((struct ip4_addr*) &(addr->addr[0]),nif);
			return ip4_addr_isbroadcast(&addr->addr.ip4addr,nif);
		} else
		{
			//return ip6_addr_ismulticast((struct ip6_addr*) &(addr->addr[0]));
			return ip6_addr_ismulticast(&addr->addr.ip6addr);

		}

}

u8_t ip_addr_ismulticast(struct ip_addr* addr) {

	if (addr == 0) return 0;

	if (addr->version == IPV4) {
				return ip4_addr_ismulticast(&addr->addr.ip4addr);
			} else
			{
				return ip6_addr_ismulticast(&addr->addr.ip6addr);
			}
}

u32_t IPH_HL(struct ip_hdr* iphdr)
{
	if (iphdr->v == IPV4) return 20;
	else if (iphdr->v == IPV6) return 40;
	else return -1;
}

void IPH_DEST(struct ip_hdr* iphdr,struct ip_addr* addr)
{
	addr->version= iphdr->v;

	if (iphdr->v == IPV4) addr->addr.ip4addr.addr = ((struct ip4_hdr*) iphdr)->dest.addr;
	else
		{
			struct ip6_addr* dest = &((struct ip6_hdr*) iphdr)->dest;
			addr->addr.ip6addr.addr[0] = dest->addr[0];
			addr->addr.ip6addr.addr[1] = dest->addr[1];
			addr->addr.ip6addr.addr[2] = dest->addr[2];
			addr->addr.ip6addr.addr[3] = dest->addr[3];
		}

}

void IPH_SRC(struct ip_hdr* iphdr,struct ip_addr* addr)
{
	addr->version= iphdr->v;

	if (iphdr->v == IPV4) addr->addr.ip4addr.addr  = ((struct ip4_hdr*) iphdr)->src.addr;
	else
		{
			struct ip6_addr* dest = &((struct ip6_hdr*) iphdr)->src;
			addr->addr.ip6addr.addr[0] = dest->addr[0];
			addr->addr.ip6addr.addr[1] = dest->addr[1];
			addr->addr.ip6addr.addr[2] = dest->addr[2];
			addr->addr.ip6addr.addr[3] = dest->addr[3];
		}

}
