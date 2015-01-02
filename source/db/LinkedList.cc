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

#include "LinkedList.hh"
#include "LinkedListItem.hh"
#include "assemblerFunctions.hh"

LinkedList::LinkedList() {
    headItem = 0;
    tailItem = 0;
    size = 0;
}

/*****************************************************************************
 * Method: LinkedList::removeHead()
 *
 * @description
 *
 *******************************************************************************/
LinkedListItem* LinkedList::removeHead() {
    DISABLE_IRQS(status);
    LinkedListItem* ret = this->headItem;

    if (headItem != 0) {
        headItem = headItem->succ;

        if (headItem != 0) {
            headItem->pred = 0;
        } else {
            tailItem = 0;
        }

        ret->pred    = 0;
        ret->succ    = 0;
        ret->host_db = 0;
        size--;
    }

    RESTORE_IRQS(status);
    return (ret);
}


/*****************************************************************************
 * Method: LinkedList::removeTail()
 *
 * @description
 *
 *******************************************************************************/
LinkedListItem* LinkedList::removeTail() {
    DISABLE_IRQS(status);
    LinkedListItem* ret = this->tailItem;
    if (tailItem != 0) {
        tailItem = tailItem->pred;
        if (tailItem != 0) {
            tailItem->succ = 0;
        } else {
            headItem = 0;
        }

        ret->pred = 0;
        ret->succ = 0;
        ret->host_db = 0;
        size--;
    }
    RESTORE_IRQS(status);
    return (ret);
}

/*LinkedListDatabaseItem* LinkedListDatabase::removeAt( int position ) {
 // Error, impossible position specified, return NULL.
 if ( position >= size )
 return NULL;

 // if the specified item is the headItem or the tailItem call the appropriate methods
 if ( position == 0 )
 return this->removeHead();
 if ( position == size - 1 )
 return this->removeTail();

 // find the specified item and store it in ret
 LinkedListDatabaseItem* ret = this->headItem;
 for ( int i = 0; i < position; i++ )
 ret = ret->getSucc();

 LinkedListDatabaseItem* pred = ret->getPred();
 LinkedListDatabaseItem* suc = ret->getSucc();

 pred->setSucc( suc );
 suc->setPred( pred );

 ret->setPred( 0 );
 ret->setSucc( 0 );
 ret->host_db = 0;

 size--;
 return ret;
 }*/

/*****************************************************************************
 * Method: LinkedList::addTail(LinkedListItem* llitem)
 *
 * @description
 *
 *******************************************************************************/
ErrorT LinkedList::addTail(LinkedListItem* llitem) {
    ASSERT(llitem);
    /* if this item is already in a database remove it there */
    DISABLE_IRQS(status);
    if (llitem->host_db != 0)
        llitem->remove();

    llitem->host_db = this;

    if (headItem == 0) {
        // no item in the list
        headItem = llitem;
        tailItem = llitem;
        llitem->setPred(0);
        llitem->setSucc(0);
    } else {
        // got at least one item in the list
        // add the item to the end of the list
        llitem->setPred(tailItem);
        llitem->setSucc(0);
        tailItem->setSucc(llitem);
        tailItem = llitem;
    }
    size++;
    RESTORE_IRQS(status);
    return (cOk );
}

/*****************************************************************************
 * Method: LinkedList::addHead(LinkedListItem* llitem)
 *
 * @description
 *
 *******************************************************************************/
ErrorT LinkedList::addHead(LinkedListItem* llitem) {
    ASSERT(llitem);
    /* if this item is already in a database, remove it there */
    DISABLE_IRQS(status);
    if (llitem->host_db != 0)
        llitem->remove();

    llitem->host_db = this;

    if (headItem == 0) {
        // no item in the list
        headItem = llitem;
        tailItem = llitem;
        llitem->setPred(0);
        llitem->setSucc(0);
    } else {
        // got at least one item in the list
        // add the item to the head of the list
        if (headItem == tailItem)
            tailItem = headItem;

        llitem->setPred(0);
        llitem->setSucc(headItem);
        headItem->setPred(llitem);
        headItem = llitem;
    }
    size++;
    RESTORE_IRQS(status);
    return (cOk );
}

