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

#include "Directory.hh"
#include "stringtools.hh"
#include "inc/defines.h"
#include "memtools.hh"
#include "filesystem/File.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

Directory::Directory(const char* p_name) :
         CharacterDevice(cDirectory, false, p_name) {

    num_entries = 0;
}

Directory::~Directory() {

    LinkedListItem* litem = dir_content.getHead();
    Resource* res;

    while (litem != 0)
    {
        // not found yet and we got anther entry in our database
        res = (Resource*) litem->getData();

        // we need to move to the successor before deleting the linked list container
        litem = litem->getSucc();
        dir_content.remove(res);

        delete res;
    }

}

ErrorT Directory::add(Resource* res) {
    dir_content.addTail(res);
    num_entries++;
    return (cOk );
}

ErrorT Directory::remove(Resource *res) {
    // For internal files we must first check the type of resource
    if (res->getType() & (cNonRemovableResource))
        return (cResourceNotRemovable );

    if (dir_content.remove(res) == cOk)
    {
        num_entries--;
        return (cOk );
    } else {
        LOG(FILESYSTEM,ERROR,"Directory::remove() Illegal resource '%s' (%x)",res->getName(),res);
    }

    return (cError );
}

Resource* Directory::get(const char* p_name, unint1 name_len) {
    // parse the whole dir_content db for an item with name 'name'
    LinkedListItem* litem = dir_content.getHead();
    Resource* res;

    if (!p_name)
        return (0);

     while (litem != 0)
    {
        // not found yet and we got anther entry in our database
        res = (Resource*) litem->getData();
        int res_namelen = strlen(res->getName());
        if (res_namelen == name_len) {
            // compare names. the res->GetName() as second parameter as that string is limited
            if (strcmp2(p_name, res->getName(), res_namelen) == 0)
                return (res);
        }

        litem = litem->getSucc();
    }

    return (0);
}

/*
 * Reads the contents description into the bytes array for up to length bytes
 *
 * Layout:
 *  Directory Type ID (1 byte) | dir name length (1 Byte)
 * | dir name (<256 bytes, zero terminated)| type (1 byte) | id (1byte)
 * | filesize  (4byte)| flags (4byte)
 *
 */
ErrorT Directory::readBytes(char *bytes, unint4 &length) {

    if (length < 1)
        return (cOk );

    unint4 pos          = 0;
    unint4 entry_num    = 0;

    LinkedListItem* litem = this->dir_content.getHead();

    /* resume reading after last read directory entry*/
    while (litem != 0 && entry_num < position) {
        litem = litem->getSucc();
        entry_num++;
    }

    /* keep reading from current directory entry until end
     * or requested length is maximally used */
    while (litem != 0)
    {
        Resource* ritem     = (Resource*) litem->getData();
        const char* p_name  = ritem->getName();
        /* max 256 chars for the name */
        unint1 namelen = (unint1) strlen(p_name);
        /* align next entry to multiple of 4 bytes */
        unint2 namelen2 = (namelen + 3) & ~(3);

        Directory_Entry_t* entry = (Directory_Entry_t*) bytes;
        if (pos + sizeof(Directory_Entry_t) + namelen2 > length)
            break;

        bytes   += sizeof(Directory_Entry_t) + namelen2;
        pos     += sizeof(Directory_Entry_t) + namelen2;

        entry->namelen  = namelen2;
        entry->datetime = 0;
        entry->filesize = 0;
        entry->resId    = ritem->getId();
        entry->resType  = ritem->getType();
        entry->flags    = 0;

        if (ritem->getType() == cFile)
        {
            entry->filesize = ((File*) ritem)->getFileSize();
            entry->flags    = ((File*) ritem)->getFlags();
        }

        /* copy name into the bytes array */
        memcpy((void*) entry->name, (const void*) p_name, namelen+1);

        litem = litem->getSucc();
        this->position++;
    }

    length = pos;
    return (cOk );
}
