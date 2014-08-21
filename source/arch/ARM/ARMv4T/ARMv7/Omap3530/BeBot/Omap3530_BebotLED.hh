/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2008 University of Paderborn

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _Omap3530_BebotLED_HH
#define _Omap3530_BebotLED_HH

#include <error.hh>
#include <types.hh>
#include <hal/CharacterDeviceDriver.hh>

#define GPIO1_BASE		0x48310000
#define GPIO2_BASE		0x49050000
#define GPIO3_BASE		0x49052000
#define GPIO4_BASE		0x49054000
#define GPIO5_BASE		0x49056000
#define GPIO6_BASE		0x49058000

#define GPIO_CTRL		0x30
#define GPIO_CLEARDATAOUT 0x90
#define GPIO_SETDATAOUT 0x94
#define GPIO_DATAIN		0x38
#define GPIO_DATAOUT	0x3c
#define GPIO_SYSCONFIG  0x10
#define GPIO_SYSSTATUS  0x14
#define GPIO_OE 		0x34
#define GPIO_IRQENABLE1 0x1c
#define GPIO_IRQENABLE2 0x2c

/*!
 *  \brief Driver providing access to the LEDs of the BeBot
 *
 */
class Omap3530_BebotLED: public CharacterDevice {

public:
    Omap3530_BebotLED(T_Omap3530_BebotLED_Init* init);
    ~Omap3530_BebotLED();

    void LedOn(int4 ledNumber);
    void LedOff(int4 ledNumber);

    void Clear();

    ErrorT writeByte(char byte);
    ErrorT readByte(char* byte);
    ErrorT readBytes(char *bytes, unint4 &length);
    ErrorT writeBytes(const char *bytes, unint4 length);
};

#endif /* _LED_HH */

