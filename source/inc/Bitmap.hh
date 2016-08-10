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
#include "inc/error.h"

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

#define CEILING(x,y) (((x) + (y) - 1) / (y))
#define BITS_PER_LONG (sizeof(long)*8)

static inline int __clz(unsigned long val)
{
    if (!val) {
        return (BITS_PER_LONG);
    }
    return (__builtin_clzl(val));
}


/*****************************************************************************
 * Template class LargeBitmap.
 *
 * Stores a large bitmap inside an array of long words. Template
 * class to allow the compiler to generate optimal code for
 * array access.
 *
 *******************************************************************************/
template <int Bits>
class LargeBitmap {
private:
    unsigned long   bitmap[CEILING(Bits, BITS_PER_LONG)];

public:
    LargeBitmap() {
        for (unsigned int i = 0; i < CEILING(Bits, BITS_PER_LONG); i++) {
            bitmap[i] = 0;
        }
    }

    /*****************************************************************************
     * Method: setBit(int bit)
     *
     * @description
     *  Sets the bit inside the bitmap
     *
     * @params
     *  bit     bit to set
     *******************************************************************************/
    void setBit(int bit)
    {
        if (bit >= Bits) return;
        int word = bit / BITS_PER_LONG;
        bit = bit - (word * BITS_PER_LONG);
        bitmap[word] |= 1ul << bit;
    }

    /*****************************************************************************
     * Method: clearBit(int bit)
     *
     * @description
     *  Clears the bit inside the bitmap
     *
     * @params
     *  bit     bit to clear
     *******************************************************************************/
    void clearBit(int bit)
    {
        if (bit >= Bits) return;
        int word = bit/ BITS_PER_LONG;
        bit = bit - (word * BITS_PER_LONG);
        bitmap[word] &= ~(1ul << bit);
    }

    /*****************************************************************************
     * Method: isSet(int bit)
     *
     * @description
     *  Returns the state of the bit
     *
     * @params
     *  bit     bit to check
     *******************************************************************************/
    bool isSet(int bit)
    {
        if (bit >= Bits) return (false);
        int word = bit/BITS_PER_LONG;
        bit = bit - (word * BITS_PER_LONG);
        return ((bitmap[word] & (1ul << bit)) != 0);
    }

    /*****************************************************************************
     * Method: getFirstBit()
     *
     * @description
     *  Returns the first bit set or -1
     *
     * @returns -1 on failure. first set bit otherwise
     *
     *******************************************************************************/
    int getFirstBit()
    {
        // to be optimized (e.g. unrolled) by compiler as upper bound is known
        // at compile time
        for (unsigned int i = 0; i < CEILING(Bits, BITS_PER_LONG); i++) {
             // compiler will generate hardware ffs version here hopefully
             // be sure to specify -march= so compiler can really use architecture
             // specific asm instructions here
             int firstset = __builtin_ffsl(bitmap[i]);
             if (firstset != 0)
             {
                 return ((i * BITS_PER_LONG) + firstset-1);
             }
        }
        return (-1);
    }
};


/*****************************************************************************
 * Template class IDMap.
 *
 * Stores a set of ID inside an array of long words. Template
 * class to allow the compiler to generate optimal code for
 * array access.
 *
 *******************************************************************************/
template <int IDs>
class IDMap {
private:
    /* map of free IDs
     * 1: ID free, 0 : ID used */
    unsigned long  map[CEILING(IDs, BITS_PER_LONG)];

public:
    IDMap() {
        for (unsigned int i = 0; i < CEILING(IDs, BITS_PER_LONG); i++)
        {
            map[i] = (long)-1;
        }
        // remove last ids in last word not belonging to the map
        int lastIds = IDs % BITS_PER_LONG;
        map[CEILING(IDs, BITS_PER_LONG)-1] &= ~((1ul << (BITS_PER_LONG - lastIds))-1);
    }


    /*****************************************************************************
     * Method: getNextID(int bit)
     *
     * @description
     *  Returns the next id and removes the id from the map
     *
     * @returns the nest id or -1
     *******************************************************************************/
    int getNextID()
    {
        int id = -1;
        for (unsigned int i = 0; i < CEILING(IDs, BITS_PER_LONG); i++)
        {
           unsigned int freeindex = __clz(map[i]);
           if (freeindex < BITS_PER_LONG)
           {
               /* mark ID as used */
               map[i] &= ~(1ul << ((BITS_PER_LONG-1)-freeindex));
               id = freeindex + (i * BITS_PER_LONG);
               break;
           }
        }
        return (id);
    }

    /*****************************************************************************
     * Method: invalidateID(int bit)
     *
     * @description
     *  Invalidates the ID. The ID can not be returned by getNextID until freed.
     *
     * @params
     *  id     ID to invalidate
     *******************************************************************************/
    void invalidateID(int id)
    {
        if (id >= IDs) return;
        int index = id / BITS_PER_LONG;
        int bit = id - (index * BITS_PER_LONG);
        map[index] &= ~(1ul << ((BITS_PER_LONG-1)-bit));
    }

    /*****************************************************************************
     * Method: freeID(int bit)
     *
     * @description
     *  Frees the ID. The ID can be reused again.
     *
     * @params
     *  id     ID to free
     *******************************************************************************/
    void freeID(int id)
    {
        if (id >= IDs) return;
        int index = id / BITS_PER_LONG;
        int bit = id - (index * BITS_PER_LONG);
        map[index] |= (1ul << ((BITS_PER_LONG-1)-bit));
    }
};

#endif /* _BITMAP_HH */
