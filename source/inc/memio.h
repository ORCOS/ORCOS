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

#ifndef _ASSEMBLER_HH
#define _ASSEMBLER_HH

//#define __force        __attribute__((force))
#define __force

extern "C"
{

///  The following definitions for IN and OUT should
///  work when the registers are memory-mapped.
#define	IN8(p)		(*(volatile unsigned char __force *)(p))
#define	INB(p)		(*(volatile unsigned char __force *)(p))
#define	INH(p)		(*(volatile unsigned short __force *)(p))
#define	INW(p)		(*(volatile unsigned int  __force *)(p))

#define	OUT8(p,d)	(*(volatile unsigned char __force *)(p) = (d))
#define	OUTB(p,d)	(*(volatile unsigned char __force *)(p) = (d))
#define	OUTH(p,d)	(*(volatile unsigned short __force *)(p) = (d))
#define	OUTW(p,d)	(*(volatile unsigned int __force  *)(p) = (d))

}

#endif /* _ASSEMBLER_HH */

