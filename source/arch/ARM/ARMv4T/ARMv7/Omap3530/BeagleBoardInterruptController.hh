#ifndef BEAGLEBOARDINTERRUPTCONTROLLER_HH_
#define BEAGLEBOARDINTERRUPTCONTROLLER_HH_

/*!
 * \brief Interrupt Controller Class for the OMAP 35x Interrupt Controller
 */
class BeagleBoardInterruptController {
public:
	BeagleBoardInterruptController();
    ~BeagleBoardInterruptController();

    /*!
     * \brief Returns the content of the interrupts status register
     *
     * The interrupt status register indicates which interrupt input is active and can be used
     * to determine which hardware deviced raised an interrupt
     */
    int getIRQStatusVector();

    /*!
     *  \brief clears the interrupt number 'num'
     */
    void clearIRQ( int num );

    /*!
     * \brief enable all IRQs
     */
    void enableIRQs();
};

#endif /*BEAGLEBOARDINTERRUPTCONTROLLER_HH_*/
