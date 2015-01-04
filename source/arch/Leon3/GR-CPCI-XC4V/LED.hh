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

#ifndef _LED_HH
#define _LED_HH

#include <hal/CharacterDeviceDriver.hh>

#define LEDS_NUMBER 3
/**
 * GPIO_OUT_OFFSET    Output register
 * GPIO_DIR_OFFSET    port direction register (sets input/output direction)
 *                    1:output enabled;  0: output disabled
 */
#define GPIO_OUT_OFFSET   0x00000004
#define GPIO_DIR_OFFSET   0x00000008

/*!
 *  \brief driver providing access to the Virtex4 board's LEDs
 */
class LED : public CharacterDevice {
private:
    char leds;
    int4 baseaddr;
public:
    LED(const char* name, int4 baseaddr);
    ~LED();

    void LedOn (int4 ledNumber);
    void LedOff(int4 ledNumber);
    ErrorT writeByte(char byte);
    ErrorT readByte(char* byte);
    ErrorT readBytes(const char *bytes, int4 length);
    ErrorT writeBytes(const char *bytes, int4 length);
};

#endif /* _LED_HH */
