/*
 * OmapGPIO.cc
 *
 *  Created on: 02.08.2013
 *    Copyright &  Author: dbaldin
 */

#include "OmapGPIO.hh"
#include "inc/memio.h"
#include "kernel/Kernel.hh"
#include "syscalls/syscalls.hh"

// Keep consistent with sys/gpio.h defines
#define IOCTL_GPIO_SET_DIR 0x0
#define IOCTL_GPIO_GET_DIR 0x1
#define IOCTL_GPIO_ENABLE_IRQ 0x2
#define IOCTL_GPIO_GET_IRQ_STATUS 0x3
#define IOCTL_GPIO_IRQ_RISING_EDGE 0x4
#define IOCTL_GPIO_IRQ_FALLING_EDGE 0x5

extern Kernel* theOS;

/* Base Address of the OMAP GPIOs:


 GPIO1    0x48310000,
 GPIO2    0x49050000,
 GPIO3    0x49052000,
 GPIO4    0x49054000,
 GPIO5    0x49056000,
 GPIO6    0x49058000
 */

OmapGPIO::OmapGPIO(T_OmapGPIO_Init *init) :
        CharacterDevice(true, init->Name) {
    this->baseAddress = init->Address;

    LOG(ARCH, INFO, "OMAPGPIO: creating '%s' [0x%x]", init->Name, init->Address);

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

    if (gpionum == -1) {
        LOG(ARCH, ERROR, "OMAPGPIO invalid base address: %x", baseAddress);
        return;
    }

    /* register the interrupt handler for the GPIO... no need to use a thread here */
    theOS->getInterruptManager()->registerIRQ(29 + gpionum, this, init->Priority, IRQ_NOTHREAD);

    if (gpionum > 1) {
        // enable functional clocks of this gpio module
        unint4 val = INW(0x48005000);
        val |= (1 << (11 + gpionum));
        OUTW(0x48005000, val);

        // enable interface clocks of this gpio module
        val = INW(0x48005010);
        val |= (1 << (11 + gpionum));
        OUTW(0x48005010, val);
    } else {
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
    OUTW(this->baseAddress + GPIO_OE, (unint4) init->DirectionBits);

    unint4 val = init->ValueBits;

    // set data out for all 1 bits
    OUTW(this->baseAddress + GPIO_SETDATAOUT, val);
    // clear data out for all 0 bits
    OUTW(this->baseAddress + GPIO_CLEARDATAOUT, ~val);
}

OmapGPIO::~OmapGPIO() {
}


/*****************************************************************************
 * Method: OmapGPIO::readBytes(char *bytes, unint4 &length)
 *
 * @description
 *  Reads the GPIO state. Length must be 4 bytes for the 32 bit GPIO
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT OmapGPIO::readBytes(char* bytes, unint4& length) {
    if (length < 4)
        return (cError);
    // check alignment
    if ((((unint4) bytes) & 0x3) != 0) {
        return (cWrongAlignment);
    }

    OUTW(bytes, INW(this->baseAddress + 0x38));
    length = 4;
    return (cOk);
}

/*****************************************************************************
 * Method: OmapGPIO::writeBytes(const char *bytes, unint4 length)
 *
 * @description
 *  Sets the GPIO output state. Length must be 4 bytes.
 * @params
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT OmapGPIO::writeBytes(const char* bytes, unint4 length) {
    if (length != 4)
        return (cError);
    // check alignment
    if ((((unint4) bytes) & 0x3) != 0) {
        return (cWrongAlignment);
    }

    unint4 val = *((const unint4*) bytes);

    // set data out for all 1 bits
    OUTW(this->baseAddress + GPIO_SETDATAOUT, val);
    // clear data out for all 0 bits
    OUTW(this->baseAddress + GPIO_CLEARDATAOUT, ~val);

    return (cOk );
}


/*****************************************************************************
 * Method: OmapGPIO::handleIRQ()
 *
 * @description
 *  Handles IRQs generated by GPIO pins
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT OmapGPIO::handleIRQ(int irq) {
    /* Nothing to do here.
     * User Space thread may be doing something.
     */
    /* accumulate the irq bits until read by user thread */
    irqStatus |= INW(this->baseAddress + GPIO_IRQSTATUS1);
    return (cOk);
}

/*****************************************************************************
 * Method: OmapGPIO::enableIRQ()
 *
 * @description
 * enables the hardware interrupts of this device
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT OmapGPIO::enableIRQ(int irq) {
    // TODO ? board specific stuff in next line?
    OUTW(MPU_INTCPS_ILR(36 + gpionum), (INW(MPU_INTCPS_ILR(36 + gpionum)) & ~0x1));// normal irq
    theOS->getBoard()->getInterruptController()->setIRQPriority(36 + gpionum, 1);  /* low priority! */
    theOS->getBoard()->getInterruptController()->unmaskIRQ(36 + gpionum);
    return (cOk);
}

/*****************************************************************************
 * Method: OmapGPIO::disableIRQ()
 *
 * @description
 * disables all interrupts of this device (does not clear them!)
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT OmapGPIO::disableIRQ(int irq) {
    theOS->getBoard()->getInterruptController()->maskIRQ(36 + gpionum);
    return (cOk);
}


/*****************************************************************************
 * Method: OmapGPIO::clearIRQ()
 *
 * @description
 * clears all interrupts of this device
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT OmapGPIO::clearIRQ(int irq) {
    /* clear all bits */
    OUTW(this->baseAddress + GPIO_IRQSTATUS1, 0xffffffff);
    return (cNotImplemented );
}

/*****************************************************************************
 * Method: OmapGPIO::ioctl(int request, void* args)
 *
 * @description
 *  IOCTL commands to:
 *   Sets the direction of the GPIO pins. 0 = Output, 1 = Input
 *
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT OmapGPIO::ioctl(int request, void* args) {
    // handle the io control request
    if (request == IOCTL_GPIO_SET_DIR) {
        OUTW(this->baseAddress + GPIO_OE, (unint4) args);
        return (cOk);
    }
    if (request == IOCTL_GPIO_GET_DIR) {
        OUTW(args, INW(this->baseAddress + GPIO_OE));
        return (cOk);
    }
    if (request == IOCTL_GPIO_ENABLE_IRQ) {
        OUTW(this->baseAddress + GPIO_SETIRQENABLE1, (unint4) args);

        if (args != 0) {
            enableIRQ(0);
        } else {
            disableIRQ(0);
        }
        return (cOk);
    }
    if (request == IOCTL_GPIO_GET_IRQ_STATUS) {
        VALIDATE_IN_PROCESS(args);
        OUTW(args, irqStatus);
        irqStatus = 0;
        return (cOk);
    }
    if (request == IOCTL_GPIO_IRQ_RISING_EDGE) {
        OUTW(this->baseAddress + GPIO_RISINGDETECT, (unint4) args);
        return (cOk);
    }
    if (request == IOCTL_GPIO_IRQ_FALLING_EDGE) {
        OUTW(this->baseAddress + GPIO_FALLINGDETECT, (unint4) args);
        return (cOk);
    }

    return (cInvalidArgument);
}
