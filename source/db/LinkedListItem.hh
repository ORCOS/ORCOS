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

#include "db/LinkedList.hh"
#include "db/ListItem.hh"

/*!
 * \ingroup databases
 * \brief This class implements a linked list.
 *
 * This class is used to identify classes that are LinkedListDatabaseItems
 * and thus may be stored in a LinkedListDataBase. Every LinkedListDatabaseItem holds
 * a reference to some DatabaseItem which may e.g be a thread or some other kind of object.
 */
class LinkedListItem /*: public ListItem*/ {

    friend class LinkedList;

private:

    /*!
     * \brief Some data (structure) to be stored in this linked list database.
     * may for example be an object like a thread.
     */
    ListItem*       data;

    /*!
     * \brief the predecessor
     */
    LinkedListItem* pred;

    /*!
     * \brief the successor
     */
    LinkedListItem* succ;

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
    LinkedList* host_db;

public:

    LinkedListItem() {
        this->data = 0;
        this->pred = 0;
        this->succ = 0;
        this->host_db = 0;
    }


    LinkedListItem(ListItem* p_data) {
        this->data = p_data;
        this->pred = 0;
        this->succ = 0;
        this->host_db = 0;
    }

    ~LinkedListItem() {
        if (host_db != 0)
            host_db->remove(this);
    }


    /*!
     * \brief Returns the reference to the DatabaseItem this LLDbItem is holding.
     */
    inline ListItem* getData() const {
        return (data);
    }

    /*!
     * \brief Sets the reference to the DatabaseItem this LLDbItem is holding.
     *
     * This may be used in order to reuse a LLDbItem if memory cant be freed.
     */
    inline void setData(ListItem* p_data) {
        this->data = p_data;
    }

    /*!
     * \brief Get the predecessor of this LLDbItem
     */
    inline LinkedListItem* getPred() const {
        return (pred);
    }

    /*!
     * \brief Get the successor of this LLDbItem
     */
    inline LinkedListItem* getSucc() const {
        return (succ);
    }

    /*!
     * \brief Set the predecessor of this LLDbItem
     */
    inline void setPred(LinkedListItem* item) {
        pred = item;
    }

    /*!
     * \brief Set the successor of this LLDbItem
     */
    inline void setSucc(LinkedListItem* item) {
        succ = item;
    }

    /*!
     * \brief Removes this LinkedListDatabaseItem from its current list and updates the host_db
     */
    void remove() {
        if (host_db != 0)
            host_db->remove(this);
    }


};

#endif /*LINKEDLISTDATABASEITEM_HH_*/
