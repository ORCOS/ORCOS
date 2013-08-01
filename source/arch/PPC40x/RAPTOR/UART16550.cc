/*
 * UART16550.cc
 *
 *  Created on: 17.01.2012
 *      Author: kgilles
 */

#include <UART16550.hh>
#include <assembler.h>
#include "comm/AddressProtocol.hh"
#include <kernel/Kernel.hh>
#include "kernel/Kernel.hh"
#include "inc/memio.h"

extern Kernel* theOS;

/*---------------------------------------------------------------------------*/
UART16550::~UART16550()
/*---------------------------------------------------------------------------*/
{

}

/*---------------------------------------------------------------------------*/
UART16550::UART16550( const char* name, int4 a ) :
    CommDeviceDriver( name )
/*---------------------------------------------------------------------------*/
{

	addr = a;
	volatile unsigned char *port = (unsigned char *) addr;
	unsigned long		divisor;
	char				lcr;

	// set baud rate
	unsigned long 		baud_rate = 9600;

	_disableInterrupts();

	// Set DLAB bit in order to access the baud rate divisor registers.
	lcr=IN8(port + UART_LCR_OFFSET);
	OUT8(port + UART_LCR_OFFSET, lcr|UART_LCR_DLAB);

	// The output frequency of the baud generator is 16 times the baud.  Eight
	// is added to compensate for the rounding to integer.
	divisor=(CLOCK_RATE/ baud_rate)/ 16;
	OUT8(port + UART_DLL_OFFSET, (divisor&0xFF));
	OUT8(port + UART_DLM_OFFSET, ((divisor&0xFF00)>>8));

	// Clear DLAB bit.
	OUT8(port + UART_LCR_OFFSET, lcr&(~UART_LCR_DLAB));

	_enableInterrupts();

	/*-------------------------------------------------------------------------+
	| Set other UART registers.
	+-------------------------------------------------------------------------*/
	OUT8(port + UART_IER_OFFSET, UART_IER_DISABLE_ALL);
	OUT8(port + UART_FCR_OFFSET, UART_FCR_FIFO_ENABLE|UART_FCR_FIFO_R_FIFO_RES|UART_FCR_FIFO_T_FIFO_RES|UART_FCR_FIFO_TRIG_1);
	OUT8(port + UART_LCR_OFFSET, UART_LCR_WORD_LENGTH8|UART_LCR_STOP_BITSONE|UART_LCR_PARITY_DISABLE|UART_LCR_ODD_PARITY);
	OUT8(port + UART_MCR_OFFSET, UART_MCR_OUT2|UART_MCR_OUT1|UART_MCR_RTS|UART_MCR_DTR);

}

/*---------------------------------------------------------------------------*/
ErrorT UART16550::enableIRQ()
/*---------------------------------------------------------------------------*/
{
	return 0;
}

/*---------------------------------------------------------------------------*/
ErrorT UART16550::disableIRQ()
/*---------------------------------------------------------------------------*/
{
	return 0;
}

/*---------------------------------------------------------------------------*/
ErrorT UART16550::readByte( char* byteptr )
/*---------------------------------------------------------------------------*/
{
    inputSCC( 1, (byte*) byteptr );
    return cOk;
}

/*---------------------------------------------------------------------------*/
ErrorT UART16550::writeByte( char byte )
/*---------------------------------------------------------------------------*/
{
    outputSCC( -1, byte );
    return cOk;
}

/*---------------------------------------------------------------------------*/
ErrorT UART16550::readBytes( char* bytes, unint4 &length )
/*---------------------------------------------------------------------------*/
{
    // This method polls (no waiting) for new data

    ErrorT err;
    unint4 count;

    for ( count = 0, err = cOk; ( ( count < length ) && ( err == cOk ) ); count++ )
        err = inputSCC( 1, (byte*) bytes + count );
    length = ( err == cOk ) ? count : count - 1;
    return err;
}

/*---------------------------------------------------------------------------*/
ErrorT UART16550::writeBytes( const char* bytes, unint4 length )
/*---------------------------------------------------------------------------*/
{
    // This method sends the bytes and waits whenever the queue is full.
    unint count;
    ErrorT err;

    for ( count = 0, err = cOk; ( count < length ) && ( err == cOk ); count++ )
        err = outputSCC( -1, bytes[ count ] );

    return err;

}

/*---------------------------------------------------------------------------*/
void UART16550::sendByte( byte Data )
/*---------------------------------------------------------------------------*/
{
    volatile unsigned char *port = (unsigned char *) addr;
    OUT8(port + UART_TX_OFFSET, (word)Data);
    __asm__ volatile ("eieio");

}

/*---------------------------------------------------------------------------*/
bool UART16550::isTransmitRegEmpty()
/*---------------------------------------------------------------------------*/
{
    volatile unsigned char *port = (unsigned char *) addr;
    volatile unsigned char *linestat=(unsigned char *)(port + UART_LSR_OFFSET);
    __asm__ volatile ("eieio");
    return (((*linestat)&UART_LSR_TX_EMPTY)==UART_LSR_TX_EMPTY);
}

/*---------------------------------------------------------------------------*/
bool UART16550::isReceiveBufferFull()
/*---------------------------------------------------------------------------*/
{
    /*volatile unsigned char *port = (unsigned char *) addr;
    word reg = INW(port + OPBUL_STATUS_REG_OFFSET);
    __asm__ volatile ("eieio");
    return ( ( reg & OPBUL_SR_RX_FIFO_FULL ) == OPBUL_SR_RX_FIFO_FULL );*/
	return 0;
}

/*---------------------------------------------------------------------------*/
byte UART16550::recvByte()
/*---------------------------------------------------------------------------*/
{
    volatile unsigned char *port = (unsigned char *) addr;

    return (byte) IN8(port + UART_RX_OFFSET);
}

/*---------------------------------------------------------------------------*/
ErrorT UART16550::inputSCC( int4 Timeout, byte *c )
/*---------------------------------------------------------------------------*/
{
    int ret = cError;

    while ( Timeout == -1 || Timeout-- ) {
        {
            if ( hasPendingData() ) {
                *c = recvByte(); /* get char */
                ret = cOk;
                break;
            }
        }
    } /* end while */
    return ret;
}

/*---------------------------------------------------------------------------*/
bool UART16550::hasPendingData()
/*---------------------------------------------------------------------------*/
{
    volatile unsigned char *port = (unsigned char *) addr;
    volatile unsigned char *linestat=(unsigned char *)(port + UART_LSR_OFFSET);
    __asm__ volatile ("eieio");
    return (((*linestat)&UART_LSR_DATA_READY)==UART_LSR_DATA_READY);
}

/*---------------------------------------------------------------------------*/
ErrorT UART16550::outputSCC( int4 Timeout, byte c )
/*---------------------------------------------------------------------------*/
{
    int ret = cError;

    while ( Timeout == -1 || Timeout-- ) {
        {
            if ( isTransmitRegEmpty() ) {
                sendByte( c ); /* output char */
                ret = cOk;
                break;
            }
        }
    } /* end while */
    return ret;
}

ErrorT UART16550::send( packet_layer* packet, char* dest_addr, int addr_len, int2 fromProtocol_ID ) {

	return cOk;
}

ErrorT UART16550::broadcast( packet_layer* packet, int2 fromProtocol_ID ) {
    return send(packet, 0, 0,fromProtocol_ID );

}

void UART16550::recv() {

}

