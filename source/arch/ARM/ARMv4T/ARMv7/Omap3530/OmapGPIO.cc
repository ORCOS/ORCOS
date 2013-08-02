/*
 * OmapGPIO.cc
 *
 *  Created on: 02.08.2013
 *      Author: dbaldin
 */

#include "OmapGPIO.hh"
#include "inc/memio.h"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

/* Base Address of the OMAP GPIOs:


GPIO1	0x48310000,
GPIO2	0x49050000,
GPIO3	0x49052000,
GPIO4	0x49054000,
GPIO5	0x49056000,
GPIO6	0x49058000

*/

OmapGPIO::OmapGPIO( const char *name, int4 a) : CharacterDeviceDriver(true,name)  {
	this->baseAddress = a;

    LOG(ARCH,INFO,(ARCH,INFO,"OMAPGPIO: creating '%s' [0x%x]",name,a) );

    //TODO: we could optimize this to enable only the clocks of the used gpios
    // we would need a lookup table for the addresses and set the correct bit

    // enable functional clocks of all gpios
    unint4 val = INW(0x48005000);
    val |= 0x3E800;
 	OUTW(0x48005000,val);

 	// enable interface clocks of all gpios
    val = INW(0x48005010);
    val |= 0x3E800;

 	OUTW(0x48005010,val);


}

OmapGPIO::~OmapGPIO() {

}

ErrorT OmapGPIO::readBytes(char* bytes, unint4& length) {
	if (length < 4) return cError;
	// check alignment
	if ( ( ((unint4)bytes) & 0x3) != 0) return cWrongAlignment;

	 * ((unint4*) bytes) = INW(this->baseAddress + 0x38);

	length = 4;
	return cOk;

}

ErrorT OmapGPIO::writeBytes(const char* bytes, unint4 length) {
	if (length != 4) return cError;
	// check alignment
	if ( ( ((unint4)bytes) & 0x3) != 0) return cWrongAlignment;

	unint4 val = * ((unint4*) bytes);

	// set data out for all 1 bits
	OUTW(this->baseAddress + 0x94, val);
	// clear data out for all 0 bits
	OUTW(this->baseAddress + 0x94, ~val);

	return cOk;

}

ErrorT OmapGPIO::setDirection(unint4 dirBits) {

	OUTW(this->baseAddress + 0x34, dirBits);
	return cOk;
}
