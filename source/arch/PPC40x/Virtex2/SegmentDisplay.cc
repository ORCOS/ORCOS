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

#include <arch/PPC40x/Virtex2/SegmentDisplay.hh>

SegmentDisplay::SegmentDisplay() {
    overflow();
}

ErrorT SegmentDisplay::driveSegment(unint1 segNo, unint1 val) {
    unint1 shift = 24 - (segNo << 3);
    Digits &= ~(0xff << shift);
    Digits |= val << shift;
    return driveSegments(Digits);
}

ErrorT SegmentDisplay::driveDecValue(unword val) {
    if (val > 9999) {
        overflow();
        //return cOutOfRange;
        return cError ;
    } else {
        for (int4 i = 0; i < 4; i++) {
            driveDigit(i, val % 10);
            val /= 10;
        }
    }
    return cOk ;
}

ErrorT SegmentDisplay::driveHexValue(unword val) {
    if (val > 0xffff) {
        overflow();
        //return cOutOfRange;
        return cError ;
    } else {
        for (int4 i = 0; i < 4; i++) {
            driveDigit(i, val % 16);
            val /= 16;
        }
    }
    return cOk ;
}

