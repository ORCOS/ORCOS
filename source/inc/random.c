/*
 * random.c
 *
 *  Created on: 02.12.2014
 *      Author: Daniel
 */

#include "inc/types.hh"

static unint4 x, y, z, w;

unint4 xorshift128(void) {
    unint4 t = x ^ (x << 11);
    x = y; y = z; z = w;
    w = w ^ (w >> 19) ^ t ^ (t >> 8);
    return (w);
}


