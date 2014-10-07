/*
 * USART0.hh
 *
 *  Created on: 04.05.2014
 *      Author: Daniel
 */

#ifndef USART0_HH_
#define USART0_HH_

#include <error.hh>
#include <types.hh>
#include "SCLConfig.hh"

class USART0 /*: public CharacterDeviceDriver*/ {
public:

	USART0();

    ~USART0();

	// interface to meet the CharacterDeviceDriver
	// ErrorT readByte  (char* byte);

	 //ErrorT writeByte (char byte);

	 //ErrorT readBytes (char *bytes, unint4 &length);

	 //ErrorT writeBytes(const char *bytes, unint4 length);
};

#endif /* USART0_HH_ */
