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

#include "ArrayDatabase.hh"


// if we are sure a -b is never > b we can use this!
#define MODULO(a,b)  a >= b ? a-b : a

ArrayDatabase::ArrayDatabase( unint1 max ) {
    maxEntries = max;
    numEntries = 0;
    entries = new DatabaseItem*[ max ];

    for ( int i = 0; i < maxEntries; i++ ) {
        entries[ i ] = 0;
    }

    headPointer = 0;

}

ArrayDatabase::~ArrayDatabase() {
    delete[] entries;
}

unint1 ArrayDatabase::size() {
    return (numEntries);
}

DatabaseItem* ArrayDatabase::getHead() {
    return (entries[ headPointer ]);
}

DatabaseItem* ArrayDatabase::getTail() {
    return (entries[ MODULO(( headPointer + numEntries - 1 ),maxEntries) ]);
}

ErrorT ArrayDatabase::addTail( DatabaseItem* item ) {

    if ( numEntries + 1 > maxEntries ) {
        return (cDatabaseOverflow);
    }

    int ptr = MODULO(( headPointer + numEntries ) , maxEntries);
    numEntries++;
    entries[ ptr ] = item;

    return (cOk);
}

ErrorT ArrayDatabase::addHead( DatabaseItem* item ) {

    if ( numEntries + 1 >= maxEntries ) {
        return (cDatabaseOverflow);
    }

    if ( headPointer == 0 ) {
        //addHead only grows to the left side,
        //so check whether it goes over 0
        headPointer = (unint1) (maxEntries - 1);
    }
    else {
        headPointer--;
    }

    entries[ headPointer ] = item;
    numEntries++;

    return (cOk);

}

DatabaseItem* ArrayDatabase::removeHead() {
    if ( 0 == numEntries ) {
        return (0);
    }

    DatabaseItem* ret = entries[ headPointer ];

    entries[ headPointer ] = 0;

    if ( headPointer == ( maxEntries - 1 ) ) {
        headPointer = 0;
    }
    else {
        headPointer++;
    }
    numEntries--;
    return (ret);
}

DatabaseItem* ArrayDatabase::removeTail() {
    if ( 0 == numEntries ) {
        return (0);
    }

    int p = MODULO(( headPointer + numEntries - 1 ) , ( maxEntries ));
    DatabaseItem* ret = entries[ p ];
    entries[ p ] = 0;
    numEntries--;
    return (ret);
}

DatabaseItem* ArrayDatabase::removeItemAt( int at ) {

    if ( at < 0 || at > numEntries ) {
        return (0);
    }

    int ptr = MODULO(( headPointer + at ) , maxEntries);

    DatabaseItem* ret = entries[ ptr ];
    entries[ ptr ] = 0;
    for ( int i = 0; i < ( ( numEntries - 1 ) - at ); i++ ) {
        int p = MODULO(( headPointer + at + i ) , maxEntries);
        entries[ p ] = entries[ MODULO(( p + 1 ) , maxEntries) ];
    }

    numEntries--;
    return (ret);
}


ErrorT ArrayDatabase::insertItemAt( int at, DatabaseItem* item ) {

    if ( numEntries + 1 > maxEntries ) {
        return (cDatabaseOverflow);
    }

    if ( at < 0 || at > numEntries ) {
        return (cIndexOutOfBounds);
    }

    for ( int i = ( numEntries - at ); i >= 0; i-- ) {
        int p = MODULO(( headPointer + at + i ) , maxEntries);
        entries[ MODULO(( p + 1 ) , maxEntries) ] = entries[ p ];
    }

    entries[ MODULO(( headPointer + at ) , maxEntries) ] = item;

    numEntries++;
    return (cOk);

}


DatabaseItem* ArrayDatabase::getItemAt( int i ) {
    return (entries[ MODULO(( headPointer + i ) , maxEntries) ]);
}

DatabaseItem* ArrayDatabase::removeItem( DatabaseItem* item ) {
    for ( int i = 0; i <= numEntries; i++ ) {
        if ( item == entries[ MODULO(( headPointer + i ) , maxEntries) ] ) {
            return (removeItemAt( i ));
        }
    }
    return (0);
}
