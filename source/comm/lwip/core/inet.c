/**
 * @file
 * Functions common to all TCP/IPv6 modules, such as the Internet checksum and the
 * byte order functions.
 *
 */

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

#include "lwip/opt.h"

#include "lwip/def.h"
#include "inet.h"

/* chksum:
 *
 * Sums up all 16 bit words in a memory portion. Also includes any odd byte.
 * This function is used by the other checksum functions.
 *
 * For now, this is not optimized. Must be optimized for the particular processor
 * arcitecture on which it is to run. Preferebly coded in assembler.
 */
#if 0
u32_t chksum_old(void *dataptr, u16_t len) {
	u8_t *sdataptr = dataptr;
	u32_t acc;

	// changed to 8 bit accesses!
	for (acc = 0; len > 1; len -= 2) {

#if BIG_ENDIAN
		acc += (*sdataptr) << 8;
		sdataptr++;
		acc += (*sdataptr);
		sdataptr++;
#else
		acc += (*sdataptr);
		sdataptr++;
		acc += (*sdataptr) << 8;
		sdataptr++;
#endif
	}

	/* add up any odd byte */
	if (len == 1) {
		// acc += htons((u16_t)(*(u8_t *)sdataptr) << 8);
		acc += (*sdataptr) << 8;
	}

	return acc;

}
#endif

u32_t chksum(u16_t *data, u16_t len) {
	u32_t sum = 0;
	union {
		u16_t s;
		u8_t b[2];
	} pad;

	// check alignment
	//ASSERT( ((u16_t) data & 0x1) == 0);

	while (len > 1) {
		sum += *data++;
		len = (u16_t) (len - 2);
	}

	if (len == 1) {
		// drop the 8 bits not belonging to the data, should work for both endiannesses
		// TODO: verify this sum on big endian machines
		pad.s = (u16_t) ((u8_t)( *(u16_t *) data));

		//pad.b[0] = (u8_t) ( *(u16_t *) data);
		//pad.b[1] = 0;
		sum += pad.s;
	}

	return (sum);
}


/* inet_chksum_pseudo:
 *
 * Calculates the pseudo Internet checksum used by TCP and UDP for a pbuf chain.
 * dest_ip and src_ip must eb in network order!
 */
u16_t inet_chksum_pseudo(struct pbuf *p, void *src_ip, void *dest_ip,
		u8_t ip_version, register u8_t proto, register u16_t proto_len) {
	u32_t acc;
	struct pbuf *q;
	u8_t swapped;

/*	union {
		u16_t s;
		u8_t b[2];
	} pad;*/

	acc = 0;
	swapped = 0;



	for (q = p; q != NULL ; q = q->next) {
		acc += chksum(q->payload, q->len);

		while (acc >> 16) {
			acc = (acc & 0xffff) + (acc >> 16);
		}
		if (q->len % 2 != 0) {
			swapped = swapped ^ 0x1; //1 - swapped;
			acc = ((acc & 0xff) << 8) | ((acc & 0xff00) >> 8);
		}
	}

	if (swapped) {
		acc = ((acc & 0xff) << 8) | ((acc & 0xff00) >> 8);
	}

	u16_t m = 0;
	if (ip_version == IPV4)
		m = 4;
	else
		m = 16;

	acc += chksum((u16_t*) src_ip,m);
	acc += chksum((u16_t*) dest_ip,m);

/*	for (i = 0; i < m; i += 2) {
		u16_t temp =
				(u16_t) htons( (((u8_t *)src_ip)[i] << 8) | ((u8_t *)src_ip)[i+1])
						& 0xffff;
		acc += temp;
		temp =
				(u16_t) htons( (((u8_t *)dest_ip)[i] << 8) | ((u8_t *)dest_ip)[i+1])
						& 0xffff;
		acc += temp;
	}*/
	acc += htons((u16_t) proto);
	acc += htons((u16_t) proto_len);

	while (acc >> 16) {
		acc = (acc & 0xffff) + (acc >> 16);
	}
	return ((u16_t) (~(acc & 0xffff)));
}

/* inet_chksum:
 *
 * Calculates the Internet checksum over a portion of memory. Used primarely for IP
 * and ICMP.
 */

u16_t inet_chksum(void *dataptr, u16_t len) {
	u32_t acc, sum;

	acc = chksum(dataptr, len);
	sum = (acc & 0xffff) + (acc >> 16);
	//sum += (sum >> 16);
	sum = (sum & 0xffff) + (sum >> 16);
	return ((u16_t) (~(sum & 0xffff)));
}

u16_t inet_chksum_pbuf(struct pbuf *p) {
	u32_t acc;
	struct pbuf *q;
	u8_t swapped;

	acc = 0;
	swapped = 0;
	for (q = p; q != NULL ; q = q->next) {
		acc += chksum(q->payload, q->len);
		while (acc >> 16) {
			acc = (acc & 0xffff) + (acc >> 16);
		}
		if (q->len % 2 != 0) {
			swapped = swapped ^ 0x1; // 1 - swapped;
			acc = (acc & 0xff << 8) | (acc & 0xff00 >> 8);
		}
	}

	if (swapped) {
		acc = ((acc & 0xff) << 8) | ((acc & 0xff00) >> 8);
	}
	return ( (u16_t) (~(acc & 0xffff)));
}

/**
 * These are reference implementations of the byte swapping functions.
 * Again with the aim of being simple, correct and fully portable.
 * Byte swapping is the second thing you would want to optimize. You will
 * need to port it to your architecture and in your cc.h:
 *
 * #define LWIP_PLATFORM_BYTESWAP 1
 * #define LWIP_PLATFORM_HTONS(x) <your_htons>
 * #define LWIP_PLATFORM_HTONL(x) <your_htonl>
 *
 * Note ntohs() and ntohl() are merely references to the htonx counterparts.
 */

#if (LWIP_PLATFORM_BYTESWAP == 0) && (BYTE_ORDER != BIG_ENDIAN)

/**
 * Convert an u16_t from host- to network byte order.
 *
 * @param n u16_t in host byte order
 * @return n in network byte order
 */
u16_t htons(u16_t n)
{
	return ((u16_t) (((n & 0xff) << 8) | ((n & 0xff00) >> 8)));
}

/**
 * Convert an u32_t from host- to network byte order.
 *
 * @param n u32_t in host byte order
 * @return n in network byte order
 */
u32_t htonl(u32_t n)
{
	return (((n & 0xff) << 24) |
	((n & 0xff00) << 8) |
	((n & 0xff0000UL) >> 8) |
	((n & 0xff000000UL) >> 24));
}

/**
 * Convert an u24_t from network- to host byte order.
 *
 * @param n u24_t in host byte order
 * @return n in network byte order
 */
u32_t ntohl_24(u32_t n) {
	return htonl(n << 8);
}

#endif /* (LWIP_PLATFORM_BYTESWAP == 0) && (BYTE_ORDER == LITTLE_ENDIAN) */
