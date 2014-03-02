/*
 * Omap3530SPI.hh
 *
 *  Created on: 24.02.2014
 *      Author: dbaldin
 */

#ifndef OMAP3530SPI_HH_
#define OMAP3530SPI_HH_

#include "hal/CharacterDeviceDriver.hh"

#define MCSPI_REVISION  0x0
#define MCSPI_SYSCONFIG 0x10
#define MCSPI_SYSSTATUS 0x14
#define MCSPI_IRQSTATUS 0x18
#define MCSPI_IRQENABLE 0x1c
#define MCSPI_MODULCTRL 0x28
#define MCSPI_CHxCONF(x) (0x2c + 0x14 *x)
#define MCSPI_CHxSTAT(x) (0x30 + 0x14 *x)
#define MCSPI_CHxCTRL(x) (0x34 + 0x14 *x)
#define MCSPI_TX(x) (0x38 + 0x14 *x)
#define MCSPI_RX(x) (0x3c + 0x14 *x)

#define MCSPI1 0x48098000
#define MCSPI2 0x4809A000
#define MCSPI3 0x480B8000
#define MCSPI4 0x480BA000


class Omap3530SPI: public CharacterDeviceDriver {
private:
	unint4 base;

public:
	Omap3530SPI(T_Omap3530SPI_Init *init);
	virtual ~Omap3530SPI();

	ErrorT readByte( char* p_byte );


	ErrorT writeByte( char c_byte );


	ErrorT readBytes( char *bytes, unint4 &length );


    ErrorT writeBytes( const char *bytes, unint4 length );


	ErrorT ioctl(int request, void* args);
};

#endif /* OMAP3530SPI_HH_ */
