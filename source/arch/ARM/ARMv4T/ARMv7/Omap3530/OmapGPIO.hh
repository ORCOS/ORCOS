/*
 * OmapGPIO.hh
 *
 *  Created on: 02.08.2013
 *      Author: dbaldin
 */

#ifndef OMAPGPIO_HH_
#define OMAPGPIO_HH_

#include "hal/CharacterDeviceDriver.hh"

/*!
 * \brief Simple GPIO Driver for the OMAP SOC
 *
 */
class OmapGPIO: public CharacterDeviceDriver {
private:

	int4 baseAddress;

public:
	OmapGPIO( T_OmapGPIO_Init *init);

	virtual ~OmapGPIO();

	/*!
	 * \brief Reads the GPIO state. Length must be 4 bytes for the 32 bit GPIO
	 *
	 */
	ErrorT readBytes( char *bytes, unint4 &length );


	/*!
	 * \brief Sets the GPIO output state. Length must be 4 bytes.
	 *
	 */
	ErrorT writeBytes( const char *bytes, unint4 length );


	/*!
	 * \brief Sets the direction of the GPIO pins. 0 = Output, 1 = Input
	 */
	ErrorT ioctl(int request, void* args);

};

#endif /* OMAPGPIO_HH_ */
