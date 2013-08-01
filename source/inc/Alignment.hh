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

#ifndef ALIGNMENT_HH_
#define ALIGNMENT_HH_
#include <types.hh>
#include <const.hh>

/*
 * The following methods are used to align a given memory address
 */

// align to the next lower address which can be divided by n
inline
byte* alignFloor( byte* addr, unint n ) {
    return (byte*) ( (unint) addr & ~( n - 1 ) );
}

// align to the next higher address which can be divided by n
inline
byte* alignCeil( byte* addr, unint n ) {
    return alignFloor( addr + ( n - 1 ), n );
}

// align address according to the user configuration, this method is called by Memory Manager
inline
byte* align( byte* addr, unint n ) {
#if(ALIGN_CEIL)
    return alignCeil( addr, n );
#else
    return alignFloor(addr,n);
#endif
}

#endif /*ALIGNMENT_HH_*/
