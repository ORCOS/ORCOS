/*
 * BeagleBoardDSS.hh
 *
 *  Created on: 07.08.2012
 *      Author: danielb
 */

#ifndef BEAGLEBOARDDSS_HH_
#define BEAGLEBOARDDSS_HH_

#include <types.hh>
#include <hal/CharacterDeviceDriver.hh>
#include "filesystem/SharedMemResource.hh"

// CharacterDevice Driver for support as the standard output device
// console
class BeagleBoardDSS: public CharacterDeviceDriver {

private:

	SharedMemResource* framebuffer;

public:
	BeagleBoardDSS(T_BeagleBoardDSS_Init *init);

	~BeagleBoardDSS();

	void init();

	ErrorT writeByte (char byte);

	ErrorT writeBytes(const char *bytes, unint4 length);
};


#endif /* BEAGLEBOARDDSS_HH_ */
