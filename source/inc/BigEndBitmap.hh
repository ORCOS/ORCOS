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

#ifndef BIGENDBITMAP_HH_
#define BIGENDBITMAP_HH_

#include "inc/types.hh"

///  \brief Big endian bitmap.
///
///  <h3>Class Description</h3>
///
class BigEndBitmap {
private:
    BitmapT Flags;

    int bitNr(int i) const {
        return (sizeof(Flags) << 3) - i - 1;
    }
protected:

public:
    ///  Default constructor
    BigEndBitmap() :
            Flags(0) {
    }
    BigEndBitmap(BitmapT b) :
            Flags(b) {
    }

    void setBit(int n) {
        Flags |= (1 << bitNr(n));
    }

    void setBits(BitmapT bitmap) {
        Flags |= bitmap;
    }
    int isSet(int n) const {
        return Flags & (1 << bitNr(n));
    }
    int areSet(BitmapT bitmap) const {
        return (Flags & bitmap) == bitmap;
    }
    int areSomeSet(int bitmap) const {
        return Flags & bitmap;
    }

    void clearBit(int n) {
        Flags &= ~(1 << bitNr(n));
    }

    void clearBits(int bitmap) {
        Flags &= ~bitmap;
    }

    void set(unint x) {
        Flags = x;
    }
    bool isEmpty() {
        return Flags == 0;
    }
    void clear() {
        Flags = 0;
    }

    operator BitmapT() const {
        return Flags;
    }
    void operator=(unint x) {
        Flags = x;
    }
};
#endif /*BIGENDBITMAP_HH_*/
