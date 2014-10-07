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

class LinkedListItem;
class ListItem;

/*!
 *  \ingroup databases
 *  \brief This class implements a linked list database.
 *
 */
class LinkedList {
    friend class LinkedListItem;
    friend class Task;  /* for export of size */
private:

    LinkedListItem* headItem;
    LinkedListItem* tailItem;

    /*!
     * Stores the number of elements in this database;
     */
    unint1 size;

public:

    LinkedList();

    ~LinkedList() {
    }


    /*!
     * Adds another LinkedListDatabaseItem to this linked list at its tail
     */
    ErrorT addTail(LinkedListItem* item);

    /*!
     * Adds another LinkedListDatabaseItem to this linked list at its tail
     */
    ErrorT addHead(LinkedListItem* item);

    /*!
     * Adds another DatabaseItem to this linked list at its tail
     */
    ErrorT addTail(ListItem* item);

    /*!
     * Adds another DatabaseItem to this linked list at its tail
     */
    ErrorT addHead(ListItem* item);

    /*!
     * Adds another DatabaseItem to this linked list after the specified item
     */
    ErrorT insertAfter(LinkedListItem* llItem, LinkedListItem* existingItem);

    /*!
     * Adds another DatabaseItem to this linked list before the specified item
     */
    //ErrorT insertBefore( LinkedListDatabaseItem* llItem, LinkedListDatabaseItem* existingItem );
    /*!
     * Adds another DatabaseItem to this linked list after the specified item
     */
    ErrorT insertAt(ListItem* newItem, int at) {
        return (cError );
    }

    /*!
     * Returns the first LLDbItem in this database.
     */
    inline LinkedListItem* getHead() {
        return (headItem);
    }

    /*!
     * Removes the associated linkedlistdatabase item of the item from this list.
     * The associated linkedlistdatebase item is deleted.
     */
    ErrorT remove(ListItem* item);

    /*!
     * Returns the first LLDbItem in this database and removes it from the database.
     */
    LinkedListItem* removeHead();

    /*!
     * Returns the specified LLDbItem in this database and removes it from the database.
     */
    // LinkedListDatabaseItem* removeAt( int position );
    /*!
     * Returns the last LLDbItem in this database and removes it from the database.
     */
    LinkedListItem* removeTail();

    /*!
     * Returns the last LLDbItem in this database.
     */
    inline LinkedListItem* getTail() {
        return (tailItem);
    }

    /*!
     * This method returns the LinkedListDatabaseItem holding the reference to the DatabaseItem given as parameter.
     * It is important to get the LinkedListDatabaseItem in order to add this LLDbItem to another linked list
     * manually! (e.g move thread from blocked list to ready list)
     */
    LinkedListItem* getItem(ListItem* data);

    /*!
     * Checks wether this list is empty
     */
    inline bool isEmpty() {
        if (headItem == 0)
        {
            return (1);
        }
        return (0);
    }

    inline unint getSize() {
        return (size);
    }

};

#endif /*LINKEDLISTDATABASE_HH_*/
