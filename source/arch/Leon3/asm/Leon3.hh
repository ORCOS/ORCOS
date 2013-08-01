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

#ifndef LEON3_HH_
#define LEON3_HH_

//instruction definitions for trap table

#define SPARC_BRANCH(destAddr, instAddr) \
          (0x10800000 | (((destAddr-instAddr)>>2)&0x3fffff))
 
#define SPARC_READ_PSR_TO_L0  (0xa1480000)

#define SPARC_NOP (0x01000000)


//Traps

#define TT_BASE_ADRESS		0x40000000	 /* Trap Table Base Address */	
#define WINDOW_OVERFLOW     0x5          /* Window Overflow */
#define WINDOW_UNDERFLOW    0x6          /* Window Underflow */



/*
 * General Purpose Timer Unit
 */


/*
 * Prescaler
 */
#define PSVR		0x80000300		// prescaler value
#define PSRLR		0x80000304		// prescaler reload value


/*
 * General Purpose Timer Unit Configuration Register (GPTCR)
 *
 * +--------------------------------------------+--+--+----------+------+																				
 * |                    zero					|DF|SI|   IRQ    |TIMERS|
 * +--------------------------------------------+--+--+----------+------+
 *                     31:10                     9  8     7:3      2:0 * 
 */
#define GPTCR			0x80000308		/* address of GPTCR */
#define GPTCR_DF		0x00000200		/* disable timer freeze */
#define GPTCR_SI		0x00000100		/* seperate interrupts for each timer */
#define GPTCR_IRQ		0x000000F8		/* read-only */
#define GPTCR_TIMERS	0x00000007		/* number of implemented timers ( read-only) */


/*
 * Timer Control Register (TCR1 - TCRn)
 * 
 * +------------------------------------------------------------+--+--+--+--+--+--+																				
 * |                             zero							|DH|CH|IP|LD|RS|EN|
 * +------------------------------------------------------------+--+--+--+--+--+--+
 */
#define TCR1		0x80000318		/* control register timer 1 */
#define TCR2		0x80000328		/* control register timer 2 */
#define TCR3		0x80000338		/* control register timer 3; last timer acts as watchdog */

#define VALUE_REG_OFFSET	0x0		/* contains the current timer value */
#define RELOAD_REG_OFFSET	0x4		/* contains the reload value */
#define CTRL_REG_OFFSET		0x8		/* timer control register */

#define TCR_DH		0x00000040		/* Debug halt; freeze counter in debug mode (read-only) */
#define TCR_CH		0x00000020		/* chain with preceding timer */
#define TCR_IP		0x00000010		/* set when an interrupt is signaled */
#define TCR_IE		0x00000008		/* interrupt enabled */
#define TCR_LD		0x00000004		/* load value from reload register */
#define TCR_RS		0x00000002		/* Restart: If set timer restarts on overflow */
#define TCR_EN		0x00000001		/* Enable Timer */

/*
 * Timer counter value register (32 Bit)
 */
#define TCVR1		0x80000310 /* counter value register timer 1 */
#define TCVR2		0x80000320 /* counter value register timer 2 */
#define TCVR3		0x80000330 /* counter value register timer 3 */


/*
 * Timer reload value register (32 Bit)
 */
#define TRLVR1		0x80000314 /* reload value register timer 1 */
#define TRLVR2		0x80000324 /* reload value register timer 2 */
#define TRLVR3		0x80000334 /* reload value register timer 3 */

/*
 * Interrupt Controller
 */

#define IMASK		0x80000240 /* Interrupt Mask Register */


#endif /*LEON3_HH_*/
