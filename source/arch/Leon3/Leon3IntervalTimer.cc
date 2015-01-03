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

#include "Leon3IntervalTimer.hh"
#include "asm/Leon3.hh"
#include <assembler.h>
#include "assemblerFunctions.hh"
#include <kernel/Kernel.hh>


extern "C" Kernel* theOS;

Leon3IntervalTimer::Leon3IntervalTimer()
{
    isEnabled = false;

    //Set address dynamically in multiprocessor configurations
    baseAddr = 0x80000320;


    int index = 0;
    GET_CPU_INDEX(index);
    baseAddr += index * 0x10;

    // base irq for timer unit is 8, Clock has assigned 8
    irqNum = 9 + index;
}

Leon3IntervalTimer::~Leon3IntervalTimer() {
    disable();
}

ErrorT Leon3IntervalTimer::enable() {
    isEnabled = true;

    this->setTimer(time);

    OUTW(baseAddr + CTRL_REG_OFFSET, (TCR_IE | TCR_LD | TCR_RS | TCR_EN));

    return cOk;
}


ErrorT Leon3IntervalTimer::disable() {
    isEnabled = false;
    OUTW(baseAddr + CTRL_REG_OFFSET, INW(baseAddr + CTRL_REG_OFFSET) & ~(TCR_IE | TCR_EN));

    return cOk;
}

ErrorT Leon3IntervalTimer::setTimer(unint4 t) {
    /*
     * the prescaler is set to a tick every microsecond,
     * so the timer is set to t
     */

    tickRate =  t;

    theOS->getBoard()->getInterruptController()->clearIRQ(irqNum);

    /*
     * Set the timer reload register to t
     */
    OUTW(baseAddr + RELOAD_REG_OFFSET, (t / (CLOCK_RATE / 1000000)));

    TimerDevice::setTimer(t);

    OUTW((baseAddr + CTRL_REG_OFFSET), (TCR_IE | TCR_LD | TCR_RS | TCR_EN));

    /*
     * It is possible that the timer generated an interrupt during the routines
     * which handles the timer interrupt. interrupts are deactivated by setting
     * the interrupt level in the psr register. If the timer underruns, it sets the
     * interrupt pending register, but don't cause an interrupt. If the pending interrupt
     * is not cleared, than it would cause an interrupt immediately after demasking
     * interrupts.
     * */
    return cOk;
}

ErrorT     Leon3IntervalTimer::tick()
{
    OUTW(baseAddr + CTRL_REG_OFFSET, INW(baseAddr + CTRL_REG_OFFSET) & ~(TCR_EN | TCR_IE | TCR_IP));


    // invoke the hal tick() method which then calls the dispatcher
    return TimerDevice::tick();
}

int Leon3IntervalTimer::getIRQ(){
    return irqNum;
}
