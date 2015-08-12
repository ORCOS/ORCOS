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

Directory::Directory(const char* p_name, ResourceType type) :
        CharacterDevice((ResourceType) (type | cDirectory), false, p_name) {
    num_entries = 0;
}

Directory::~Directory() {
    LinkedListItem* litem = dir_content.getHead();

    while (litem != 0) {
        Resource* res = static_cast<Resource*>(litem->getData());

        /* we need to move to the successor before deleting the linked list container */
        litem = litem->getSucc();
        dir_content.remove(res);

        delete res;
    }
}

/*****************************************************************************
 * Method: Directory::invalidate()
 *
 * @description
 *  Marks this Directory and all of its sub directories as invalid.
 *******************************************************************************/
void Directory::invalidate() {
    LinkedListItem* litem = dir_content.getHead();
    while (litem != 0) {
        Resource* res = static_cast<Resource*>(litem->getData());
        /* invalidate resource */
        res->invalidate();
        litem = litem->getSucc();
    }
}

/*****************************************************************************
 * Method: Directory::add(Resource* res)
 *
 * @description
 *
 *******************************************************************************/
ErrorT Directory::add(Resource* res) {
    dir_content.addTail(res);
    num_entries++;
    return (cOk );
}

/*****************************************************************************
 * Method: Directory::remove(Resource *res)
 *
 * @description
 *
 *******************************************************************************/
ErrorT Directory::remove(Resource *res) {
    /* For internal files we must first check the type of resource */
    if (res->getType() & (cNonRemovableResource))
        return (cResourceNotRemovable);

    if (dir_content.remove(res) == cOk) {
        num_entries--;
        return (cOk );
    } else {
        LOG(FILESYSTEM, ERROR, "Directory::remove() Illegal resource '%s' (%x)", res->getName(), res);
    }

    return (cError );
}

/*****************************************************************************
 * Method: Directory::get(const char* p_name, unint1 name_len)
 *
 * @description
 *  gets the resource with name 'name'.
 *******************************************************************************/
Resource* Directory::get(const char* p_name, unint1 name_len) {
    /* parse the whole dir_content db for an item with name 'name' */
    LinkedListItem* litem = dir_content.getHead();

    if (!p_name)
        return (0);

    while (litem != 0) {
        /* not found yet and we got another entry in our database */
        Resource* res = static_cast<Resource*>(litem->getData());
        int res_namelen = strlen(res->getName());
        if (res_namelen == name_len) {
            /* compare names. the res->GetName() as second parameter as that string is limited */
            if (strncmp(p_name, res->getName(), res_namelen) == 0)
                return (res);
        }

        litem = litem->getSucc();
    }

    return (0);
}

/*****************************************************************************
 * Method: Directory::createDirectory(char* p_name, unint4 flags)
 *
 * @description
 *   Creates a new directory inside this directory.
 *   Virtual to allow specializations of directory to create the appropriate
 *   Directory objects and update the filesystem if any.
 *******************************************************************************/
Directory*  Directory::createDirectory(char* p_name, unint4 flags) {
    Directory* dir;
    if (p_name == 0)
        return (0);

    char* name = new char[strlen(p_name)+1];
    strcpy(name, p_name);

    dir = new Directory(name);
    ErrorT ret = this->add(dir);
    if (isError(ret)) {
        LOG(FILESYSTEM, ERROR, "Directory::createDirectory(): Error creating directory %d", ret);
        return (0);
    }
    return (dir);
}

/*****************************************************************************
 * Method: Directory::readBytes(char *bytes, unint4 &length)
 *
 * @description
 *  Reads the contents description into the bytes array for up to length bytes
 *  using the Directory_Entry_t structures.
 *  USed from user space to list the directory contents.
 *******************************************************************************/
