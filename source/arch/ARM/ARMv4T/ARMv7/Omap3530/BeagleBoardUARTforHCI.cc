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

#include "arch/ARM/ARMv4T/ARMv7/Omap3530/BeagleBoardUARTforHCI.hh"
#include "comm/AddressProtocol.hh"
#include "inc/memio.h"
#include "kernel/Kernel.hh"
#include "OMAP3530.h"

/*---------------------------------------------------------------------------*/
BeagleBoardUARTforHCI::~BeagleBoardUARTforHCI()
/*---------------------------------------------------------------------------*/
{

}

/*---------------------------------------------------------------------------*/
BeagleBoardUARTforHCI::BeagleBoardUARTforHCI(const char* name, int4 a) :
        CommDeviceDriver(name)
/*---------------------------------------------------------------------------*/
{

    baseAddr = a;
    currentPosition = 0;
    //char read_byte = 0;

    unint1 temp_lcr, temp_mcr, temp_efr;

    // configure and enable interrupts within interrupt controller
    /*OUTW(MPU_INTCPS_ILR(74), (INW(MPU_INTCPS_ILR(74)) & ~ 0x1 )); // normal irq
     OUTW(MPU_INTCPS_ILR(74), (INW(MPU_INTCPS_ILR(74)) & ~ 0xFC )); // priority 0
     OUTW(MPU_INTCPS_MIR_CLEAR(2), 0x400 ); // enable interrupt*/

    // Software reset
    //OUTW(baseAddr + UART_SYSC_REG_OFFSET, 0x2);
    //while((INW(baseAddr + UART_SYSC_REG_OFFSET) & 0x1) == 0) ;
    // disable fifo
    /*OUTW(baseAddr + UART_FCR_REG_OFFSET, 0x0000);
     // FIFO settings
     // save the LCR register
     temp_lcr = INW(baseAddr + UART_LCR_REG_OFFSET);
     // switch to register configuration mode B
     OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x00BF);
     // Enable register submode TCR_TLR to access UARTi.TLR_REG (part 1 of 2):
     // Save the UARTi.EFR_REG[4] ENHANCED_EN value
     temp_efr = ((INW(baseAddr + UART_EFR_REG_OFFSET) & 0x10) >> 4);
     // Set the UARTi.EFR_REG[4] ENHANCED_EN bit to 1
     OUTW(baseAddr + UART_EFR_REG_OFFSET, (INW(baseAddr + UART_EFR_REG_OFFSET) | UART_EFR_ENHANCED_EN));
     // switch to register configuration mode A
     OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x0080);
     // Enable register submode TCR_TLR to access UARTi.TCR_REG (part 2 of 2):
     temp_mcr = ((INW(baseAddr + UART_MCR_REG_OFFSET) & 0x40) >> 6);
     OUTW(baseAddr + UART_MCR_REG_OFFSET, INW(baseAddr + UART_MCR_REG_OFFSET) | 0x40);
     // Enable FIFO, load the new FIFO triggers (part 1 of 3) and the new DMA mode (part 1 of 2):
     //OUTW(baseAddr + UART_FCR_REG_OFFSET, ); // RX_FIFO_TRIG 7:6 not considered
     //OUTW(baseAddr + UART_FCR_REG_OFFSET, ); // TX_FIFO_TRIG 5:4 not considered
     OUTW(baseAddr + UART_FCR_REG_OFFSET, (INW(baseAddr + UART_FCR_REG_OFFSET) & ~0x8)); // DMA_MODE off
     OUTW(baseAddr + UART_FCR_REG_OFFSET, (INW(baseAddr + UART_FCR_REG_OFFSET) | 0x1)); // Enable FIFO
     // switch to register configuration mode B
     OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x00BF);
     // Load the new FIFO triggers (part 2 of 3):
     OUTW(baseAddr + UART_TLR_REG_OFFSET, INW(baseAddr + UART_TLR_REG_OFFSET) | 0xF); // RX_FIFO_TRIG_DMA 7:4
     OUTW(baseAddr + UART_TLR_REG_OFFSET, INW(baseAddr + UART_TLR_REG_OFFSET) | 0x10); // TX_FIFO_TRIG_DMA 3:0
     // Load the new FIFO triggers (part 3 of 3) and the new DMA mode (part 2 of 2):
     OUTW(baseAddr + UART_SCR_REG_OFFSET, (INW(baseAddr + UART_SCR_REG_OFFSET) & ~0x80)); // RX_TRIG_GRANU1
     OUTW(baseAddr + UART_SCR_REG_OFFSET, (INW(baseAddr + UART_SCR_REG_OFFSET) & ~0x40)); // TX_TRIG_GRANU1
     // Restore the UARTi.EFR_REG[4] ENHANCED_EN value saved in Step 2a.
     OUTW(baseAddr + UART_EFR_REG_OFFSET, (INW(baseAddr + UART_EFR_REG_OFFSET) & (~0x10)) | temp_efr << 4);
     // switch to register configuration mode A
     OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x0080);
     // Restore the UARTi.MCR_REG[6] TCR_TLR value
     OUTW(baseAddr + UART_MCR_REG_OFFSET, (INW(baseAddr + UART_MCR_REG_OFFSET) & (~0x40)) | temp_mcr << 6);
     // Restore the UARTi.LCR_REG value
     OUTW(baseAddr + UART_LCR_REG_OFFSET, temp_lcr);
     // skip DMA configuration because it is not used*/

    /* Baud rate settings */
    // disable mode select on uart1 to access DLL and DLH
    OUTW(baseAddr + UART_MDR1_REG_OFFSET, 0x7);
    // Switch to register configuration mode B
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x00BF);
    // Enable access to UARTi.IER_REG[7:4]: set ENHANCED_EN to 1
    OUTW(baseAddr + UART_EFR_REG_OFFSET, (INW(baseAddr + UART_EFR_REG_OFFSET) | UART_EFR_ENHANCED_EN));
    // Switch to register operational mode to access the UARTi.IER_REG register
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x0000);
    // Clear the UARTi.IER_REG. Set UARTi.IER_REG to 0x0000
    OUTW(baseAddr + UART_IER_REG_OFFSET, 0x0000);
    // Switch to register configuration mode B
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x00BF);
    // for baud rate 9600: set DLH and DLL to 0x01, 0x38
    // for baud rate 38400: set DLH and DLL to 0x00, 0x4E
    // for baud rate 115200: set DLH and DLL to 0x00, 0x1A
    // for baud rate 921600: set DLH and DLL to 0x00, 0x04
    OUTW(baseAddr + UART_DLH_REG_OFFSET, 0x00);
    OUTW(baseAddr + UART_DLL_REG_OFFSET, 0x04);
    //Switch to register operational mode to access the UARTi.IER_REG register
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x0000);
    enableIRQ();
    // Switch to register configuration mode B
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x00BF);
    // Disable access to UARTi.IER_REG[7:4]: set ENHANCED_EN to 0
    OUTW(baseAddr + UART_EFR_REG_OFFSET, (INW(baseAddr + UART_EFR_REG_OFFSET) & ~ UART_EFR_ENHANCED_EN));
    // Switch to register operational mode
    OUTW(baseAddr + UART_LCR_REG_OFFSET, (INW(baseAddr + UART_LCR_REG_OFFSET) & ~(UART_LCR_DIV_EN | UART_LCR_BREAK_EN)));
    // set protocol format: no parity, 1 stop bit, char length 8 bit
    OUTW(baseAddr + UART_LCR_REG_OFFSET, ((INW(baseAddr + UART_LCR_REG_OFFSET) | UART_LCR_CHAR_LENGTH) & ~(UART_LCR_PARITY_EN | UART_LCR_NB_STOP)));
    // set uart1 MODE_SELECT to uart16x mode (0x0)
    OUTW(baseAddr + UART_MDR1_REG_OFFSET, 0x0);

    /* Enable hardware flow control */
    // switch to register mode A
    temp_lcr = INW(baseAddr + UART_LCR_REG_OFFSET);
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x0080);
    // Enable register submode TCR_TLR to access UARTi.TCR_REG (part 1 of 2):
    temp_mcr = ((INW(baseAddr + UART_MCR_REG_OFFSET) & 0x40) >> 6);
    OUTW(baseAddr + UART_MCR_REG_OFFSET, INW(baseAddr + UART_MCR_REG_OFFSET) | 0x40);
    // Switch to register configuration mode B to access the UARTi.EFR_REG register:
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x00BF);
    // Enable register submode TCR_TLR to access the UARTi.TCR_REG register (part 2 of 2):
    temp_efr = ((INW(baseAddr + UART_EFR_REG_OFFSET) & 0x10) >> 4);
    OUTW(baseAddr + UART_EFR_REG_OFFSET, INW(baseAddr + UART_EFR_REG_OFFSET) | 0x10);
    // Load the new start and halt trigger values for hardware flow control:
    OUTW(baseAddr + UART_TCR_REG_OFFSET, INW(baseAddr + UART_TCR_REG_OFFSET) & ~0xF0);
    OUTW(baseAddr + UART_TCR_REG_OFFSET, INW(baseAddr + UART_TCR_REG_OFFSET) | 0xF);
    // Enable receive and transmit hardware flow control mode
    OUTW(baseAddr + UART_EFR_REG_OFFSET, INW(baseAddr + UART_EFR_REG_OFFSET) | 0x40);
    OUTW(baseAddr + UART_EFR_REG_OFFSET, INW(baseAddr + UART_EFR_REG_OFFSET) | 0x80);
    // restore the UARTi.EFR_REG[4] ENHANCED_EN value
    OUTW(baseAddr + UART_EFR_REG_OFFSET, (INW(baseAddr + UART_EFR_REG_OFFSET) & (~0x10)) | temp_efr << 4);
    // Switch to register configuration mode A to access UARTi.MCR_REG:
    OUTW(baseAddr + UART_LCR_REG_OFFSET, 0x0080);
    // Restore the UARTi.MCR_REG[6] TCR_TLR value
    OUTW(baseAddr + UART_MCR_REG_OFFSET, (INW(baseAddr + UART_MCR_REG_OFFSET) & (~0x40)) | temp_mcr << 6);
    // Restore the UARTi.LCR_REG value
    OUTW(baseAddr + UART_LCR_REG_OFFSET, temp_lcr);

