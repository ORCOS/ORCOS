/*
 * Omap3530SPI.cc
 *
 *  Created on: 24.02.2014
 *   Copyright &  Author: dbaldin
 */

#include "Omap3530SPI.hh"
#include "inc/types.hh"
#include "kernel/Kernel.hh"
#include "inc/sys/spi.h"

extern Kernel* theOS;

#define CM_ICLKEN1_CORE 0x48004A10

Omap3530SPI::Omap3530SPI(T_Omap3530SPI_Init *init) :
        CharacterDevice(true, init->Name) {
    this->base = init->Address;

    // set base settings and soft reset
    OUTW(base+ MCSPI_SYSCONFIG, 0x303);
    // wait for soft reset
    while (INW(base+ MCSPI_SYSSTATUS) & 0x1) {
    }

    // set to master mode multi channel
    OUTW(base+ MCSPI_MODULCTRL, 0x0);

    // disable channel 0
    OUTW(base+ MCSPI_CHxCTRL(0), 0x0);

    OUTW(base+ MCSPI_IRQENABLE, 0x0);

    OUTW(base+ MCSPI_IRQSTATUS, 0x0001777F);

    OUTW(base+ MCSPI_CHxCONF(0), 0x4);
}

Omap3530SPI::~Omap3530SPI() {
}

/*****************************************************************************
 * Method: Omap3530SPI::readBytes(char *bytes, unint4 &length)
 *
 * @description
 *
 *******************************************************************************/
ErrorT Omap3530SPI::readBytes(char* bytes, unint4& length) {
    int channel = bytes[0];

    if (INW(base + MCSPI_IRQSTATUS) != 0) {
        // data available
        int data = INW(base+ MCSPI_RX(channel));
        if (length == 1)
            bytes[0] = data & 0xff;
        if (length == 2) {
            bytes[0] = data & 0xff;
            bytes[1] = (data >> 8) & 0xff;
        }
        if (length == 3) {
            bytes[0] = data & 0xff;
            bytes[1] = (data >> 8) & 0xff;
            bytes[2] = (data >> 16) & 0xff;
        }
        if (length == 4) {
            bytes[0] = data & 0xff;
            bytes[1] = (data >> 8) & 0xff;
            bytes[2] = (data >> 16) & 0xff;
            bytes[3] = (data >> 24) & 0xff;
        }

        OUTW(base+ MCSPI_IRQSTATUS, -1);
        return (cOk);
    } else {
        return (cError);
    }
}

/*****************************************************************************
 * Method: Omap3530SPI::writeBytes(const char *bytes, unint4 length)
 *
 * @description
 *
 *******************************************************************************/
ErrorT Omap3530SPI::writeBytes(const char* bytes, unint4 length) {
    if (length < 2)
        return (cError );

    /* First byte is the channel we want to communicate with.
     * Every device connected to the bus is associated with one channel.
     * The number of channels depends on the physical connections the
     * controller offers.
     **/
    int channel = bytes[0];
    if (channel > 4)
        return (cInvalidArgument );

    OUTW(base+ MCSPI_CHxCTRL(channel), 1);
    //OUTW(base+ MCSPI_CHxCTRL(0),1);
    OUTW(base+ MCSPI_IRQSTATUS, -1);

    int data = 0;
    if (length == 2)
        data = bytes[1];
    if (length == 3) {
        // 2 bytes transfer
        data = bytes[2] | (bytes[1] << 8);
    }
    if (length == 4) {
        // 3 bytes transfer
        data = bytes[3] | (bytes[2] << 8) | (bytes[1] << 16);
    }
    if (length == 5) {
        data = bytes[4] | (bytes[3] << 8) | (bytes[2] << 16) | (bytes[1] << 24);
    }

    OUTW(base+ MCSPI_TX(channel), data);

    while ((INW(base + MCSPI_CHxSTAT(channel)) & (1 << 2)) == 0) {}

    OUTW(base+ MCSPI_IRQSTATUS, -1);

    OUTW(base+ MCSPI_CHxCTRL(channel), 0);

    char* retbytes = const_cast<char*>(bytes);

    // get RX data
    int data2 = INW(base+ MCSPI_RX(channel));

    LOG(ARCH, TRACE, "SPI RX: %x, STATUS %x", data2, INW(base+ MCSPI_CHxSTAT(channel)));
    if (length == 1)
        retbytes[0] = data2 & 0xff;
    if (length == 2) {
        retbytes[0] = data2 & 0xff;
        retbytes[1] = (data2 >> 8) & 0xff;
    }
    if (length == 3) {
        retbytes[0] = data2 & 0xff;
        retbytes[1] = (data2 >> 8) & 0xff;
        retbytes[2] = (data2 >> 16) & 0xff;
    }
    if (length == 4) {
        retbytes[0] = data2 & 0xff;
        retbytes[1] = (data2 >> 8) & 0xff;
        retbytes[2] = (data2 >> 16) & 0xff;
        retbytes[3] = (data2 >> 24) & 0xff;
    }

    return (cOk);
}

/*****************************************************************************
 * Method: Omap3530SPI::ioctl(int request, void* args)
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
ErrorT Omap3530SPI::ioctl(int request, void* args) {
    int channel = -1;
    if (request == SPI_CONFIGURE_CHANNEL0)
        channel = 0;
    if (request == SPI_CONFIGURE_CHANNEL1)
        channel = 1;
    if (request == SPI_CONFIGURE_CHANNEL2)
        channel = 2;
    if (request == SPI_CONFIGURE_CHANNEL3)
        channel = 3;

    if (channel < 0)
        return (cInvalidArgument );

    /* configure channel*/
    OUTW(base + MCSPI_CHxCONF(channel), ((unint4) args & ~(1 << 18)) | (1 << 16));
    LOG(ARCH, TRACE, "Channel %x CONF: %x", channel, INW(base+ MCSPI_CHxCONF(channel)));
    return (cOk );
}
