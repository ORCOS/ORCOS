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

#include "PPC405TlbEntry.hh"

void PPC405TlbEntry::setEffectivePage(BitmapT b) {
    tag.clearBits(0xFFFFFC00);
    byte bits = 22 - (getSize() << 1);
    BitmapT mask = ~(0xffffffff >> bits);
    tag.setBits(b & mask);
}

BitmapT PPC405TlbEntry::getEffectivePage() {
    byte bits = 22 - (getSize() << 1);
    BitmapT mask = ~(0xffffffff >> bits);
    return tag & mask;
}

void PPC405TlbEntry::setRealPage(BitmapT b) {
    data.clearBits(0xFFFFFC00);
    byte bits = 22 - (getSize() << 1);
    BitmapT mask = ~(0xffffffff >> bits);
    data.setBits(b & mask);
}

BitmapT PPC405TlbEntry::getRealPage() {
    byte bits = 22 - (getSize() << 1);
    BitmapT mask = ~(0xffffffff >> bits);
    return data & mask;
}
