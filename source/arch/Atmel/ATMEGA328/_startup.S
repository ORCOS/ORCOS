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
 rjmp TIM2 ; Timer2 Compare B Handler
 rjmp TIM2 ; Timer2 Overflow Handler
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
   sts SPH, r16 			; Set Stack Pointer to top of RAM
   ldi r16, lo8(__stack)
   sts SPL, r16

   clr r1
   clr r0
	sei

   // copy the data from flash to ram
   rcall __copy_data

   //clear bss
   rcall __clear_bss

   rcall kernelmain
   nop
   jmp .

 EXT_INT0:
   rcall handleEXT_INT0
   reti

 EXT_INT1:
   rcall handleEXT_INT1
   reti

 WDT:
   rcall handleWatchdog
   reti

// timer may only happen for wakeup in sleep mode
// as we are a single threading environment
 TIM0:
   rcall handleTIM0
   reti

 TIM1:
   rcall handleTIM1
   reti

 TIM2:
   rcall handleTIM2
   reti

 NONE:
   rcall  invalidIRQ
   reti

__copy_data:
 	 ldi     r26, lo8(__data_start)
     ldi     r27, hi8(__data_start)
     ldi     r30, lo8(__data_load_start)
     ldi     r31, hi8(__data_load_start)
     rjmp    cmp
 loop:
 	 lpm     r24, Z
     st      X+, r24
     adiw    r30, 0x01
 cmp:
     ldi     r24, lo8(__data_load_end)
     ldi     r25, hi8(__data_load_end)
     cp      r30, r24
     cpc     r31, r25
     brlt    loop
     ret


__clear_bss:
	 ldi     XL, lo8(__bss_start)
     ldi     XH, hi8(__bss_start)
     rjmp    cmp2
 loop2:
     st      X+, r1
 cmp2:
     ldi     r24, lo8(__bss_end)
     ldi     r25, hi8(__bss_end)
     cp      XL, r24
     cpc     XH, r25
     brcs    loop2
     ret



