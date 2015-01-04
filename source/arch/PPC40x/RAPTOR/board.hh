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

#ifndef BOARD_HH_
#define BOARD_HH_

#define cStartOfRAM        0x00000000
#define cStartOfROM        0x00800000
#define cBootRAM_Size      (4 MByte)
#define cBootROM_Size      (4 MByte)
#define cSharedMemSize     0

#define cTopOfBootMemAddr  (__stack - cSharedMemSize)

#endif /*BOARD_HH_*/
