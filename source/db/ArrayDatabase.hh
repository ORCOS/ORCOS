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

#ifndef ARRAYDATABASE_HH_
#define ARRAYDATABASE_HH_

#include "DatabaseItem.hh"
#include "inc/types.hh"
#include "inc/const.hh"

/*!
 * \brief Database using an array to store the entries
 * \ingroup databases
 *
 * The database is organized as a 'ringbuffer'
 *
 * (H)eadPointer
 *
 *
 * Head						Tail
 *
 * Case 1: numentries = 5
 * [1][2][3][4][5][0][0][0][0][0]
 *  H
 *
 * Case 2: numentries = 3
 * [0][0][1][2][3][0][0][0][0][0]
 *        H
 *
 * Case 3: numentries = 6
 * [3][4][5][6][0][0][0][0][1][2]
 *  		 	    		H
 *
 *
 * By using this structure databases with known maximum sizes
 * could be realized with O(1) for get/set Head/Tail Operations.
 *
 */

class ArrayDatabase {

public:

    //! The array containing the DatabaseIems. public for speedup
    DatabaseItem** entries;

private:
    //! The maximum size of this arrayDB
    unint1 maxEntries;

    //! current number of entries in db
    unint1 numEntries;

    //! current position of the headPointer
    unint1 headPointer;

public:
    ArrayDatabase( int maxEntries );
    ~ArrayDatabase();

    /*!
     * \brief returns the number of elements in this database
     */
    int size();

    /*!
     * \brief Adds another element to the database at the end
     */
    ErrorT addTail( DatabaseItem* item );

    /*!
     * \brief Adds another element at the beginning of the database
     *
     * This method performs bad since it may have to move all items
     * backwards in the database
     */
    ErrorT addHead( DatabaseItem* item );

    /*!
     * \brief Returns the head-element of this db or 0 if empty.
     */
    DatabaseItem* getHead();

    /*!
     * \brief Returns the tail-element of this db or 0 if empty.
     */
    DatabaseItem* getTail();

    /*!
     * \brief Removes the head-element of the db and returns it.
     */
    DatabaseItem* removeHead();

    /*!
     * \brief Removes the tail-element of the db and returns it.
     */
    DatabaseItem* removeTail();

    /*!
     * \brief Inserts a new element at the specified position.
     *
     * Inserts a new element at the specified position.
     * The position must exist (0<=position<=numEntries).
     * With insertion all elements (including position) are moved to the right.
     */
    ErrorT insertItemAt( int at, DatabaseItem* item );

    /*!
     * \brief Returns the element at the specified position.
     */
    DatabaseItem* getItemAt( int i );

    /*!
     * \brief Removes the element at the specified position.
     *
     * Removes the element at the specified position.
     * The position must exist (0<=position<=numEntries).
     * The item at position is removed and all elements
     * right of posiotion are moved left.
     */
    DatabaseItem* removeItemAt( int );

    /*!
     * \brief Removes the specified element.
     *
     * Removes the specified element. If the element is included
     * in the db it is removed with the removeItemAt() function.
     */
    DatabaseItem* removeItem( DatabaseItem* item );
};

#endif /*ARRAYDATABASE_HH_*/
