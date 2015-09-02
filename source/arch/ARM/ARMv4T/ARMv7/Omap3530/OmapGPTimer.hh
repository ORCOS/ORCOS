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

#ifndef BEAGLEBOARDGPTIMER2_HH_
#define BEAGLEBOARDGPTIMER2_HH_

#include <error.hh>
#include <types.hh>
#include <hal/TimerDevice.hh>

typedef struct {
    volatile unint4 tidr;        /* 0x0 TI internal data*/
    volatile unint4 reserved0;   /* 0x4 */
    volatile unint4 reserved1;   /* 0x8 */
    volatile unint4 reserved2;   /* 0xc */
    volatile unint4 tiocp_cfg;   /* 0x10 */
    volatile unint4 tistat;      /* 0x14 */
    volatile unint4 tisr;        /* 0x18 */
    volatile unint4 tier;        /* 0x1c */
    volatile unint4 twer;        /* 0x20 */
    volatile unint4 tclr;        /* 0x24 */
    volatile unint4 tcrr;        /* 0x28 */
    volatile unint4 tldr;        /* 0x2c */
    volatile unint4 ttgr;        /* 0x30 */
    volatile unint4 twps;        /* 0x34 */
    volatile unint4 tmar;        /* 0x38 */
    volatile unint4 tcar1;       /* 0x3c */
    volatile unint4 tsicr;       /* 0x40 */
    volatile unint4 tcar2;       /* 0x44 */
    /* only specific timers for 1 ms tick generation */
    volatile unint4 tpir;        /* 0x48 */
    volatile unint4 tnir;        /* 0x4c */
    volatile unint4 tcvr;        /* 0x50 */
    volatile unint4 tocr;        /* 0x54 */
    volatile unint4 towr;        /* 0x58 */
} omapgp_regs;


/*!
 * \brief BeagleBoardGPTimer1, implementation of HAL TimerDevice
 *
 * this is the hardware specific implementation of the TimerDevice HAL class
 */
class OmapGPTimer: public TimerDevice {
private:
    volatile omapgp_regs* hwregs;
    int irq;

public:
    /*****************************************************************************
     * Method: OmapGPTimer(T_OmapGPTimer_Init * init)
     *
     * @description
     *******************************************************************************/
    explicit OmapGPTimer(T_OmapGPTimer_Init * init);
    ~OmapGPTimer();


    /*****************************************************************************
     * Method: enable()
     *
     * @description
     *  Enable the general purpose timer
     *******************************************************************************/
    ErrorT enable();

    /*****************************************************************************
     * Method: disable()
     *
     * @description
     *  Disable the general purpose timer
     *******************************************************************************/
    ErrorT disable();

    /*****************************************************************************
     * Method: setTimer(TimeT t)
     *
     * @description
     *  Set the timer register. The next IRQ will be generated at the given
     *  clock cycles.
     *******************************************************************************/
    ErrorT setTimer(TimeT t);


    /*****************************************************************************
    * Method: getTimerValue()
    *
    * @description
    *  Returns the current timer value.
    *******************************************************************************/
    unint getTimerValue() {
        return (hwregs->tcrr - hwregs->tldr);
    }

    /*****************************************************************************
     * Method: setPeriod(unint4 microseconds)
     *
     * @description
     *  Configures the Timer to periodic mode issuing timer ticks (IRQS) at the given
     *  period.
     *******************************************************************************/
    void setPeriod(unint4 microseconds);

    /*****************************************************************************
     * Method: clearIRQ()
     *
     * @description
     *  Clears the IRQ request flag.
     *******************************************************************************/
    inline ErrorT clearIRQ(int irq_num) {
        /* reset timer interrupt pending bit */
        hwregs->tisr = 0xf;
        return (cOk);
    }

    /*****************************************************************************
     * Method: tick()
     *
     * @description
     *   Invoke the hal tick() method which then calls the dispatcher
     *******************************************************************************/
    inline void tick() __attribute__((noreturn)) {
        dispatcher->dispatch();
    }
};

#endif /* BEAGLEBOARDGPTIMER1_HH_ */
