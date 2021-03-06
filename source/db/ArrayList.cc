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

#include "ArrayList.hh"
#include "kernel/Kernel.hh"

// if we are sure a -b is never > b we can use this!
#define MODULO(a, b)  a >= b ? a-b : a

ArrayList::ArrayList(unint1 max) {
    maxEntries = max;
    numEntries = 0;
    entries = new ListItem*[max];

    for (int i = 0; i < maxEntries; i++) {
        entries[i] = 0;
    }

    headPointer = 0;
}

ArrayList::~ArrayList() {
    delete[] entries;
}

/*****************************************************************************
 * Method: ArrayList::getTail()
 *
 * @description
 *
 *******************************************************************************/
ListItem* ArrayList::getTail() {
    if (numEntries == 0)
        return (0);
    else
        return (entries[MODULO((headPointer + numEntries - 1), maxEntries)]);
}

/*****************************************************************************
 * Method: ArrayList::addTail(ListItem* item)
 *
 * @description
 *
 *******************************************************************************/
ErrorT ArrayList::addTail(ListItem* item) {
    if (numEntries + 1 > maxEntries) {
        return (cDatabaseOverflow );
    }

    int ptr = MODULO((headPointer + numEntries), maxEntries);
    numEntries++;
    entries[ptr] = item;

    return (cOk );
}

/*****************************************************************************
 * Method: ArrayList::addHead(ListItem* item)
 *
 * @description
 *
 *******************************************************************************/
ErrorT ArrayList::addHead(ListItem* item) {
    if (numEntries + 1 > maxEntries) {
        return (cDatabaseOverflow );
    }

    if (headPointer == 0) {
        //addHead only grows to the left side,
        //so check whether it goes over 0
        headPointer = (unint1) (maxEntries - 1);
    } else {
        headPointer--;
    }

    entries[headPointer] = item;
    numEntries++;

    return (cOk );
}

/*****************************************************************************
 * Method: ArrayList::removeHead()
 *
 * @description
 *
 *******************************************************************************/
ListItem* ArrayList::removeHead() {
    if (0 == numEntries) {
        return (0);
    }

    ListItem* ret = entries[headPointer];

    entries[headPointer] = 0;

    if (headPointer == (maxEntries - 1)) {
        headPointer = 0;
    } else {
        headPointer++;
    }
    numEntries--;
    return (ret);
}

/*****************************************************************************
 * Method: ArrayList::removeTail()
 *
 * @description
 *
 *******************************************************************************/
ListItem* ArrayList::removeTail() {
    if (0 == numEntries) {
        return (0);
    }

    int p = MODULO((headPointer + numEntries - 1), (maxEntries));
    ListItem* ret = entries[p];
    entries[p] = 0;
    numEntries--;
    return (ret);
}

/*****************************************************************************
 * Method: ArrayList::removeItemAt(int at)
 *
 * @description
 *
 *******************************************************************************/
ListItem* ArrayList::removeItemAt(int at) {
    if (at < 0 || at >= numEntries) {
        return (0);
    }

    int ptr = MODULO((headPointer + at), maxEntries);

    ListItem* ret = entries[ptr];
    entries[ptr] = 0;
    for (int i = 0; i < ((numEntries - 1) - at); i++) {
        int p = MODULO((headPointer + at + i), maxEntries);
        entries[p] = entries[MODULO((p + 1), maxEntries)];
    }

    numEntries--;
    return (ret);
}

/*****************************************************************************
 * Method: ArrayList::insertItemAt(int at, ListItem* item)
 *
 * @description
 *
 *******************************************************************************/
ErrorT ArrayList::insertItemAt(int at, ListItem* item) {
    if (numEntries + 1 > maxEntries) {
        return (cDatabaseOverflow );
    }

    if (at < 0 || at > numEntries) {
        return (cIndexOutOfBounds );
    }

    for (int i = (numEntries - at); i >= 0; i--) {
        int p = MODULO((headPointer + at + i), maxEntries);
        entries[MODULO((p + 1), maxEntries)] = entries[p];
    }

    entries[MODULO((headPointer + at), maxEntries)] = item;

    numEntries++;
    return (cOk );
}

/*****************************************************************************
 * Method: ArrayList::getItemAt(int i)
 *
 * @description
 *
 *******************************************************************************/
ListItem* ArrayList::getItemAt(int i) {
    return (entries[MODULO((headPointer + i), maxEntries)]);
}

/*****************************************************************************
 * Method: ArrayList::removeItem(ListItem* item)
 *
 * @description
 *
 *******************************************************************************/
ListItem* ArrayList::removeItem(ListItem* item) {
    for (int i = 0; i < numEntries; i++) {
        if (item == entries[MODULO((headPointer + i), maxEntries)]) {
            return (removeItemAt(i));
        }
    }
    return (0);
}

/*****************************************************************************
 * Method: ArrayList::print()
 *
 * @description
 *
 *******************************************************************************/
void ArrayList::print() {
    for (int i = 0; i < maxEntries; i++) {
        if (headPointer == i)
            printf("H ");
        printf("|%8x", reinterpret_cast<unint4>(entries[i]));
    }
    printf("|"LINEFEED);
}
