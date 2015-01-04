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

#include <LED.hh>
#include "inc/memio.h"

LED::LED(T_LED_Init* init) :
        CharacterDeviceDriver(true, init->Name) {
    leds = 0;
    this->baseaddr = init->Address;
}

LED::~LED() {
    leds = 0;

    // clear the leds. turn them all off
    OUTW(baseaddr + XGPIO_TRI_OFFSET, 0xFFFFFFFF);
    __asm__ volatile ("eieio");
}

void LED::Clear() {

    // clear the leds. turn them all off
    OUTW(baseaddr + XGPIO_DATA_OFFSET, 0xFFFFFFFF);
    __asm__ volatile ("eieio");
}

void LED::LedOn(int4 ledNumber) {
    if (ledNumber > AVNET_LEDS_NUMBER)
        return;
    register int4 led_on = 1;
    led_on = led_on << (ledNumber - 1);

    leds = leds | led_on;
    register int4 ledval = (leds ^ 0xFFFFFFFF) << 12;
    OUTW(baseaddr + XGPIO_DATA_OFFSET, ledval);
    __asm__ volatile ("eieio");
}

void LED::LedOff(int4 ledNumber) {
    if (ledNumber > AVNET_LEDS_NUMBER)
        return;
    register int4 led_off = 1;
    led_off = led_off << (ledNumber - 1);

    leds = leds & ~(led_off);
    register int4 ledval = (leds ^ 0xFFFFFFFF) << 12;
    OUTW(baseaddr + XGPIO_DATA_OFFSET, ledval);
    __asm__ volatile ("eieio");
}

ErrorT LED::writeByte(char byte) {
    register short b;

    for (unint2 i = 0; i < 8; i++) {
        b = (1 << i);
        b = (byte & b);
        if (b == (1 << i))
            LedOn(i + 1);
        else
            LedOff(i + 1);
    }
    return cOk;
}

ErrorT LED::readByte(char* byte) {
    *byte = leds;
    return cOk;
}

ErrorT LED::readBytes(char *bytes, unint4 &length) {
    return cError;
}

ErrorT LED::writeBytes(const char *bytes, unint4 length) {
    return cError;
}
