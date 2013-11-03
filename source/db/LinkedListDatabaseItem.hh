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

#ifndef LINKEDLISTDATABASEITEM_HH_
#define LINKEDLISTDATABASEITEM_HH_

#include "db/LinkedListDatabase.hh"
#include "db/DatabaseItem.hh"


/*!
 * \ingroup databases
 * \brief This class implements a linked list.
 *
 * This class is used to identify classes that are LinkedListDatabaseItems
 * and thus may be stored in a LinkedListDataBase. Every LinkedListDatabaseItem holds
 * a reference to some DatabaseItem which may e.g be a thread or some other kind of object.
 */
class LinkedListDatabaseItem: public DatabaseItem {

    friend class LinkedListDatabase;

private:

    /*!
     * \brief Some data (structure) to be stored in this linked list database.
     * may for example be an object like a thread.
     */
    DatabaseItem* data;

    /*!
     * \brief the predecessor
     */
    LinkedListDatabaseItem* pred;

    /*!
     * \brief the successor
     */
    LinkedListDatabaseItem* succ;

    /*!
     * \brief Reference to the LinkedListDatabase this item is stored in
     *
     * This Reference is needed for the remove() method which needs
     * to update the host database to maintain a correct database.
     *
     * For security reasons each LinkedListDatabaseItem can only
     * be stored in one LinkedListDatabase at the same moment. Thus if
     * invoking addTail on a LinkedListDatabaseItem stored already in another
     * Database will automatically remove that item from that database.
     */
    LinkedListDatabase* host_db;

public:

    LinkedListDatabaseItem() {
        this->data = data;
        this->pred = 0;
        this->succ = 0;
        this->host_db = 0;
    }
    ;

    LinkedListDatabaseItem( DatabaseItem* p_data ) {
        this->data = p_data;
        this->pred = 0;
        this->succ = 0;
        this->host_db = 0;
    }

    ~LinkedListDatabaseItem() {
    }
    ;

    /*!
     * \brief Returns the reference to the DatabaseItem this LLDbItem is holding.
     */
    inline DatabaseItem* getData() {
        return (data);
    }

    /*!
     * \brief Sets the reference to the DatabaseItem this LLDbItem is holding.
     *
     * This may be used in order to reuse a LLDbItem if memory cant be freed.
     */
    inline void setData( DatabaseItem* p_data ) {
        this->data = p_data;
    }

    /*!
     * \brief Get the predecessor of this LLDbItem
     */
    inline LinkedListDatabaseItem* getPred() {
        return (pred);
    }

    /*!
     * \brief Get the successor of this LLDbItem
     */
    inline LinkedListDatabaseItem* getSucc() {
        return (succ);
    }

    /*!
     * \brief Set the predecessor of this LLDbItem
     */
    inline void setPred( LinkedListDatabaseItem* item ) {
        pred = item;
    }

    /*!
     * \brief Set the successor of this LLDbItem
     */
    inline void setSucc( LinkedListDatabaseItem* item ) {
        succ = item;
    }

    /*!
     * \brief Removes this LinkedListDatabaseItem from its current list and updates the host_db
     */
    void remove() {
            // if we are not stored anywhere just return
            if ( host_db == 0 )
                return;

            if ( host_db->getHead() == this ) {
                // i'm the head of the db. update the head
                host_db->removeHead();
            }
            else if ( host_db->getTail() == this ) {
                // i'm the tail of the db
                host_db->removeTail();
            }
            else {
                // i'm neither the head nor the tail so we can just manipulate the pointers
                host_db->size--;
                host_db = 0;

                // update both
                pred->setSucc( succ );
                succ->setPred( pred );

                // finally set my succ and pred to null
                pred = 0;
                succ = 0;
            }
        };


};




#endif /*LINKEDLISTDATABASEITEM_HH_*/
