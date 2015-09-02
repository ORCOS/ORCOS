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
#include <assemblerFunctions.hh>

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

    int             m_lock;
    /*!
     * Stores the number of elements in this database;
     */
    unint1 size;

public:
    LinkedList();

    ~LinkedList() {
    }


    /*****************************************************************************
     * Method: addTail(LinkedListItem* item)
     *
     * @description
     *  Adds another LinkedListDatabaseItem to this linked list at its tail
     *******************************************************************************/
    ErrorT addTail(LinkedListItem* item);


    /*****************************************************************************
     * Method: addHead(LinkedListItem* item)
     *
     * @description
     *  Adds another LinkedListDatabaseItem to this linked list at its head
     *******************************************************************************/
    ErrorT addHead(LinkedListItem* item);


    /*****************************************************************************
     * Method: addTail(ListItem* item)
     *
     * @description
     *  Adds another DatabaseItem to this linked list at its tail thereby
     *  creating a new LinkedListItem
     *******************************************************************************/
    ErrorT addTail(ListItem* item);


    /*****************************************************************************
     * Method: addHead(ListItem* item)
     *
     * @description
     *  Adds another DatabaseItem to this linked list at its tail thereby
     *  creating a new LinkedListItem
     *******************************************************************************/
    ErrorT addHead(ListItem* item);

    /*****************************************************************************
     * Method: insertAfter(LinkedListItem* llItem, LinkedListItem* existingItem)
     *
     * @description
     *  Adds llItem to this linked list after existingItem
     *******************************************************************************/
    ErrorT insertAfter(LinkedListItem* llItem, LinkedListItem* existingItem);


    ErrorT insertBefore( LinkedListItem* llItem, LinkedListItem* existingItem );

    /*****************************************************************************
     * Method: insertAt(ListItem* newItem, int at)
     *
     * @description
     *  Adds another DatabaseItem to this linked list after the specified item.
     *  Currently not supported!
     *******************************************************************************/
#if 0
    ErrorT insertAt(ListItem* newItem, int at) {
        return (cError);
    }
#endif

    /*****************************************************************************
     * Method: getHead()
     *
     * @description
     *  Returns the first LinkedListItem in this Linked List
     *******************************************************************************/
    inline LinkedListItem* getHead() {
        return (headItem);
    }

    /*****************************************************************************
     * Method: remove(ListItem* item)
     *
     * @description
     *  Removes the associated linkedlistdatabase item of the item from this list.
     * The associated linkedlistdatebase item is deleted.
     *******************************************************************************/
    ErrorT remove(ListItem* item);


    ErrorT remove(LinkedListItem* litem);

    /*****************************************************************************
     * Method: removeHead()
     *
     * @description
     * Returns the first LLDbItem in this database and removes it from the database.
     *******************************************************************************/
    LinkedListItem* removeHead();


    /*****************************************************************************
     * Method: removeTail()
     *
     * @description
     * Returns the last LLDbItem in this database and removes it from the database.
     *******************************************************************************/
    LinkedListItem* removeTail();


    /*****************************************************************************
     * Method: getTail()
     *
     * @description
     * Returns the last Linked List Item in this Linked List.
     *******************************************************************************/
    inline LinkedListItem* getTail() {
        return (tailItem);
    }


    /*****************************************************************************
     * Method: getItem(ListItem* data)
     *
     * @description
     *  This method returns the LinkedListDatabaseItem holding the reference to the DatabaseItem given as parameter.
     * It is important to get the LinkedListDatabaseItem in order to add this LLDbItem to another linked list
     * manually! (e.g move thread from blocked list to ready list)
     *******************************************************************************/
    LinkedListItem* getItem(ListItem* data);


    /*****************************************************************************
     * Method: isEmpty()
     *
     * @description
     *  Checks whether this list is empty
     *******************************************************************************/
    inline bool isEmpty() {
        if (headItem == 0) {
            return (1);
        }
        return (0);
    }

    /*****************************************************************************
     * Method: getSize()
     *
     * @description
     *  Returns the number of elements in this list
     *******************************************************************************/
    inline unint getSize() {
        return (size);
    }

    /*****************************************************************************
      * Method: lock()
      *
      * @description
      *  Locks the list against concurrent modification.
      *  Must be called before access to it.
      *******************************************************************************/
    inline void lock() {
        SMP_SPINLOCK_GET(m_lock);
    }

    /*****************************************************************************
     * Method: unlock()
     *
     * @description
     *  Unlocks the list again.
     *******************************************************************************/
    inline void unlock() {
        SMP_SPINLOCK_FREE(m_lock);
    }



};

#endif /*LINKEDLISTDATABASE_HH_*/
