/*
 * AM335xGPIO.hh
 *
 *  Created on: 16.01.2015
 *   Copyright &  Author: dbaldin
 */

#ifndef AM335XGPIO_HH_
#define AM335XGPIO_HH_

#include "hal/CharacterDevice.hh"
#include "hal/GenericDeviceDriver.hh"

#define AM335X_GPIO_REVISION               0x0
#define AM335X_GPIO_SYSCONFIG              0x0

#define AM335X_GPIO_IRQSTATUS_SET_0        0x34
#define AM335X_GPIO_IRQSTATUS_SET_1        0x38

#define AM335X_GPIO_IRQSTATUS_CLR_0        0x3c
#define AM335X_GPIO_IRQSTATUS_CLR_1        0x40

#define AM335X_GPIO_CLEARDATAOUT           0x190
#define AM335X_GPIO_SETDATAOUT             0x194

#define AM335X_GPIO_FALLINGDETECT          0x14C
#define AM335X_GPIO_RISINGDETECT           0x148

#define AM335X_GPIO_OE                     0x134

#define AM335X_GPIO_DATA_IN                0x138 // get current pin state
#define AM335X_GPIO_DATA_OUT               0x13c // set pin output level

#define AM335X_GPIO_IRQSTATUS0             0x2c // core 0 interrupt line
#define AM335X_GPIO_IRQSTATUS1             0x30 // core 1 interrupt line



/*!
 * \brief Simple GPIO Driver for the AM335x SOC. May be compatible to
 * other SOCs.
 *
 */
class AM335xGPIO: public CharacterDevice {
private:
    /* The base address of this GPIO device*/
    int4 baseAddress;

    /* The GPIO number (1-6) of this device*/
    unint1 gpionum;

    unint4 irqStatus;

    unint4 irqNum;

public:
    /*****************************************************************************
     * Method: AM335xGPIO(T_AM335xGPIO_Init *init)
     *
     * @description
     *  Constructor called by board initialization
     *******************************************************************************/
    explicit AM335xGPIO(T_AM335xGPIO_Init *init);

    virtual ~AM335xGPIO();


    /*****************************************************************************
     * Method: readBytes(char *bytes, unint4 &length)
     *
     * @description
     *  Reads the GPIO state. Length must be 4 bytes for the 32 bit GPIO
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT readBytes(char *bytes, unint4 &length);

    /*****************************************************************************
     * Method: writeBytes(const char *bytes, unint4 length)
     *
     * @description
     *  Sets the GPIO output state. Length must be 4 bytes.
     * @params
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT writeBytes(const char *bytes, unint4 length);

    /*****************************************************************************
     * Method: handleIRQ()
     *
     * @description
     *  Handles IRQs generated by GPIO pins
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT handleIRQ();

    /*****************************************************************************
     * Method: probe()
     *
     * @description
     *  Checks if this device is valid.
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT probe() {
        if (gpionum == -1)
            return (cError);
        return (cOk);
    }


    /*****************************************************************************
     * Method: enableIRQ()
     *
     * @description
     * enables the hardware interrupts of this device
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT enableIRQ();

    /*****************************************************************************
     * Method: disableIRQ()
     *
     * @description
     * disables all interrupts of this device (does not clear them!)
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT disableIRQ();

    /*****************************************************************************
     * Method: clearIRQ()
     *
     * @description
     * clears all interrupts of this device
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT clearIRQ();


    /*****************************************************************************
     * Method: ioctl(int request, void* args)
     *
     * @description
     *  IOCTL commands to:
     *   Sets the direction of the GPIO pins. 0 = Output, 1 = Input
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT ioctl(int request, void* args);
};

#endif /* OMAPGPIO_HH_ */