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

#ifndef _UART_HH
#define _UART_HH

/// Either interrupt-driven input, interrupt-driven output,
/// or both may be enabled by setting USE_RXIRQ or USE_TXIRQ,
/// respectively, to 1.
/// int-driven transmit?  
#define USE_TXIRQ		(0)		

/// int-driven receive?  
#define USE_RXIRQ		INTERRUPTS_Cfd		

/// Define buffer sizes when IRQ buffered IO is used
#define IN_BUF_SIZE		cIOExceptionBufferSize
#define OUT_BUF_SIZE		cIOExceptionBufferSize

/// auto RTS/CTS handling?  
#define HW_FLOW_CONTROL		(1)		

/// Set the baud rate at which the UART test will run.
#define TEST_BAUD_RATE		kBaud38400

/// Setting POLLING_BUFFER to (1) will cause the polling
/// routines to place excess input into a memory buffer.
/// This reduces the chances of dropped characters when
/// using polled input, but it requires an additional 4k
/// of memory for the buffer.
#define POLLING_BUFFER		(0)		/* buffer polled input? */
#define IN_BUFFER_SIZE		cPollingBufferSize

/// Enumerate supported clock speeds 
#define CLOCK_1_8432MHZ		0
#define CLOCK_3_6864MHZ		1
#define CLOCK_4MHZ		2
#define CLOCK_7MHZ		3
#define CLOCK_16MHZ		4
#define CLOCK_20MHZ		5
#define CLOCK_20_8896MHZ	6
#define CLOCK_22_1184MHZ	7
#define CLOCK_24MHZ		8
#define CLOCK_40MHZ		9
#define CLOCK_55_296MHZ		10

/// Use the compiler endianness option setting.
#define TARGET_BIG_ENDIAN	(!__option( little_endian ))

/// Define operating mode masks for mode flag in init routine 
#define cIntDrivenInput			0x0001
#define cIntDrivenOutput		0x0002
#define cInstallIntHandler		0x0004
/// Additional mode masks not used in init routine
#define cIdleLineInt                    0x0008


typedef enum {
  /// use HW settings such as DIP switches  
  kBaudHWSet = -1,	
	
  /// valid baud rates  
  kBaud300 = 300,		
  kBaud600 = 600,
  kBaud1200 = 1200,
  kBaud1800 = 1800,
  kBaud2000 = 2000,
  kBaud2400 = 2400,
  kBaud3600 = 3600,
  kBaud4800 = 4800,
  kBaud7200 = 7200,
  kBaud9600 = 9600,
  kBaud19200 = 19200,
  kBaud38400 = 38400,
  kBaud57600 = 57600,
  kBaud115200 = 115200,
  kBaud230400 = 230400
} UARTBaudRate;



#endif /* _UART_HH */

