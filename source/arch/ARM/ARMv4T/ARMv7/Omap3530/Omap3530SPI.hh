/*
 * Omap3530SPI.hh
 *
 *  Created on: 24.02.2014
 *     Copyright &  Author: dbaldin
 */

#ifndef OMAP3530SPI_HH_
#define OMAP3530SPI_HH_

#include "hal/CharacterDevice.hh"
#include "hal/GenericDeviceDriver.hh"

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

class Omap3530SPI: public CharacterDevice {
private:
    unint4 base;

public:
    /*****************************************************************************
     * Method: Omap3530SPI(T_Omap3530SPI_Init *init)
     *
     * @description
     *  Constructor
     *******************************************************************************/
    explicit Omap3530SPI(T_Omap3530SPI_Init *init);

    ~Omap3530SPI();

    /*****************************************************************************
     * Method: readByte(char* p_byte)
     *
     * @description
     *  Reading single byte not supported due to SPI protocol
     *******************************************************************************/
    ErrorT readByte(char* p_byte) { return (cError); }

    /*****************************************************************************
     * Method: writeByte(char c_byte)
     *
     * @description
     *  Writing single byte not supported due to SPI protocol
     *******************************************************************************/
    ErrorT writeByte(char c_byte) { return (cError); }

    /*****************************************************************************
     * Method: readBytes(char *bytes, unint4 &length)
     *
     * @description
     *
     *******************************************************************************/
    ErrorT readBytes(char *bytes, unint4 &length);

    /*****************************************************************************
     * Method: writeBytes(const char *bytes, unint4 length)
     *
     * @description
     *
     *******************************************************************************/
    ErrorT writeBytes(const char *bytes, unint4 length);

    /*****************************************************************************
     * Method: ioctl(int request, void* args)
     *
     * @description
     *  Provides SPI configuration to user space
     *  using ioctl syscalls.
     *
     * @params
     *  request     Request type
     *  args        argument of request
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT ioctl(int request, void* args);
};

#endif /* OMAP3530SPI_HH_ */
