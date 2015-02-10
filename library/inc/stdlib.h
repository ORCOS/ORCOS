/*
 * stdlib.h
 *
 *  Created on: 08.02.2015
 *      Author: Daniel
 */

#ifndef LIBRARY_INC_STDLIB_H_
#define LIBRARY_INC_STDLIB_H_

#ifdef __cplusplus
extern "C" {
#endif

int atoi(const char *p);
int abs(int i);

unsigned int xorshift128(void);

void srand(unsigned int seed);

#ifdef __cplusplus
}
#endif

#define random() xorshift128()
#define rand() xorshift128()

#endif /* LIBRARY_INC_STDLIB_H_ */