#if ENABLE_TXRX_LEDS
    TX_COUNT = 0;
    RX_COUNT = 0;
#endif

}

/*---------------------------------------------------------------------------*/
ErrorT BeagleBoardUARTforHCI::enableIRQ()
/*---------------------------------------------------------------------------*/
{
    // enable irq within UART module
    //OUTW(baseAddr + UART_IER_REG_OFFSET, (INW(baseAddr + UART_IER_REG_OFFSET) | UART_IER_MODEM | UART_IER_LINE | UART_IER_THR | UART_IER_RHR));
    OUTW(baseAddr + UART_IER_REG_OFFSET, (INW(baseAddr + UART_IER_REG_OFFSET) | UART_IER_RHR));
    return 0;
}

/*---------------------------------------------------------------------------*/
ErrorT BeagleBoardUARTforHCI::disableIRQ()
/*---------------------------------------------------------------------------*/
{
    // disable all irqs
    OUTW(baseAddr + UART_IER_REG_OFFSET, (INW(baseAddr + UART_IER_REG_OFFSET) & ~( UART_IER_MODEM | UART_IER_LINE | UART_IER_THR | UART_IER_RHR )));
    return 0;
}

/*---------------------------------------------------------------------------*/
ErrorT BeagleBoardUARTforHCI::readByte(char* byteptr)
/*---------------------------------------------------------------------------*/
{
    ErrorT err = cOk;
    err = inputSCC(1, (byte*) byteptr);
    return err;
}

