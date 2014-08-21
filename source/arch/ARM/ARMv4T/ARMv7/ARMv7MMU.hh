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

#ifndef ARMv7MMU_HH_
#define ARMv7MMU_HH_

#include "ARMv7PtEntry.hh"
#include "types.hh"

/*!
 *  \brief This class is an abstraction of the memory management unit of the ARMv7 architecture
 *
 */
class ARMv7MMU {
private:

public:

    /*!
     * \brief All existing TLB entries are set to not valid
     */
    /*static void invalidate() {
     asm volatile(
     ".align 4;"
     "mov    r0,pc;"
     "bx     r0;"
     ".code 32;"
     "MOV r0, #0;"
     "MCR p15, 0, r0, c8, c5, 0;" // Invalidate Inst-TLB
     "MCR p15, 0, r0, c8, c6, 0;" // Invalidate Data-TLB

     "add r0, pc,#1;"
     "bx  r0;"
     ".code 16;"
     :
     :
     : "r0"
     );

     }*/

};

#endif /* ARMv7MMU_HH_ */
