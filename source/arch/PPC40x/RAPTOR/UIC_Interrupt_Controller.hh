/*
 * UIC_Interrupt_Controller.hh
 *
 *  Created on: 19.01.2012
 *      Author: kgilles
 */

#ifndef UIC_INTERRUPT_CONTROLLER_HH_
#define UIC_INTERRUPT_CONTROLLER_HH_

#define UIC_INTC_BASE	0xc0

#define UIC_SR_OFFSET	0x00	//UIC Status Register
#define UIC_SRS_OFFSET	0x01	//UIC Status Register Set
#define UIC_ER_OFFSET	0x02	//UIC Enable Register
#define UIC_CR_OFFSET	0x03	//UIC Critical Register
#define UIC_PR_OFFSET	0x04	//UIC Polarity Register
#define UIC_TR_OFFSET	0x05	//UIC Triggering Register
#define UIC_MSR_OFFSET	0x06	//UIC Masked Status Register
#define UIC_VCR_OFFSET	0x07	//UIC Vector Configuration Register
#define UIC_VR_OFFSET	0x08	//UIC Vector Register

#define OPB_UART_LITE_IRQ 	2	// Interrupt port of OPB_UART_LITE
#define PLB_EMAC0_IRQ		1 	// Interrupt port of the ethernet EMAC
#define PUSH_BUTTON_IRQ 	5 	// Interrupt port of the PUSH_BUTTON
/*!
 * \brief Interrupt Controller Class for the UIC_Interrupt_Controller
 *
 * This class allows easy access to the UIC_Interrupt_Controller of ISS
 * instruction set simulator
 */
class UIC_Interrupt_Controller {
public:
	UIC_Interrupt_Controller();
    ~UIC_Interrupt_Controller();

    /*!
     * \brief Returns the content of the interrupts status register
     *
     * The interrupt status register indicates which interrupt input is active and can be used
     * to determine which hardware device raised an interrupt
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

#endif /* UIC_INTERRUPT_CONTROLLER_HH_ */
