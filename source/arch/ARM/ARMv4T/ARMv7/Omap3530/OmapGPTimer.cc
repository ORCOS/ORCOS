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

#include "OMAP3530.h"
#include "inc/memio.h"
#include <kernel/Kernel.hh>
#include <OmapGPTimer.hh>
#include <assemblerFunctions.hh>

extern Kernel* theOS;

OmapGPTimer::OmapGPTimer(T_OmapGPTimer_Init * init) : TimerDevice(init->Name) {
    hwregs = reinterpret_cast<omapgp_regs*>(init->Address);

    /* functional and interface clocks must be enabled */

#if 0
    /* only for 1 ms tick generation */
    // GPT1_TPIR POSITIVE_INC_VALUE = 232000 = 0x38A40
    hwregs->tpir = 0;

    // GPT1_TNIR NEGATIVE_INC_VALUE = 768000 = 0xBB800
    hwregs->tnir = 0;
#endif

    /* GPT1_TLDR LOAD_VALUE = 0xFFFFFFE0 */
    hwregs->tldr = 0;

    /* configure for autoreload, compare mode and disable timer */
    //hwregs->tclr = 0x2 | (0x1 << 6);
    hwregs->tclr = 0x0;

    /* set counter to start value */
    hwregs->tcrr = 0;

    /* clear pending interrupts */
    hwregs->tisr = 0x7;

    irq = init->INTC_IRQ;

    theOS->getBoard()->getInterruptController()->setIRQPriority(init->INTC_IRQ, init->INTC_Priority);
    theOS->getBoard()->getInterruptController()->unmaskIRQ(init->INTC_IRQ);
    theOS->getInterruptManager()->registerIRQ(init->INTC_IRQ, this, 0, IRQ_NOTHREAD);

    /* match value */
    hwregs->tmar = -1;

    /* enable interrupt on timer match (compare mode) */
    /* enable interrupt on timer overflow */
    hwregs->tier = 2;

    /* reset timer interrupt pending bit */
    hwregs->tisr = 0xf;

    /* initialize timer */
    isEnabled       = false;
    time            = 0;
    elapsedCycles   = 0;
    tickRate        = 0;
}

OmapGPTimer::~OmapGPTimer() {
    disable();
}

/*****************************************************************************
 * Method: OmapGPTimer::enable()
 *
 * @description
 *  Enables the hardware timer using the current configuration.
 *******************************************************************************/
ErrorT OmapGPTimer::enable() {
    this->setTimer(time);

    /* start timer */
    hwregs->tclr = 0x1;
    isEnabled = true;
    return (cOk );
}

/*****************************************************************************
 * Method: OmapGPTimer::disable()
 *
 * @description
 *  Disables the hardware timer and clears all interrupt flags.
 *******************************************************************************/
ErrorT OmapGPTimer::disable() {
    isEnabled = false;
    /* stop timer */
    hwregs->tclr = 0;
    /* reset timer interrupt pending bit */
    hwregs->tisr = 0xf;
    return (cOk );
}

/*****************************************************************************
 * Method: OmapGPTimer::setTimer(TimeT t)
 *
 * @description
 *  Programs the hardware timer to generate an interrupt at time t. Also starts the
 *  timer. If t - currenTime does not fit into an int32 the hw interrupt
 *  will be programmed to the maximum time possible.
 *******************************************************************************/
ErrorT OmapGPTimer::setTimer(TimeT t) {
    TimeT currentTime = theOS->getClock()->getClockCycles() + 3 MICROSECONDS;
    /* set to 5 ticks if currentTime > t*/
    unint4 dt = 30;

    /* otherwise generate the next irq in (t- currentTime) ticks */
    if (currentTime < t) {
       TimeT diff = t - currentTime;
       if (diff >= 0xffffffff) {
           dt = 0xffffffff;
       } else {
           dt = (unint4) diff;
       }
    }


    /* minimum 30 ticks to avoid buggy situations*/
     if (dt <= 30) {
       dt = 30;
       hwregs->tclr = 0x0;
       /* directly raise irq by software*/
       theOS->getBoard()->getInterruptController()->raiseIRQ(irq);
     } else {
       /* reset current value to max - dt */
       hwregs->tcrr = 0xffffffff - dt;
       /* initial irq flag clear */
       hwregs->tisr = 0xf;
       /* be sure no spurious irq exists */
       theOS->getBoard()->getInterruptController()->clearIRQ(irq);
       /* start timer again */
       hwregs->tclr = 0x1;
    }


    return (cOk );
}

/*****************************************************************************
 * Method: OmapGPTimer::setPeriod(unint4 microseconds)
 *
 * @description
 *  Configures the Timer to periodic mode issuing timer ticks (IRQS) at the given
 *  period.
 *******************************************************************************/
void OmapGPTimer::setPeriod(unint4 microseconds) {
    unint4 value = 0xffffffff - (microseconds MICROSECONDS);
    /* reset current value to max - dt */
    hwregs->tcrr = value;
    /* reload value is the same */
    hwregs->tldr = value;
    /* initial irq flag clear */
    hwregs->tisr = 0xf;
    /* start timer in auto reload mode */
    hwregs->tclr = 0x1 | (1 << 1);
}



