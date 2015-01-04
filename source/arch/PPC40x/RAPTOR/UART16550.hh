/*
 * UART16550.hh
 *
 *  Created on: 17.01.2012
 *      Author: kgilles
 */

#ifndef UART16550_HH_
#define UART16550_HH_

#include <error.hh>
#include <types.hh>
#include <arch/shared/UART/UART.hh>
#include <hal/CommDeviceDriver.hh>
#include "../powerpc.h"

/*----------------------------------------------------------------------------+
 | UART1 register definitions.
 +----------------------------------------------------------------------------*/
#define UART_RX_OFFSET              0x0
#define UART_DLL_OFFSET             0x0
#define UART_TX_OFFSET              0x0
#define UART_IER_OFFSET             0x1
#define UART_DLM_OFFSET             0x1
#define UART_IIR_OFFSET             0x2
#define UART_FCR_OFFSET             0x2
#define UART_LCR_OFFSET             0x3
#define UART_MCR_OFFSET             0x4
#define UART_LSR_OFFSET             0x5
#define UART_MSR_OFFSET             0x6
#define UART_SR_OFFSET              0x7

#define UART_IER_DISABLE_ALL            0x00

#define UART_FCR_FIFO_ENABLE            0x01
#define UART_FCR_FIFO_R_FIFO_RES        0x02
#define UART_FCR_FIFO_T_FIFO_RES        0x04
#define UART_FCR_FIFO_TRIG_1            0x00

#define UART_LCR_DLAB                   0x80
#define UART_LCR_WORD_LENGTH8           0x03
#define UART_LCR_STOP_BITSONE           0x00
#define UART_LCR_PARITY_DISABLE         0x00
#define UART_LCR_ODD_PARITY             0x00

#define UART_MCR_OUT2                   0x08
#define UART_MCR_OUT1                   0x04
#define UART_MCR_RTS                    0x02
#define UART_MCR_DTR                    0x01

#define UART_LSR_DATA_READY             0x01
#define UART_LSR_TX_EMPTY               0x60

/*!
 *  \brief ISS Uart driver.
 *
 *  This class encapsulates the UART driver implementation.
 */
class UART16550: public CharacterDevice {
private:
    //! the memory mapped IO address of this device
    int4 addr;

    /*!
     * \brief put a byte into the send buffer of the device
     */
    void sendByte(byte Data);

    /*!
     * \brief read a byte out of the receive buffer
     */
    byte recvByte();

    //! checks whether the transmit buffer is full or nor
    bool isTransmitRegEmpty();

    //! check whether the receive buffer is full or not
    bool isReceiveBufferFull();

    /*!
     *  \brief Method that sends data. A Timeout specifies how may times it shall retry to send
     *          it if the transmit buffer of the device is full.
     *
     *  A Timeout of -1 causes it to wait indefinitely until the transmit buffer has free space.
     */
    ErrorT outputSCC(int4 Timeout, byte c);

    /*!
     *  \brief Method that reads data. A Timeout specifies how may times it shall retry to read
     *          it if the receive buffer of the device is empty.
     *
     *  A Timeout of -1 causes it to wait indefinitely until the receive buffer has some data.
     */
    ErrorT inputSCC(int4 Timeout, byte *c);

public:
    //!  constructor
    UART16550(const char *name, int4 a);

    //!  destructor
    ~UART16550();

    //! enables Interrupt Requests of this device
    ErrorT enableIRQ();

    //! disables Interrupt Requests of this device. Interrupts may still be pending.
    ErrorT disableIRQ();

    //! HAL implementation of readBytes()
    ErrorT readBytes(char *bytes, unint4 &length);

    //! HAL implementation of writeBytes()
    ErrorT writeBytes(const char *bytes, unint4 length);

    //! Checks whether there is some data available inside the receive fifo.
    bool hasPendingData();
};

#endif /* UART16550_HH_ */
