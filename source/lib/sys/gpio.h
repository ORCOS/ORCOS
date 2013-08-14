/*
 * gpio.h
 *
 *  Created on: 14.08.2013
 *      Author: dbaldin
 */

#ifndef GPIO_H_
#define GPIO_H_

//  IOCTL request to set the direction of a GPIO pin
#define IOCTL_GPIO_SET_DIR 0xa7764
// argument is a 32 bit word. Direction is set per bit:  1 == input, 0 == output
// example call: ioctl(IOCTL_GPIO_SET_DIR,0x3)
// sets the first 2 gpio pins to input, all other to output




#endif /* GPIO_H_ */
