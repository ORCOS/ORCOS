 /*
  *
  *   Copyright &  Author: dbaldin
  */

#ifndef BEAGLEBOARDINTERRUPTCONTROLLER_HH_
#define BEAGLEBOARDINTERRUPTCONTROLLER_HH_

#include "inc/types.hh"
#include "inc/memio.h"
#include <SCLConfig.hh>

#define INTCPS_REVISION     0x0
#define INTCPS_SYSCONFIG    0x10
#define INTCPS_SYSSTATUS    0x14
#define INTCPS_SIR_IRQ      0x40
#define INTCPS_SIR_FIQ      0x44
#define INTCPS_CONTROL      0x48
#define INTCPS_PROTECTION   0x4c
#define INTCPS_IDLE         0x50

#define INTCPS_THRESHOLD    0x68

#define INTCPS_ILR(m)             0x100 + (0x4 * (m))
#define INTCPS_MIR_CLEAR(n)       0x088 + (0x20 * (n))
#define INTCPS_MIR_SET(n)         0x08C + (0x20 * (n))
#define INTCPS_ISR_SET(n)         0x090 + (0x20 * (n))
#define INTCPS_ISR_CLEAR(n)       0x094 + (0x20 * (n))

/*!
 * \brief Interrupt Controller Class for the OMAP 35x Interrupt Controller
 */
class Omap3530InterruptController {
private:
    int4 baseAddr;
public:
    Omap3530InterruptController(T_Omap3530InterruptController_Init* init);
    ~Omap3530InterruptController();

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
        unsigned int irqSrc = INW(baseAddr + INTCPS_SIR_IRQ);

        /* spurious irq? */
        if (irqSrc >= 0x80)
            return (0);

        return (irqSrc & 0x7f);
    }

    /*****************************************************************************
     * Method: clearIRQ(int num)
     *
     * @description
     *  Clears the current pending interrupt and allows new interrupt generation
     *******************************************************************************/
    inline void clearIRQ(int num) {
        // data synchronization barrier to ensure peripheral clear irq instructions finished
        asm volatile ("MOV R1, #0; MCR P15, #0, R1, C7, C10, #4;" : : : "r1");
        OUTW(baseAddr + INTCPS_CONTROL, 0x1);
    }

    /*****************************************************************************
     * Method: setPriorityThreshold(int priority)
     *
     * @description
     *  Clears the current priority threshold. All interrupts with lower priorities
     *  will be disabled. Valid range [0-63]. Setting the threshold to 0 will allow
     *  all interrupts as no lower priority exists.
     *******************************************************************************/
    inline void setPriorityThreshold(int priority) {
        /* range check */
        if (priority > 63)
            priority = 63;

        /* invert as this is programmed into hardware */
        int prioval = 63 - priority;
        OUTW(baseAddr + INTCPS_THRESHOLD, prioval);
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


    ErrorT raiseIRQ(unint4 irq_number);

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
