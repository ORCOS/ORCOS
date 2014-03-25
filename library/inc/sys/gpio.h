/*
 * gpio.h
 *
 *  I/O Control commands for generic GPIO handling.
 *
 *  IRQ Generation for GPIO pins allows user threads
 *  to wait for external IRQs raised in GPIO lines using
 *  the irq_wait syscall.
 *
 *  Be aware: GPIO drivers generally do nothing on IRQ except clearing the IRQ flag.
 *  Thus, if you do not need a specific GPIO IRQ do not enable the GPIO IRQ as it will take
 *  useless processor time.
 *
 *  Created on: 14.08.2013
 *      Author: Daniel Baldin
 */

#ifndef GPIO_H_
#define GPIO_H_

/*
* IOCTL request to set the direction of a GPIO bank.
* Argument is a 32 bit word.
* Direction is set per bit:  1 == input, 0 == output
*
* Example call:
*
* ioctl(IOCTL_GPIO_SET_DIR,0x3)
*
* sets the first 2 gpio pins to input, all other to output
*
*/
#define IOCTL_GPIO_SET_DIR 0x0

/*
* IOCTL request to get the direction of a GPIO bank.
* Argument is a pointer to a 32 bit variable.
* Direction per bit:  1 == input, 0 == output
*
* Example call:
*
* unint4 dir;
* ioctl(IOCTL_GPIO_GET_DIR,&dir)
*
*/
#define IOCTL_GPIO_GET_DIR 0x1

/*
*  IOCTL request to allow or disable the IRQ generation
*  of a PGIO line.
*
*  Behavior on enabling IRQ on output PGIO lines is architecture dependent, however
*  should fail for most implementations
*
*  Argument is a 32 bit word.
*  IRQ enable is set per bit:  1 == IRQ enabled  0 == IRQ disabled
*
*  Example call:
*
*  unint4 irq = 0b 1101;
*  ioctl(IOCTL_GPIO_IRQ_FALLING_EDGE,&irq); // allow irq generation for gpio 0, 2 and 3
*/
#define IOCTL_GPIO_ENABLE_IRQ 0x2

/*
* IOCTL request to get the irq status of a GPIO bank since last read access.
* Argument is a pointer to a 32 bit variable.
* IRQ status per bit:  1 == IRQ was risen, 0 == no irq,
*
* After reading the irq status vector all irq statuses will be reset to 0.
* Example call:
*
* unint4 irqstatus;
* ioctl(IOCTL_GPIO_GET_IRQ_STATUS,&irqstatus)
*/
#define IOCTL_GPIO_GET_IRQ_STATUS 0x3

/*
*  IOCTL request to allow interrupt generation for an
*  gpio line on rising edge detect.
*  Argument is a pointer to a 32 bit variable.
*  IRQ Condition per bit:  1 == IRQ on rising edge, 0 == unchanged
*
*  Depending on the PGIO device it may or may not be possible to set
*  both edge detection conditions at the same time.
*
*  Example call:
*
*  unint4 irq = 0b 1101;
*  ioctl(IOCTL_GPIO_IRQ_RISING_EDGE,&irq); // allow irq generation for gpio 0, 2 and 3 on rising edge
*/
#define IOCTL_GPIO_IRQ_RISING_EDGE 0x4

/*
*  IOCTL request to allow interrupt generation for an
*  gpio line on falling edge detect.
*  Argument is a pointer to a 32 bit variable.
*  IRQ Condition per bit:  1 == IRQ on falling edge, 0 == unchanged
*
*  Depending on the PGIO device it may or may not be possible to set
*  both edge detection conditions at the same time.
*
*  Example call:
*
*  unint4 irq = 0b 1101;
*  ioctl(IOCTL_GPIO_IRQ_FALLING_EDGE,&irq); // allow irq generation for gpio 0, 2 and 3 on falling edge
*/
#define IOCTL_GPIO_IRQ_FALLING_EDGE 0x5

#endif /* GPIO_H_ */
