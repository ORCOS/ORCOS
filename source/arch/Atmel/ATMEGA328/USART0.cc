/*
 * USART0.cc
 *
 *  Created on: 04.05.2014
 *      Author: Daniel
 */

#include "USART0.hh"
#include "inc/types.hh"
#include "inc/const.hh"

#include "avr/io.h"
#include "inc/memio.h"
#include <kernel/Kernel.hh>
#include "avr/common.h"
#include "avr/pgmspace.h"

#if DATABITS_9
void USART_Transmit( unsigned int data )
{
	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0))) 	;
	/* Copy 9th bit to TXB8 */
	UCSR0B &= ~( 1<< TXB8 );

	if ( data & 0x0100 )
		UCSR0B |= ( 1<< TXB8 );

	/* Put data into buffer, sends the data */
	UDR0 = data;
}
#endif



#define FOSC 8000000 // Clock Speed
#define BAUD 38400
#define MYUBRR FOSC/16/BAUD-1

static int initialized = 0;
#define PRINT_BUF_LEN 12

extern unint2 __data_load_end; // for sanity check of printf

int puts_p(const void*  str);


void USART_Init( unsigned int ubrr)
{
    CLKPR = 128; // allow clock change!
    CLKPR = 0;   // divide by 1!

    for (volatile int i = 0; i < 100; i++);

    /*Set baud rate */
    UBRR0H = (unsigned char)(ubrr>>8);
    UBRR0L = (unsigned char) ubrr;

    /*Enable receiver and transmitter */
    UCSR0B = (0<<RXEN0)|(1<<TXEN0);

    /* Set frame format: 8data, 2stop bit */
    UCSR0C = (1<<USBS0)|(3<<UCSZ00);


    //UCSR0A |= (1<<1);
    initialized = 1;

    puts_p((void*) PSTR("\nUSUART0 initialized.\n"));
}



void USART_Transmit( unsigned char data )
{
    if (!initialized) USART_Init(MYUBRR);

	/* Wait for empty transmit buffer */
	while ( !( UCSR0A & (1<<UDRE0)) ) {

	}

	/* Put data into buffer, sends the data */
	UDR0 = data;


}

int puts_p(const void*  str) {
    unint2 s = (unint2) str;

    char c = pgm_read_byte(s);
    s++;

    while (c != 0) {
        USART_Transmit(c);
        c = pgm_read_byte(s);
        s++;
    }
    return (0);
}



int puts_d(char* str) {
	char* c = str;

	while (*c) {
		USART_Transmit(*c);
		c++;


	}
	return (0);
}





void uitoa_p(unsigned int value, char* str, int base) {
    char* wstr = str;
    const char* nums = PSTR("0123456789abcdefghijklmnopqrstuvwxyz");
    // Validate base
    if (base < 2 || base > 35)
    {
        *wstr = '\0';
        return;
    }

    // Conversion. Number is reversed.
    do
        *wstr++ = pgm_read_byte(nums  +(value % base));
    while (value /= base);

    *wstr = '\0';

    // Reverse string
    strreverse(str, wstr - 1);
}

int printf_p( const char *format, ... ) {


    va_list args;
    va_start(args, format);

    for (; ((unint2) format < ((unint2)&__data_load_end)) && pgm_read_byte(format) != 0; ++format)
    {
        if (pgm_read_byte(format) == '%')
        {
            ++format;

            if (pgm_read_byte(format) == '\0')
                break;

            if (pgm_read_byte(format) == '%')
                goto output;

            if (pgm_read_byte(format) == '-')
            {
                ++format;
            }

            for (; pgm_read_byte(format) >= '0' && pgm_read_byte(format) <= '9'; ++format)
            {

            }

            /*if (pgm_read_byte(format) == 's')
            {
                register char *s = va_arg(args, char*);
                puts_p(s);
                continue;
            }*/
            if (pgm_read_byte(format) == 'd')
            {
                char print_buf[ PRINT_BUF_LEN];
                uitoa_p(va_arg(args, int), print_buf, 10);
                puts_d(print_buf);
                continue;
            }
            if (pgm_read_byte(format) == 'u')
            {
                char print_buf[ PRINT_BUF_LEN];
                uitoa_p(va_arg(args, unsigned int), print_buf, 10);
                puts_d(print_buf);
                continue;
            }
            if (pgm_read_byte(format) == 'x')
            {
                char print_buf[ PRINT_BUF_LEN];
                uitoa_p(va_arg(args, unsigned int), print_buf, 16);
                puts_d(print_buf);
                continue;
            }
        }
        else
        {
            output: USART_Transmit(pgm_read_byte(format));
        }
    }

    va_end(args);
    return 0;
}


extern "C" void test() {
    puts_p((void*) PSTR("\nUSUART0 initialized.\n"));
}

USART0::USART0() {
    if (!initialized) USART_Init(MYUBRR);
}




USART0::~USART0() {

}



