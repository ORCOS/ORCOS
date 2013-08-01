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

#ifndef WINDOWS_HH_
#define WINDOWS_HH_

/*
 * Stores the content of a window to its stack
 */
/*#define STORE_WINDOW(reg) 	\
	/*std	%l0, [%reg +  0];	\
	std	%l2, [%reg +  8];	\
	std	%l4, [%reg + 16];	\
	std	%l6, [%reg + 24];	\
	std	%i0, [%reg + 32];	\
	std	%i2, [%reg + 40];	\
	std	%i4, [%reg + 48];	\
	std	%i6, [%reg + 56];*/

#define STORE_WINDOW(reg) 	\
    st	%l0, [%reg +  0];	\
    st	%l1, [%reg +  4];	\
	st	%l2, [%reg +  8];	\
	st	%l3, [%reg + 12];	\
	st	%l4, [%reg + 16];	\
	st	%l5, [%reg + 20];	\
	st	%l6, [%reg + 24];	\
	st	%l7, [%reg + 28];	\
	st	%i0, [%reg + 32];	\
	st	%i1, [%reg + 36];	\
	st	%i2, [%reg + 40];	\
	st	%i3, [%reg + 44];	\
	st	%i4, [%reg + 48];	\
	st	%i5, [%reg + 52];	\
	st	%i6, [%reg + 56];   \
	st	%i7, [%reg + 60];	

/*
 * Loads the window contents from its stack
 */
/*#define LOAD_WINDOW(reg) 	\
	ldd	[%reg +  0], %l0;	\
	ldd	[%reg +  8], %l2;	\
	ldd	[%reg + 16], %l4;	\
	ldd	[%reg + 24], %l6;	\
	ldd	[%reg + 32], %i0;	\
	ldd	[%reg + 40], %i2;	\
	ldd	[%reg + 48], %i4;	\
	ldd	[%reg + 56], %i6;*/
	
#define LOAD_WINDOW(reg)	\
	ld	[%reg +  0], %l0;	\
	ld	[%reg +  4], %l1;	\
	ld	[%reg +  8], %l2;	\
	ld	[%reg + 12], %l3;	\
	ld	[%reg + 16], %l4;	\
	ld	[%reg + 20], %l5;	\
	ld	[%reg + 24], %l6;	\
	ld	[%reg + 28], %l7;	\
	ld	[%reg + 32], %i0;	\
	ld	[%reg + 36], %i1;	\
	ld	[%reg + 40], %i2;	\
	ld	[%reg + 44], %i3;	\
	ld	[%reg + 48], %i4;	\
	ld	[%reg + 52], %i5;	\
	ld	[%reg + 56], %i6;	\
	ld	[%reg + 60], %i7;	

/*
 * Stores the content of used windows to the stack.
 * (by using the window overflow handler) 
 */
#define PUSH_WINDOWS_TO_STACK \
	save; save; save; save; save; save; \
	restore; restore; restore; restore; restore; restore;  
	
#endif /* WINDOWS_HH_ */
