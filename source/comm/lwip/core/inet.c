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

/** Like the name says... */
#if LWIP_PLATFORM_BYTESWAP && (BYTE_ORDER == LITTLE_ENDIAN)
/* little endian and PLATFORM_BYTESWAP defined */
#define SWAP_BYTES_IN_WORD(w) LWIP_PLATFORM_HTONS(w)
#else
/* can't use htons on big endian (or PLATFORM_BYTESWAP not defined)... */
#define SWAP_BYTES_IN_WORD(w) ((w & 0xff) << 8) | ((w & 0xff00) >> 8)
#endif

/** Split an u32_t in two u16_ts and add them up */
#define FOLD_U32T(u)          ((u >> 16) + (u & 0x0000ffffUL))

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

#if 1
u32_t chksum(u16_t *data, u16_t len) {
	int sum = 0;

	// check alignment
	/*if ((u16_t) data & 0x1) {

		data = ((u16_t) data + 1);
	}*/

	while (len > 1) {
		sum += *data++;
		len -= 2;
	}

	if (len > 0) {
		sum += * (unsigned char *) data;
	}

	return sum;

		/*  Fold 32-bit sum to 16 bits */
	/*while (sum>>16)
		sum = (sum & 0xffff) + (sum >> 16);

	return (~sum);*/
}
#endif

#if 0
static u16_t chksum(void *dataptr, u16_t len)
{
  u8_t *pb = (u8_t *)dataptr;
  u16_t *ps, t = 0;
  u32_t sum = 0;
  int odd = ((mem_ptr_t)pb & 1);

  /* Get aligned to u16_t */
  if (odd && len > 0) {
    ((u8_t *)&t)[1] = *pb++;
    len--;
  }

  /* Add the bulk of the data */
  ps = (u16_t *)(void *)pb;
  while (len > 1) {
    sum += *ps++;
    len -= 2;
  }

  /* Consume left-over byte, if any */
  if (len > 0) {
    ((u8_t *)&t)[0] = *(u8_t *)ps;
  }

  /* Add end bytes */
  sum += t;

  /* Fold 32-bit sum to 16 bits
     calling this twice is propably faster than if statements... */
  sum = FOLD_U32T(sum);
  sum = FOLD_U32T(sum);

  /* Swap if alignment was odd */
  if (odd) {
    sum = SWAP_BYTES_IN_WORD(sum);
  }

  return (u16_t)sum;
}
#endif

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
			swapped = 1 - swapped;
			acc = ((acc & 0xff) << 8) | ((acc & 0xff00) >> 8);
		}
	}

	if (swapped) {
		acc = ((acc & 0xff) << 8) | ((acc & 0xff00) >> 8);
	}

	int m = 0;
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
	return ~(acc & 0xffff);
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
	return ~(sum & 0xffff);
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
			swapped = 1 - swapped;
			acc = (acc & 0xff << 8) | (acc & 0xff00 >> 8);
		}
	}

	if (swapped) {
		acc = ((acc & 0xff) << 8) | ((acc & 0xff00) >> 8);
	}
	return ~(acc & 0xffff);
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
	return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

/**
 * Convert an u32_t from host- to network byte order.
 *
 * @param n u32_t in host byte order
 * @return n in network byte order
 */
u32_t htonl(u32_t n)
{
	return ((n & 0xff) << 24) |
	((n & 0xff00) << 8) |
	((n & 0xff0000UL) >> 8) |
	((n & 0xff000000UL) >> 24);
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
