/*
 ORCOS - an Organic Reconfigurable Operating System
 Copyright (C) 2011 University of Paderborn

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

#ifndef ARMv7PtEntry_HH_
#define ARMv7PTEntry_HH_

#include <types.hh>
#include <Bitmap.hh>

#define ptTypeSection 0      /* 1 MB page */
#define ptTypeSuperSection 1 /* 16 MB page */
#define ptTypeL2PageTable  2 /* Link to a Level2 page table containing smaller pages */
#define ptTypeLargePage 3    /* 64 Kb page*/
#define ptTypeSmallPage 4    /* 4 Kb page*/

typedef struct {
    unint1 type         : 5;
    unint1 domain       : 4;
    unint1 impl_defined : 1;
    unint4 base_address : 22;
}__attribute__((packed)) ARMv7PtEntry_PageTable_t;

typedef struct {
    unint1 type         : 2; // must be 0x2
    unint1 B_bit        : 1;
    unint1 C_bit        : 1;
    unint1 XN_bit       : 1;
    unint1 domain       : 4;
    unint1 impl_defined : 1;
    unint1 AP1_bits     : 2;
    unint1 TEX_bits     : 3;
    unint1 AP2_bit      : 1;
    unint1 S_bit        : 1;
    unint1 nG_bit       : 1;
    unint1 null_bit     : 1;
    unint1 ns_bit       : 1;
    unint4 base_address : 12;
} __attribute__((packed)) ARMv7PtEntry_Section_t;

typedef struct {
    unint1 type         : 2; // must be 0x1
    unint1 B_bit        : 1;
    unint1 C_bit        : 1;
    unint1 AP1_bits     : 2;
    unint1 SBZ_bits     : 3;
    unint1 AP2_bit      : 1;
    unint1 S_bit        : 1;
    unint1 nG_bit       : 1;
    unint1 TEX_bits     : 3;
    unint1 XN_bit       : 1;
    unint4 base_address : 16;
} __attribute__((packed)) ARMv7PtEntry_LargePage_t;

typedef struct {
    unint1 XN_bit       : 1;
    unint1 type         : 1; // must be 1
    unint1 B_bit        : 1;
    unint1 C_bit        : 1;
    unint1 AP1_bits     : 2;
    unint1 TEX_bits     : 3;
    unint1 AP2_bit      : 1;
    unint1 S_bit        : 1;
    unint1 nG_bit       : 1;
    unint4 base_address : 20;
} __attribute__((packed)) ARMv7PtEntry_SmallPage_t;

typedef union {
    unint4 raw_bytes;
    ARMv7PtEntry_PageTable_t page_table;
    ARMv7PtEntry_Section_t   section;
    ARMv7PtEntry_LargePage_t large_page;
    ARMv7PtEntry_SmallPage_t small_page;
} ARMv7PtEntry_t;


#endif /* ARMv7PtEntry_HH_ */
