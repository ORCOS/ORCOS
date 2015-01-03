/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2010 University of Paderborn

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

#include "Timer1.hh"
#include "avr/io.h"
#include "inc/memio.h"
#include <kernel/Kernel.hh>
#include "avr/common.h"

extern "C" Kernel* theOS;

Timer1::Timer1() {
    // initialize timer
    time = 0;

    GTCCR |= (1 << TSM) | (1 << PSRASY);  //Timer anhalten, Prescaler Reset
    TCCR2A = 0;
    TCCR2B = 0;
    TCNT2 = 0;
    OCR2A = 0;
    OCR2B = 0;

    TCCR0B = 0;
    TCCR1B = 0;
   // CLKPR = 128; // enable clock change
   //CLKPR = 6;   // divide 8 mhz input clock by 64
   // 125000 clock ticks per second!
}

Timer1::~Timer1() {
    disable();
}

/*****************************************************************************
 * Method: Timer1::enable()
 *
 * @description
 *  enables the general purpose timer
 *******************************************************************************/
ErrorT Timer1::enable() {
    ASSR = 0;
    // enable clock for timer = clk_sys / 1024
    //TCCR2B = 135;    // divisor = 1024
    TCCR2B = 6;   // divisor = 1024

    TCCR0B = 0;   // disable timer 0
    TCCR1B = 0;   // disable timer 1

    TCCR2A = (1 << WGM21);                //CTC Modus

    TIFR2 = 0;

    // unmaks the compare a interrupt
    TIMSK2 = 0;
    GTCCR &= ~(1 << TSM);                 //Timer starten

    //LOG(ARCH,INFO,"Timer enabled..");
    return (cOk);
}

/*****************************************************************************
 * Method: Timer1::disable()
 *
 * @description
 *  disables the general purpose timer
 *******************************************************************************/
ErrorT Timer1::disable() {
    // disable clock for timer
    //TCCR2B = 0;
    //TIFR2 = 0;
    // mask all interrupts
    //TIMSK2 = 0;
    return (cOk);
}

/*****************************************************************************
 * Method: Timer1::setTimer(unint4 t)
 *
 * @description
 *
 *******************************************************************************/
ErrorT Timer1::setTimer(unint4 t) {
    if (t == 0) t = 1;
    if (t > 254) t = 254;
    time = t;

    GTCCR |= (1 << TSM) | (1 << PSRASY);  //Timer anhalten, Prescaler Reset
    TCCR2A = (1 << WGM21);                //CTC Modus
    TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);  //Prescaler 1024

    // 125000 / 1024 = 122 = 8 ms ticks
    OCR2A = t;
    //ASSR |= (1 << AS2);                //Asynchron Mode einschalten

    TIMSK2 = (1<<OCIE2A);                //Enable Compare Interrupt
    GTCCR &= ~(1 << TSM);                //Timer starten

    return (cOk);
}

/*****************************************************************************
 * Method: Timer1::tick()
 *
 * @description
 *
 *******************************************************************************/
ErrorT Timer1::tick() {
    // invoke the hal tick() method which then calls the dispatcher
    theOS->getClock()->incTime(2);
    return (cOk);
}

