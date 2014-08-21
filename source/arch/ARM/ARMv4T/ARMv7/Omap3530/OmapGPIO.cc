/*
 * OmapGPIO.cc
 *
 *  Created on: 02.08.2013
 *      Author: dbaldin
 */

#include "OmapGPIO.hh"
#include "inc/memio.h"
#include "kernel/Kernel.hh"

// Keep consistent with sys/gpio.h defines
#define IOCTL_GPIO_SET_DIR 0x0
#define IOCTL_GPIO_GET_DIR 0x1
#define IOCTL_GPIO_ENABLE_IRQ 0x2
#define IOCTL_GPIO_GET_IRQ_STATUS 0x3
#define IOCTL_GPIO_IRQ_RISING_EDGE 0x4
#define IOCTL_GPIO_IRQ_FALLING_EDGE 0x5

extern Kernel* theOS;

/* Base Address of the OMAP GPIOs:


 GPIO1	0x48310000,
 GPIO2	0x49050000,
 GPIO3	0x49052000,
 GPIO4	0x49054000,
 GPIO5	0x49056000,
 GPIO6	0x49058000

 */

OmapGPIO::OmapGPIO(T_OmapGPIO_Init *init) :
        CharacterDevice(true, init->Name) {
    this->baseAddress = init->Address;

    LOG(ARCH, INFO, "OMAPGPIO: creating '%s' [0x%x]",init->Name,init->Address);

    gpionum = -1;

    if (baseAddress == 0x48310000)
        gpionum = 1;
    if (baseAddress == 0x49050000)
        gpionum = 2;
    if (baseAddress == 0x49052000)
        gpionum = 3;
    if (baseAddress == 0x49054000)
        gpionum = 4;
    if (baseAddress == 0x49056000)
        gpionum = 5;
    if (baseAddress == 0x49058000)
        gpionum = 6;

    /*
     * GPIO 5 + 6 are connected internally to provide
     * 5v to the TPM module. Be sure to not disable the output
     * pins as USB will not work any more then.
     */

    if (gpionum == -1)
    {
        LOG(ARCH, ERROR, "OMAPGPIO invalid base address: %x",baseAddress);
        return;
    }

    theOS->getInterruptManager()->registerIRQ(36 + gpionum, this, init->Priority);

    if (gpionum > 1)
    {
        // enable functional clocks of this gpio module
        unint4 val = INW(0x48005000);
        val |= (1 << (11 + gpionum));
        OUTW(0x48005000, val);

        // enable interface clocks of this gpio module
        val = INW(0x48005010);
        val |= (1 << (11 + gpionum));
        OUTW(0x48005010, val);
    }
    else
    {
        // we must be gpio 1
        unint4 val = INW(0x48004c00);
        val |= (1 << 3);
        OUTW(0x48004c00, val);

        // enable interface clocks of this gpio module
        val = INW(0x48004c10);
        val |= (1 << 3);
        OUTW(0x48004c10, val);
    }

    irqStatus = 0;

    /* clear all irq enable bits */
    OUTW(this->baseAddress + GPIO_CLEARIRQENABLE1, 0xffffffff);

    // set direction
    OUTW(this->baseAddress + GPIO_OE, (unint4 ) init->DirectionBits);

    unint4 val = init->ValueBits;

    // set data out for all 1 bits
    OUTW(this->baseAddress + GPIO_SETDATAOUT, val);
    // clear data out for all 0 bits
    OUTW(this->baseAddress + GPIO_CLEARDATAOUT, ~val);

}

OmapGPIO::~OmapGPIO() {

}

ErrorT OmapGPIO::readBytes(char* bytes, unint4& length) {
    if (length < 4)
        return (cError );
    // check alignment
    if ((((unint4) bytes) & 0x3) != 0)
    {
        return (cWrongAlignment );
    }

    *((unint4*) bytes) = INW(this->baseAddress + 0x38);

    length = 4;
    return (cOk );

}

ErrorT OmapGPIO::writeBytes(const char* bytes, unint4 length) {
    if (length != 4)
        return (cError );
    // check alignment
    if ((((unint4) bytes) & 0x3) != 0)
    {
        return (cWrongAlignment );
    }

    unint4 val = *((const unint4*) bytes);

    // set data out for all 1 bits
    OUTW(this->baseAddress + GPIO_SETDATAOUT, val);
    // clear data out for all 0 bits
    OUTW(this->baseAddress + GPIO_CLEARDATAOUT, ~val);

    return (cOk );
}

ErrorT OmapGPIO::handleIRQ() {
    /* Nothing to do here.
     * User Space thread may be doing something.
     */
    /* accumulate the irq bits until read by user thread */
    irqStatus |= INW(this->baseAddress + GPIO_IRQSTATUS1);
    return (cOk );
}

ErrorT OmapGPIO::enableIRQ() {

    OUTW(MPU_INTCPS_ILR(36 + gpionum), (INW(MPU_INTCPS_ILR(36 + gpionum)) & ~ 0x1 ));  // normal irq
    OUTW(MPU_INTCPS_ILR(36 + gpionum), (INW(MPU_INTCPS_ILR(36 + gpionum)) & ~ 0x1C ));  // priority (0 is highest)
    /* unmask the interrupt */
    OUTW(MPU_INTCPS_MIR_CLEAR(1), (1 << (4 + gpionum)));  // enable interrupt: (gpion int no. 36 + gpionum: ((36 + gpionum) mod 32): position of bit

    return (cNotImplemented );
}

//! disables all interrupts of this device (does not clear them!)
ErrorT OmapGPIO::disableIRQ() {

    OUTW(MPU_INTCPS_MIR_CLEAR(1), INW(MPU_INTCPS_MIR_CLEAR(1)) & ~(1 << (4+gpionum)));  // mask interrupt: (gpion int no. 36 + gpionum: ((36 + gpionum) mod 32): position of bit

    return (cNotImplemented );
}

//! clears all interrupts of this device
ErrorT OmapGPIO::clearIRQ() {
    /* clear all bits */
    OUTW(this->baseAddress + GPIO_IRQSTATUS1, 0xffffffff);
    return (cNotImplemented );
}

ErrorT OmapGPIO::ioctl(int request, void* args) {
    // handle the io control request
    if (request == IOCTL_GPIO_SET_DIR)
    {
        OUTW(this->baseAddress + GPIO_OE, (unint4 ) args);
        return (cOk );
    }
    if (request == IOCTL_GPIO_GET_DIR)
    {
        (*(unint4*) args) = INW(this->baseAddress + GPIO_OE);
        return (cOk );
    }
    if (request == IOCTL_GPIO_ENABLE_IRQ)
    {
        OUTW(this->baseAddress + GPIO_SETIRQENABLE1, (unint4 ) args);

        if (args != 0)
            enableIRQ();
        else
            disableIRQ();
        return (cOk );
    }
    if (request == IOCTL_GPIO_GET_IRQ_STATUS)
    {
        (*(unint4*) args) = irqStatus;
        irqStatus = 0;
        return (cOk );
    }
    if (request == IOCTL_GPIO_IRQ_RISING_EDGE)
    {
        OUTW(this->baseAddress + GPIO_RISINGDETECT, (unint4 ) args);
        return (cOk );
    }
    if (request == IOCTL_GPIO_IRQ_FALLING_EDGE)
    {
        OUTW(this->baseAddress + GPIO_FALLINGDETECT, (unint4 ) args);
        return (cOk );
    }

    return (cInvalidArgument );
}
