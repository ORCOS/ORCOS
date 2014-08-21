/*
 * time.h
 *
 *  Created on: 22.06.2014
 *      Author: Daniel
 */

#ifndef TIME_H_
#define TIME_H_

/*
 * Converts the datetime given as seconds since epoch (1. Jan. 1970)
 * into the time_t struct given as parameter.
 */
void time2date(time_t* pTm, unint4 seconds);


#endif /* TIME_H_ */
