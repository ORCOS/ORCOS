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

#include <SCLConfig.hh>
#include "avr/io.h"
#include "avr/common.h"

// Interrupt verctor section
.section	.iv
.org 0x0000
 rjmp RESET ; Reset Handler
 rjmp EXT_INT0 ; IRQ0 Handler
 rjmp EXT_INT1 ; IRQ1 Handler
 rjmp NONE ; PCINT0 Handler
 rjmp NONE ; PCINT1 Handler
 rjmp NONE ; PCINT2 Handler
 rjmp WDT ; Watchdog Timer Handler
 rjmp TIM2 ; Timer2 Compare A Handler
 rjmp NONE ; Timer2 Compare B Handler
 rjmp NONE ; Timer2 Overflow Handler
 rjmp NONE ; Timer1 Capture Handler
 rjmp TIM1 ; Timer1 Compare A Handler
 rjmp NONE ; Timer1 Compare B Handler
 rjmp NONE ; Timer1 Overflow Handler
 rjmp TIM0 ; Timer0 Compare A Handler
 rjmp NONE ; Timer0 Compare B Handler
 rjmp NONE ; Timer0 Overflow Handler
 rjmp NONE ; SPI Transfer Complete Handler
 rjmp NONE ; USART, RX Complete Handler
 rjmp NONE ; USART, UDR Empty Handler
 rjmp NONE ; USART, TX Complete Handler
 rjmp NONE ; ADC Conversion Complete Handler
 rjmp NONE ; EEPROM Ready Handler
 rjmp NONE ; Analog Comparator Handler
 rjmp NONE ; 2-wire Serial Interface Handler (TWI)
 rjmp NONE ; Store Program Memory Ready Handler

.globl RESET
 RESET:
 ldi r16, hi8(__stack)  	; Main program start
 sts SPH,r16 			; Set Stack Pointer to top of RAM
 ldi r16, lo8(__stack)
 sts SPL,r16

 // copy the data from flash to ram
 rcall __copy_data

// clear bss
 rcall __clear_bss

 rcall kernelmain
 jmp .

 EXT_INT0:
   jmp .

 EXT_INT1:
   jmp .

 WDT:
   jmp .

 TIM0:
   jmp .

 TIM1:
   jmp .

 TIM2:
   jmp .

 NONE:
   jmp .


__copy_data:
	ldi     r17, hi8(__data_end)
	ldi     r26, lo8(__data_start) ; X low
	ldi     r27, hi8(__data_start) ; X high
	ldi     r30, lo8(__data_load_start) ; Z low
	ldi     r31, hi8(__data_load_end) ; Z high
	rjmp    .+6
	lpm  				; load from program memory (Z) into r0
	adiw    r30, 0x01
	st      X+, r0		; store loaded byte to X then inc X
	cpi     r26, lo8(__data_end)
	cpc     r27, r17
	brne	.-12
	ret

__clear_bss:
	ret


