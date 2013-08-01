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

#include "windows.hh"

#define tmp_psr		l0
#define tmp_wim		l3
#define tmp_pc		l1
#define tmp_npc		l2

	.text
	.align	4
	
	.globl	window_overflow_handler
window_overflow_handler:
	mov %wim, %tmp_wim
	
	//rotate WIM one bit to the right
	srl %tmp_wim, 1, %l4
	sll %tmp_wim, 7, %l5
	or	%l4, %l5, %tmp_wim
	and  %tmp_wim, 0xFF, %tmp_wim
	
	// disable WIM traps
    mov %g0,%wim
    nop; nop; nop
    
    //go to next window
    save
    
	//save current window
	STORE_WINDOW(sp)	
	
	//go back to previous window
	restore
	
	//set WIM
	mov %tmp_wim,%wim
	nop; nop; nop
	
	//return from trap and reexecute the instruction which
	//caused the trap
	jmpl %tmp_pc, %g0
	rett %tmp_npc

	
	
	.globl	window_underflow_handler
window_underflow_handler:
	mov %wim, %tmp_wim
	
	//rotate WIM one bit to the left
	srl %tmp_wim, 7, %l4
	sll %tmp_wim, 1, %l5
	or	%l4, %l5, %tmp_wim
	and  %tmp_wim, 0xFF, %tmp_wim
	
	// disable WIM traps
    mov %g0,%wim
    nop; nop; nop
    
    //go to the window to load
    restore
    restore
    
	//load current window
	LOAD_WINDOW(sp)
	
	//back to trap window
	save
	save
	
	//set WIM
	mov %tmp_wim,%wim
	nop; nop; nop
	
	//return from trap and reexecute the instruction which
	//caused the trap
	jmpl %tmp_pc, %g0
	rett %tmp_npc

	
