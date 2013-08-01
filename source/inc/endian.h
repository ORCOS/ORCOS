/*
 * endian.h
 *
 *  Created on: 10.07.2013
 *      Author: dbaldin
 */

#ifndef ENDIAN_H_
#define ENDIAN_H_

extern "C" {

unint4 cputole32(unint4 n);

unint2 cputole16(unint2 n);

unint4 cputobe32(unint4 n);

unint2 cputobe16(unint2 n);

}

#define ___swab32(x) \
	((unint4)( \
		(((unint4)(x) & (unint4)0x000000ffUL) << 24) | \
		(((unint4)(x) & (unint4)0x0000ff00UL) <<  8) | \
		(((unint4)(x) & (unint4)0x00ff0000UL) >>  8) | \
		(((unint4)(x) & (unint4)0xff000000UL) >> 24) ))


#endif /* ENDIAN_H_ */
