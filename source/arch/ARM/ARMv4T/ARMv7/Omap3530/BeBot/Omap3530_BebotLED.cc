/*
 * Omap3530_GPIO.cc
 *
 *  Created on: 03.09.2012
 *      Author: danielb
 */

#include "Omap3530_BebotLED.hh"
#include "kernel/Kernel.hh"
#include "inc/memio.h"
#include "inc/types.hh"

extern Kernel* theOS;

#define A6281_GPIO_OFFET 128
#define A6281_GPIO GPIO5_BASE

#define A6281_DATA    (154-A6281_GPIO_OFFET)
#define A6281_CLOCK   (152-A6281_GPIO_OFFET)
#define A6281_LATCH   (155-A6281_GPIO_OFFET)
#define A6281_ENABLE  (153-A6281_GPIO_OFFET)

static void udelay(unint4 loops)
{

	while(--loops) {
	 __asm__ __volatile__("nop");
	}

}

#define a6281_setclock(i) 	OUTW(A6281_GPIO + GPIO_SETDATAOUT, i << A6281_CLOCK )
#define a6281_setdata(i) 	OUTW(A6281_GPIO + GPIO_SETDATAOUT, i << A6281_DATA )
#define a6281_setlatch(i) 	OUTW(A6281_GPIO + GPIO_SETDATAOUT, i << A6281_LATCH )

#define A6281_PWM		(0 << 30)
#define A6281_CFG		(1 << 30)
#define A6281_CHANNEL_SHIFT(x)	(x * 10)
#define A6281_CORRECTION_MASK	0x7f
#define A6281_CLOCK_MASK	0x3
#define A6281_PWM_MASK		0x3ff
#define A6281_CLOCK_SHIFT	7


void a6281_write(unint4 *data, int count) {

	for (int i = 0; i < count; i++) {
		for (int j = 31; j >= 0; j--) {
			OUTW(A6281_GPIO + GPIO_CLEARDATAOUT,1 << A6281_DATA);
			a6281_setdata( ((data[i] >> j) & 0x1));
			udelay(100);
			OUTW(A6281_GPIO + GPIO_CLEARDATAOUT,1 << A6281_CLOCK);
			a6281_setclock(1);
			udelay(100);
			OUTW(A6281_GPIO + GPIO_CLEARDATAOUT,1 << A6281_CLOCK);
			a6281_setclock(0);
		}

	}

	OUTW(A6281_GPIO + GPIO_CLEARDATAOUT,1 << A6281_LATCH);
	a6281_setlatch(1);
	udelay(100);
	OUTW(A6281_GPIO + GPIO_CLEARDATAOUT,1 << A6281_LATCH);
	a6281_setlatch(0);
}

#define LEDS_NUM 12

struct a6281_led {
	int brightness;
	int correction;
};

struct a6281_led leds[13];

void a6281_set_cfg() {
	unint4 data[LEDS_NUM / 3];

	for (int i = 0; i < 4; i++) {
		int n = i *3;
		int dw = 0;

		for (int j = 0; j < 3; j++) {
			int counter = leds[n+j].correction & A6281_CORRECTION_MASK;
			dw |= counter << A6281_CHANNEL_SHIFT(j);
		}
		int clock = 1 & A6281_CLOCK_MASK;
		dw |= clock << A6281_CLOCK_SHIFT;

		data[4-i-1] = dw | A6281_CFG;
	}

	a6281_write((unint4*) &data,4);
}

// sets brightness of the leds ..
void a6281_set_pwm() {
	unint4 data[LEDS_NUM / 3];

	for (int i = 0; i < 4; i++) {
		int n = i *3;
		int dw = 0;

		for (int j = 0; j < 3; j++) {
			int counter = leds[n+j].brightness & A6281_PWM_MASK;
			dw |= counter << A6281_CHANNEL_SHIFT(j);
		}

		data[4-i-1] = dw | A6281_PWM;
	}

	a6281_write((unint4*) &data,4);
}

Omap3530_BebotLED:: Omap3530_BebotLED(T_Omap3530_BebotLED_Init* init )
	: CharacterDeviceDriver(false,init->Name)
{

	OUTW(A6281_GPIO + GPIO_SYSCONFIG,0);
	OUTW(GPIO6_BASE + GPIO_SYSCONFIG,0);

	// disable all irqs
	OUTW(A6281_GPIO + GPIO_IRQENABLE1,0);
	OUTW(GPIO6_BASE + GPIO_IRQENABLE1,0);

	// enable all gpios
	OUTW(A6281_GPIO + GPIO_CTRL,0);
	OUTW(GPIO6_BASE + GPIO_CTRL,0);

	// set all pins to output
	OUTW(A6281_GPIO + GPIO_OE,0);
	OUTW(GPIO6_BASE + GPIO_OE,0);

	leds[0].brightness = 0x40; // blau
	leds[1].brightness = 0x0; // green
	leds[2].brightness = 0x0; // red

	leds[3].brightness = 0x40; // blau
	leds[4].brightness = 0x0; // green
	leds[5].brightness = 0x0; // red

	leds[6].brightness = 0x40; // blau
	leds[7].brightness = 0x0; // green
	leds[8].brightness = 0x0; // red

	leds[9].brightness = 0x40; // blau
	leds[10].brightness = 0x0; // green
	leds[11].brightness = 0x0; // red

	for (int i = 0; i < 12; i++) {
		leds[i].correction = 0x0;
	}

	OUTW(A6281_GPIO + GPIO_CLEARDATAOUT, 1 << A6281_ENABLE );
	OUTW(A6281_GPIO + GPIO_SETDATAOUT, 0 << A6281_ENABLE );

	a6281_set_cfg();
	a6281_set_pwm();
}

Omap3530_BebotLED::~Omap3530_BebotLED() {

}


void Omap3530_BebotLED::LedOn(int4 ledNumber)
{

}



void Omap3530_BebotLED::LedOff(int4 ledNumber)
{

}



void Omap3530_BebotLED::Clear()
{

}



ErrorT Omap3530_BebotLED::writeByte(char byte)
{
	return cError;
}



ErrorT Omap3530_BebotLED::readByte(char *byte)
{
	return cError;
}



ErrorT Omap3530_BebotLED::readBytes(char *bytes, unint4 & length)
{
	return cError;
}




ErrorT Omap3530_BebotLED::writeBytes(const char *bytes, unint4 length)
{
	if (length != 15) {
		LOG(ARCH,WARN,(ARCH,WARN,"Omap3530_LED::writeBytes length %d != 24",length));
		return cError;
	}

	for (int i = 0; i < 12; i++) {
		leds[i].brightness = bytes[i];
	}

	OUTW(GPIO6_BASE + GPIO_CLEARDATAOUT, 1 << 19 | 1 << 20 | 1 << 21);

	OUTW(GPIO6_BASE + GPIO_SETDATAOUT,((bytes[12] > 0) << 19) | ((bytes[13] > 0) << 20) | ((bytes[14] > 0) << 21) );

	a6281_set_pwm();

	return cOk;

}
