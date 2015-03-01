/*
 * random.h
 *
 *  Created on: 02.12.2014
 *      Author: Daniel
 */

#ifndef SOURCE_INC_RANDOM_H_
#define SOURCE_INC_RANDOM_H_

#ifdef __cpluspus
extern "C" {
#endif


unint4 xorshift128(void);

#define rand() xorshift128()

#ifdef __cpluspus
}
#endif

#endif /* SOURCE_INC_RANDOM_H_ */
