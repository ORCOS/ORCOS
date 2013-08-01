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

#ifndef _SEGMENTDISPLAY_HH
#define _SEGMENTDISPLAY_HH

#include <error.hh>
#include <types.hh>
#include "../powerpc.h"

#ifndef AVNET_7SEG_BASE
#define AVNET_7SEG_BASE		0x80000400
#endif

#define AVNET_7SEG_ADDR(X)	((volatile unsigned *) (AVNET_7SEG_BASE+(X)))
#define AVNET_7SEG_REG(X)	(*(AVNET_7SEG_ADDR(X)))

#define AVNET_7SEG_REVISION     AVNET_7SEG_REG(0x000c)
#define AVNET_7SEG_DIGITS       AVNET_7SEG_REG(0x0000)
#define AVNET_7SEG_DESCRETES    AVNET_7SEG_REG(0x0004)

#define AVNET_HELO		1

static unint1 fgSegments[] = { 0x3f, 0x06, 0x5b, 0x4f, 0x66, 0x6d, 0x7d, 0x07, 0x7f, 0x6f, 0x77, 0x7c, 0x39, 0x5e,
        0x79, 0x71 };

/*!
 *  \brief provides access to the Virtex2 board's segment display
 */
class SegmentDisplay {
private:
    RegisterT Digits;

protected:
    ErrorT driveSegments( unword val ) {
        AVNET_7SEG_DIGITS = Digits = val;
        //__asm__ volatile ("eieio");
        return cOk;
    }

    ErrorT driveSegment( unint1 segNo, unint1 val );

public:
    SegmentDisplay();

    ~SegmentDisplay() {
    }

    ErrorT clear() {
        return driveSegments( 0 );
    }

    ErrorT overflow() {
        return driveSegments( 0x40404040 );
    }

    ErrorT driveDigit( unint1 segNo, unint1 val ) {
        if ( val < 16 )
            return driveSegment( segNo, fgSegments[ val ] );
        else
            return cError;
    } //formerly cInvalidArgument

    ErrorT driveDecValue( unword val );

    ErrorT driveHexValue( unword val );

};

#endif /* _SEGMENTDISPLAY_HH */