/*---------------------------------------------------------------------------*/
ErrorT BeagleBoardUARTforHCI::writeByte(char byte)
/*---------------------------------------------------------------------------*/
{
    outputSCC(-1, byte);
    return cOk ;
}

/*---------------------------------------------------------------------------*/
ErrorT BeagleBoardUARTforHCI::readBytes(char* bytes, unint4 &length)
/*---------------------------------------------------------------------------*/
{
    // This method polls (no waiting) for new data

    ErrorT err;
    unint4 count;

    for (count = 0, err = cOk ; ((count < length) && (err == cOk )); count++)
        err = inputSCC(1, (byte*) bytes + count);
    length = (err == cOk ) ? count : count - 1;
    return err;
}

/*---------------------------------------------------------------------------*/
ErrorT BeagleBoardUARTforHCI::writeBytes(const char* bytes, unint4 length)
/*---------------------------------------------------------------------------*/
{
    // This method sends the bytes and waits whenever the queue is full.
    unint count;
    ErrorT err;

    for (count = 0, err = cOk ; (count < length) && (err == cOk ); count++)
        err = outputSCC(-1, bytes[count]);

    return err;
}

/*---------------------------------------------------------------------------*/
ErrorT BeagleBoardUARTforHCI::handleIRQ()
/*---------------------------------------------------------------------------*/
{
    while (this->hasPendingData())
    {
        // check for out of bounds error
        if (currentPosition >= sizeof(packetBuffer))
        {
            // start all over again (we may miss a packet)
            currentPosition = 0;
            headerFound = false;
        }
        // read 1 byte and don't wait
        packetBuffer[currentPosition] = recvByte();
        currentPosition++;
    }

    // clear and re-enable interrupts
    this->interruptPending = false;
    this->enableIRQ();

    return (cOk );

}