/*****************************************************************************
 * Method: LinkedList::remove(ListItem* item)
 *
 * @description
 *
 *******************************************************************************/
ErrorT LinkedList::remove(ListItem* item) {
    ASSERT(item);

    DISABLE_IRQS(status);
    LinkedListItem* llitem = getItem(item);
    if (llitem != 0) {
        ErrorT ret = remove(llitem);
        RESTORE_IRQS(status);
        delete llitem;
        return (ret);
    }

    RESTORE_IRQS(status);
    return (cError );
}


/*****************************************************************************
 * Method: LinkedList::remove(ListItem* item)
 *
 * @description
 *
 *******************************************************************************/
ErrorT LinkedList::remove(LinkedListItem* litem) {
    if (litem->host_db != this)
        return (cElementNotInDatabase);

    DISABLE_IRQS(status);

   /* set pointers */
   LinkedListItem *succ = litem->getSucc();
   LinkedListItem *pred = litem->getPred();

   if (litem->pred == 0)
       headItem = succ;

   if (tailItem == litem)
       tailItem = pred;

   if (pred != 0)
       pred->succ = succ;
   if (succ != 0)
       succ->pred = pred;

   litem->host_db = 0;
   litem->pred = 0;
   litem->succ = 0;

   size--;
   RESTORE_IRQS(status);
   return (cOk);
}


/*****************************************************************************
 * Method: LinkedList::addTail(ListItem* item)
 *
 * @description
 *
 *******************************************************************************/
ErrorT LinkedList::addTail(ListItem* item) {
    ASSERT(item);
    // Create a new LinkedListDatabaseItem which stores a reference to this DatabaseItem
    LinkedListItem* llitem = new LinkedListItem(item);

    return (addTail(llitem));
}

/*****************************************************************************
 * Method: LinkedList::addHead(ListItem* item)
 *
 * @description
 *
 *******************************************************************************/
ErrorT LinkedList::addHead(ListItem* item) {
    ASSERT(item);
    // Create a new LinkedListDatabaseItem which stores a reference to this DatabaseItem
    LinkedListItem* llitem = new LinkedListItem(item);

    return (addHead(llitem));
}

/*****************************************************************************
 * Method: LinkedList::insertAfter(LinkedListItem* llItem, LinkedListItem* existingItem)
 *
 * @description
 *
 *******************************************************************************/
ErrorT LinkedList::insertAfter(LinkedListItem* llItem, LinkedListItem* existingItem) {
    ASSERT(llItem);
    ASSERT(existingItem);
    ASSERT((existingItem != llItem));

    DISABLE_IRQS(status);
    if (llItem->host_db != 0)
        llItem->remove();

    if (existingItem->succ != 0) {
        existingItem->succ->setPred(llItem);
        llItem->setSucc(existingItem->succ);
    } else {
        // we must be the new tailitem
        tailItem = llItem;
        llItem->setSucc(0);
    }

    existingItem->setSucc(llItem);
    llItem->setPred(existingItem);

    // set this database to its owner
    llItem->host_db = this;
    size++;
    RESTORE_IRQS(status);

    return (cOk );
}

/*ErrorT LinkedListDatabase::insertBefore( LinkedListDatabaseItem* llItem, LinkedListDatabaseItem* existingItem ) {

 if ( existingItem == 0 ) {
 return cError;
 }

 if ( existingItem->getPred() != 0 ) {
 existingItem->getPred()->setSucc( llItem );
 llItem->setPred( existingItem->getPred() );
 }
 existingItem->setPred( llItem );
 llItem->setSucc( existingItem );

 // set this database to its owner
 llItem->host_db = this;
 size++;
 return cOk;

 }
 ;*/

LinkedListItem*
LinkedList::getItem(ListItem* data) {
    DISABLE_IRQS(status);
    LinkedListItem* curItem = headItem;
    while (curItem != 0 && curItem->getData() != data)
        curItem = curItem->getSucc();

    RESTORE_IRQS(status);
    return (curItem);
}

