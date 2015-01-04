#ifndef OPB_INTERRUPT_CONTROLLER_HH_
#define OPB_INTERRUPT_CONTROLLER_HH_

#define OPB_INTC_BASE       0x90240000
#define OPB_INTC_ISR_OFFSET 0x0 // Interrupt Status Register
#define OPB_INTC_IER_OFFSET 0x8 // Interrupt Enable Register
#define OPB_INTC_IAR_OFFSET 0xC // Interrupt Acknowledge Register
#define OPB_INTC_MER_OFFSET 0x1C // Master Enable Register

#define OPB_UART_LITE_IRQ   2    // Interrupt port of OPB_UART_LITE
#define PLB_EMAC0_IRQ       1     // Interrupt port of the ethernet EMAC
#define PUSH_BUTTON_IRQ     5     // Interrupt port of the PUSH_BUTTON
/*!
 * \brief Interrupt Controller Class for the OPB_Interrupt_Controller
 *
 * This class allows easy access to the OPB_Interrupt_Controller on the RAPTOR
 * board if built into the fpga.
 */
class OPB_Interrupt_Controller {
public:
    OPB_Interrupt_Controller();
    ~OPB_Interrupt_Controller();

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
    void clearIRQ(int num);

    /*!
     * \brief enable all IRQs
     */
    void enableIRQs();
};

#endif /*OPB_INTERRUPT_CONTROLLER_HH_*/