/*---------------------------------------------------------------------------*/
ErrorT BeagleBoardUARTforHCI::send(packet_layer* packet, char* dest_addr, int addr_len, int2 fromProtocol_ID)
/*---------------------------------------------------------------------------*/
{
    return 0;
}

/*---------------------------------------------------------------------------*/
ErrorT BeagleBoardUARTforHCI::broadcast(packet_layer* packet, int2 fromProtocol_ID)
/*---------------------------------------------------------------------------*/

{
    return send(packet, 0, 0, fromProtocol_ID);
}

/*---------------------------------------------------------------------------*/
byte BeagleBoardUARTforHCI::recvByte()
/*---------------------------------------------------------------------------*/
{
    return IN8(baseAddr + UART_RHR_REG_OFFSET);
}

/*---------------------------------------------------------------------------*/
void BeagleBoardUARTforHCI::sendByte(byte Data)
/*---------------------------------------------------------------------------*/
{
    OUT8(baseAddr + UART_THR_REG_OFFSET, Data);
}

/*---------------------------------------------------------------------------*/
ErrorT BeagleBoardUARTforHCI::inputSCC(int4 Timeout, byte *c)
/*---------------------------------------------------------------------------*/
{
    int ret = cError;

    while (Timeout == -1 || Timeout--)
    {
        {
            if (hasPendingData())
            {
                *c = recvByte(); /* get char */
                ret = cOk;
                break;
            }
        }
    } /* end while */
    return ret;
}

/*---------------------------------------------------------------------------*/
ErrorT BeagleBoardUARTforHCI::outputSCC(int4 Timeout, byte c)
/*---------------------------------------------------------------------------*/
{
    int ret = cError;

    while (Timeout == -1 || Timeout--)
    {
        {
            if (!isTransmitBufferFull())
            {
                sendByte(c); /* output char */
                ret = cOk;
                break;
            }
        }
    } /* end while */
    return ret;
}

/*---------------------------------------------------------------------------*/
bool BeagleBoardUARTforHCI::isTransmitBufferFull()
/*---------------------------------------------------------------------------*/
{
    byte ret = ((IN8(baseAddr + UART_SSR_REG_OFFSET)) & 0x01);
    return (bool) ret;
}

/*---------------------------------------------------------------------------*/
bool BeagleBoardUARTforHCI::isReceiveBufferFull()
/*---------------------------------------------------------------------------*/
{
    return 0;
}

/*---------------------------------------------------------------------------*/
bool BeagleBoardUARTforHCI::hasPendingData()
/*---------------------------------------------------------------------------*/
{
    int ret = ((INW(baseAddr + UART_LSR_REG_OFFSET)) & 0x01);
    return (bool) ret;
}

