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


#define WRITE_VALUE_IN_REGISTER(Reg_Addr,Value) \
	 { ((*(volatile unsigned int*)(Reg_Addr)) = Value);}

/*---------------------------------------------------------------------------*/
LED::LED(const char* name, int4 baseaddr) : CharacterDeviceDriver(true,name)
/*---------------------------------------------------------------------------*/
{
	leds = 0;
	this->baseaddr = baseaddr;

	//initialize out register
	WRITE_VALUE_IN_REGISTER(baseaddr + GPIO_OUT_OFFSET, 0x00000000)

	//enable output for the first 3 bits
	WRITE_VALUE_IN_REGISTER(baseaddr + GPIO_DIR_OFFSET, 0x00000007)
}

/*---------------------------------------------------------------------------*/
LED::~LED()
/*---------------------------------------------------------------------------*/
{
	leds = 0;
	//reset the led_gpio to default input direction
	WRITE_VALUE_IN_REGISTER(baseaddr + GPIO_OUT_OFFSET, 0)
}




/*---------------------------------------------------------------------------*/
void LED::LedOn (int4 ledNumber)
/*---------------------------------------------------------------------------*/
{
  if(ledNumber > LEDS_NUMBER) return;
  int4 led_on = 1;
  led_on = led_on << (ledNumber-1);
  //
  leds = leds | led_on;
  WRITE_VALUE_IN_REGISTER(baseaddr + GPIO_OUT_OFFSET, leds)
}

/*---------------------------------------------------------------------------*/
void LED::LedOff(int4 ledNumber)
/*---------------------------------------------------------------------------*/
{
  if(ledNumber > LEDS_NUMBER) return;
  int4 led_off = 1;
  led_off = led_off << (ledNumber-1);
  //
  leds = leds & ~(led_off);
  WRITE_VALUE_IN_REGISTER(baseaddr + GPIO_OUT_OFFSET, leds)
}

ErrorT LED::writeByte(char byte)
{
	short b;

	for (unint2 i=0; i<8; i++) {
		b = (1<<i);
		b = (byte & b);
		if (b== (1<<i)) LedOn(i+1);
		else LedOff(i+1);
	}
	return cOk;
}

ErrorT LED::readByte  (char* byte) {
	*byte = leds; return cOk;
	}

ErrorT LED::readBytes (const char *bytes, int4 length) { return cError; }
ErrorT LED::writeBytes(const char *bytes, int4 length) { return cError; }
