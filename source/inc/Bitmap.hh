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

#ifndef _BITMAP_HH
#define _BITMAP_HH

#include "inc/types.hh"

///  \brief Little endian bitmap.
///
///  <h3>Class Description</h3>
///
class Bitmap {
private:
    BitmapT Flags;

public:
    ///  Default constructor
    Bitmap() :
            Flags(0) {
    }
    Bitmap(BitmapT b) :
            Flags(b) {
    }

    ///  Sets the bit numbered 'bitNr'
    inline void setBit(int bitNr) {
        Flags |= (1 << bitNr);
    }

    ///  Set all bits of the given 'bitmap' in this bitmap
    inline void setBits(int bitmap) {
        Flags |= bitmap;
    }

    ///  Returns the unsigned integer value of this bitmap
    inline BitmapT getBits() const {
        return (Flags);
    }

    ///  Returns the bitmap masked by the given 'mask'
    inline BitmapT getBits(BitmapT mask) const {
        return (Flags & mask);
    }

    inline BitmapT isSet(int bitNr) const {
        return (Flags & (1 << bitNr));
    }

    inline int areSet(int bitmap) const {
        return ((Flags & bitmap) == (BitmapT) bitmap);
    }

    inline int areSomeSet(int bitmap) const {
        return (Flags & bitmap);
    }

    ///  Returns true iff any bit of the given 'mask' is set in this bitmap
    inline BitmapT anySet(BitmapT mask) const {
        return (getBits(mask));
    }

    ///  Clears all bits in the bitmap
    inline void clear() {
        Flags = 0;
    }

    ///  Clears the bit numbered 'bitNr'
    inline void clearBit(int bitNr) {
        Flags &= ~(1 << bitNr);
    }

    ///  Clears all bits of the given 'bitmap' in this bitmap
    inline void clearBits(int bitmap) {
        Flags &= ~bitmap;
    }

    ///  Sets the bitmap to 'x'
    inline void set(unint x) {
        Flags = x;
    }

    ///  Returns true iff no bit is set
    inline bool isEmpty() const {
        return (Flags == 0);
    }

    inline operator BitmapT() const {
        return (Flags);
    }

    inline ErrorT operator=(unint x) {
        Flags = x;
        return (cOk);
    }

    inline ErrorT operator=(Bitmap* b) {
        *this = *b;
        return (cOk);
    }

};

#endif /* _BITMAP_HH */
