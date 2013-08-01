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

#ifndef LINKEDLISTDATABASE_HH_
#define LINKEDLISTDATABASE_HH_

#include "inc/types.hh"
#include "inc/const.hh"

class LinkedListDatabaseItem;
class DatabaseItem;

/*!
 *  \ingroup databases
 *  \brief This class implements a linked list database.
 *
 */
class LinkedListDatabase {
friend class LinkedListDatabaseItem;
private:

    LinkedListDatabaseItem* headItem;
    LinkedListDatabaseItem* tailItem;

    /*!
     * Stores the number of elements in this database;
     */
    unint1 size;

public:

    LinkedListDatabase();
    ~LinkedListDatabase() {
    }
    ;

    /*!
     * Adds another LinkedListDatabaseItem to this linked list at its tail
     */
    ErrorT addTail( LinkedListDatabaseItem* item );

    /*!
     * Adds another LinkedListDatabaseItem to this linked list at its tail
     */
    ErrorT addHead( LinkedListDatabaseItem* item );

    /*!
     * Adds another DatabaseItem to this linked list at its tail
     */
    ErrorT addTail( DatabaseItem* item );

    /*!
     * Adds another DatabaseItem to this linked list at its tail
     */
    ErrorT addHead( DatabaseItem* item );

    /*!
     * Adds another DatabaseItem to this linked list after the specified item
     */
    ErrorT insertAfter( LinkedListDatabaseItem* llItem, LinkedListDatabaseItem* existingItem );

    /*!
     * Adds another DatabaseItem to this linked list before the specified item
     */
    //ErrorT insertBefore( LinkedListDatabaseItem* llItem, LinkedListDatabaseItem* existingItem );

    /*!
     * Adds another DatabaseItem to this linked list after the specified item
     */
    ErrorT insertAt( DatabaseItem* newItem, int at ) {
        return cError;
    }
    ;

    /*!
     * Returns the first LLDbItem in this database.
     */
    inline LinkedListDatabaseItem* getHead() {
        return headItem;
    }

    /*!
     * Removes the associated linkedlistdatabase item of the item from this list.
     * The associated linkedlistdatebase item is deleted.
     */
    ErrorT remove( DatabaseItem* item );

    /*!
     * Returns the first LLDbItem in this database and removes it from the database.
     */
    LinkedListDatabaseItem* removeHead();

    /*!
     * Returns the specified LLDbItem in this database and removes it from the database.
     */
   // LinkedListDatabaseItem* removeAt( int position );

    /*!
     * Returns the last LLDbItem in this database and removes it from the database.
     */
    LinkedListDatabaseItem* removeTail();

    /*!
     * Returns the last LLDbItem in this database.
     */
    inline LinkedListDatabaseItem* getTail() {
        return tailItem;
    }

    /*!
     * This method returns the LinkedListDatabaseItem holding the reference to the DatabaseItem given as parameter.
     * It is important to get the LinkedListDatabaseItem in order to add this LLDbItem to another linked list
     * manually! (e.g move thread from blocked list to ready list)
     */
    LinkedListDatabaseItem* getItem( DatabaseItem* data );

    /*!
     * Checks wether this list is empty
     */
    inline bool isEmpty() {
        if ( headItem == 0 ) {
            return 1;
        }
        return 0;
    }

    inline unint getSize() {
        return size;
    }

};





#endif /*LINKEDLISTDATABASE_HH_*/