ErrorT Directory::readBytes(char *bytes, unint4 &length) {
    if (length < 1)
        return (cOk);

    unint4 pos = 0;
    unint4 entry_num = 0;

    LinkedListItem* litem = this->dir_content.getHead();

    /* resume reading after last read directory entry*/
    while (litem != 0 && entry_num < position) {
        litem = litem->getSucc();
        entry_num++;
    }

    /* keep reading from current directory entry until end
     * or requested length is maximally used */
    while (litem != 0) {
        Resource* ritem = static_cast<Resource*>(litem->getData());
        const char* p_name = ritem->getName();
        /* max 256 chars for the name */
        unint1 namelen = (unint1) (strlen(p_name)+1);
        /* align next entry to multiple of 4 bytes */
        unint2 namelen2 = (namelen + 3) & ~(3);

        Directory_Entry_t* entry = reinterpret_cast<Directory_Entry_t*>(bytes);
        if (pos + sizeof(Directory_Entry_t) + namelen2 > length)
            break;

        bytes   += sizeof(Directory_Entry_t) + namelen2;
        pos     += sizeof(Directory_Entry_t) + namelen2;

        entry->namelen      = namelen2;
        entry->datetime     = 0;
        entry->filesize     = 0;
        entry->resId        = ritem->getId();
        entry->resType      = ritem->getType();
        entry->flags        = 0;

        if (ritem->getType() & cFile) {
            File* file = static_cast<File*>(ritem);
            entry->filesize     = file->getFileSize();
            entry->flags        = file->getFlags();
            entry->datetime     = file->getDateTime();
        }

        /* copy name into the bytes array */
        strncpy(entry->name, p_name, namelen);

        litem = litem->getSucc();
        this->position++;
    }

    length = pos;
    return (cOk);
}

/*****************************************************************************
 * Method: Directory::rename(char* newName)
 *
 * @description
 *  Renames the directory.
 *******************************************************************************/
ErrorT Directory::rename(char* newName) {
    if (newName != 0) {
        /* try freeing the old name */
        delete this->name;
        this->name = newName;
        return (cOk);
    }
    return (cNullPointerProvided);
}

OverlayDirectory::OverlayDirectory(const char* name, Directory* baseDir, Directory* overlay) :
        Directory(name, cOverlay) {
    this->baseDir       = baseDir;
    this->overlayDir    = overlay;
    if (overlayDir == 0) {
        LOG(FILESYSTEM, ERROR, "OverlayDirectory created with null pointer provided for overlay!!");
    }
}

/*****************************************************************************
 * Method: OverlayDirectory::get(const char* name, unint1 name_len)
 *
 * @description
 *
 *******************************************************************************/
Resource* OverlayDirectory::get(const char* name, unint1 name_len) {
    Resource* ret = overlayDir->get(name, name_len);
    if (ret)
        return (ret);

    /* overlay does not contain name.. try in base */
    if (baseDir == 0)
        return (0);

    return (baseDir->get(name, name_len));
}

/*****************************************************************************
 * Method: OverlayDirectory::readBytes(char *bytes, unint4 &length)
 *
 * @description
 *
 *******************************************************************************/
ErrorT OverlayDirectory::readBytes(char *bytes, unint4 &length) {
    /* TODO: properly implement this here... right now duplicate resources names
     * may be returned.. we must filter out the resources inside the baseDir that
     * are actually overlayed by entries inside the overlay directory */
    if (overlayDir->getPosition() < overlayDir->getNumEntries()) {
        return (overlayDir->readBytes(bytes, length));
    } else {
        if (baseDir != 0) {
            return (baseDir->readBytes(bytes, length));
        }
    }

    length = 0;
    return (cError );
}

/*****************************************************************************
 * Method: OverlayDirectory::getNumEntries()
 *
 * @description
 *
 *******************************************************************************/
unint2 OverlayDirectory::getNumEntries() {
    /*TODO: same as in readbytes! filter out duplicates!  */
    unint2 entries = overlayDir->getNumEntries();
    if (baseDir != 0)
        entries += baseDir->getNumEntries();
    return (entries);
}

/*****************************************************************************
 * Method: OverlayDirectory::seek(int4 seek_value)
 *
 * @description
 *
 *******************************************************************************/
ErrorT OverlayDirectory::seek(int4 seek_value) {
    return (cNotImplemented);
}

/*****************************************************************************
 * Method: OverlayDirectory::getContent()
 *
 * @description
 *
 *******************************************************************************/
LinkedList* OverlayDirectory::getContent() {
    LOG(FILESYSTEM, ERROR, "OverlayDirectory::getContent not implemented!");
    return (overlayDir->getContent());
}
