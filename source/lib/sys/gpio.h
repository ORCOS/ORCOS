/*
 * gpio.h
 *
 *  Created on: 14.08.2013
 *      Author: dbaldin
 */

#ifndef GPIO_H_
#define GPIO_H_

/*
*  IOCTL request to set the direction of a GPIO bank
* argument is a 32 bit word. Direction is set per bit:  1 == input, 0 == output
* example call:
*
* ioctl(IOCTL_GPIO_SET_DIR,0x3)
*
* sets the first 2 gpio pins to input, all other to output
*
*/
#define IOCTL_GPIO_SET_DIR 0xa7764

/*
*  IOCTL request to get the direction of a GPIO bank
* argument is a pointer to a 32 bit variable.
* Direction per bit:  1 == input, 0 == output
* example call:
*
* unint4 dir;
* ioctl(IOCTL_GPIO_GET_DIR,&dir)
*
*/
#define IOCTL_GPIO_GET_DIR 0xa7765


#endif /* GPIO_H_ */
