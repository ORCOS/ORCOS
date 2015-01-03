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

#ifndef SOURCE_ARCH_ARM_ARMV4T_ASSEMBLER_H_
#define SOURCE_ARCH_ARM_ARMV4T_ASSEMBLER_H_

#define _GLOBAL(n) .globl n

#define FUNC_START(name) name:
#define FUNC_NAME(name) name

#define EXPORT_VAR(type, var)           _GLOBAL(var)
#define EXPORT_FCN(type, fcnname, args) _GLOBAL(fcnname)

#endif // SOURCE_ARCH_ARM_ARMV4T_ASSEMBLER_H_
