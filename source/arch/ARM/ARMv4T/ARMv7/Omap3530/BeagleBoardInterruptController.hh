 /*
  *
  *   Copyright &  Author: dbaldin
  */

#ifndef BEAGLEBOARDINTERRUPTCONTROLLER_HH_
#define BEAGLEBOARDINTERRUPTCONTROLLER_HH_

#include "inc/types.hh"
#include "OMAP3530.h"
#include "inc/memio.h"

/*!
 * \brief Interrupt Controller Class for the OMAP 35x Interrupt Controller
 */
class BeagleBoardInterruptController {
public:
    BeagleBoardInterruptController();
    ~BeagleBoardInterruptController();

    /*****************************************************************************
     * Method: getIRQStatusVector()
     *
     * @description
     *  Returns the currently highest priority irq asserted.
     *
     * The interrupt status register indicates which interrupt input is active and can be used
     * to determine which hardware device raised an interrupt
     *******************************************************************************/
    inline int getIRQStatusVector()  {
        int irqSrc = INW(MPU_INTCPS_SIR_IRQ);

        /* spurious irq? */
        if (irqSrc >= 0x80)
            return (0);

        return (irqSrc & 0x7f);
    }

    /*****************************************************************************
     * Method: clearIRQ(int num)
     *
     * @description
     *  Clears the current pending interrupt
     *******************************************************************************/
    inline void clearIRQ(int num) {
        OUTW(MPU_INTCPS_CONTROL, 0x1);
    }


    /*****************************************************************************
     * Method: unmaskIRQ(unint4 irq_number)
     *
     * @description
     *  Unmask the spcific IRQ which allows the IRQ to be raised
     *******************************************************************************/
    ErrorT unmaskIRQ(unint4 irq_number);


    /*****************************************************************************
     * Method: maskIRQ(unint4 irq_number)
     *
     * @description
     *  Masks the spcific IRQ which prevents the IRQ from being raised
     *******************************************************************************/
    ErrorT maskIRQ(unint4 irq_number);


    /*****************************************************************************
     * Method: setIRQPriority(int irq_number, int priority)
     *
     * @description
     *  Sets the hardware priority of the IRQ source with priority 63 being the highest
     *  priority. 0 beeing the lowest priority.
     *******************************************************************************/
    ErrorT setIRQPriority(unint4 irq_number, unint4 priority);

    /*****************************************************************************
     * Method: enableIRQs()
     *
     * @description
     *  Enable IRQ generation
     *******************************************************************************/
    void enableIRQs();
};

#endif /*BEAGLEBOARDINTERRUPTCONTROLLER_HH_*/
