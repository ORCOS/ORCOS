/*
 * random.c
 *
 *  Created on: 02.12.2014
 *      Author: Daniel
 */


static unsigned int x, y, z, w;

unsigned int xorshift128(void) {
    unsigned int t = x ^ (x << 11);
    x = y; y = z; z = w;
    w = w ^ (w >> 19) ^ t ^ (t >> 8);
    return (w);
}



void srand(unsigned int seed) {
    x = seed;
    y = (seed ^ 123451) | 0xa2;
    z = ((seed ^ x) ^ y) + 17;
    w = (seed ^ ((x|y)&z)) ^ 0x781112a3;
}

