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

#ifndef DIRECTORY_H_
#define DIRECTORY_H_

#include "db/LinkedList.hh"
#include "filesystem/Resource.hh"
#include "hal/CharacterDevice.hh"
#include "filesystem/File.hh"

/*!
 * \brief Directory in a filesystem.
 *
 * \ingroup filesystem
 *
 * The directory is a node of a tree and can thus have multiple directories
 * or leaf nodes (resources other than a directory) as child.
 * A directory is also a characterdevice as it can be read using
 * fread() to get the directory contents in user space. The user space
 * library method readdir(...) already implements this so it is easy to use.
 */
class Directory: public CharacterDevice {
protected:
    /*!
     * \brief The content of the directory. May contain other directory objects.
     */
    LinkedList  dir_content;

    //! the amount of entries
    unint2      num_entries;

protected:
    /*****************************************************************************
     * Method: Directory(const char* name, ResourceType type)
     *
     * @description
     *  Additional constructor for superclasses which specify additional
     *  resource types.
     *******************************************************************************/
    Directory(const char* name, ResourceType type);

public:
    /*****************************************************************************
     * Method: Directory(const char* name)
     *
     * @description
     *  Creates a new directory object with given name.
     *******************************************************************************/
    explicit Directory(const char* name);

    virtual             ~Directory();

    /*****************************************************************************
     * Method: add(Resource* res)
     *
     * @description
     *  Adds another existing resource to this directory
     * @params
     *  res         Rresource to add to this directory.
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    virtual ErrorT      add(Resource* res);

    /*****************************************************************************
     * Method: remove(Resource* res)
     *
     * @description
     *  Tries to remove the Resource from the directory
     * @params
     *  res         Resource to remove from this directory.
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    virtual ErrorT      remove(Resource *res);

    /*****************************************************************************
     * Method: get(const char* name, unint1 name_len = 0)
     *
     * @description
     *   gets the resource with name 'name'.
     *******************************************************************************/
    virtual Resource*   get(const char* name, unint1 name_len = 0);


    /*****************************************************************************
     * Method: getNumEntries()
     *
     * @description
     *   Returns the number of entries in this directory
     *******************************************************************************/
    virtual unint2      getNumEntries() {
        return (num_entries);
    }

    /*****************************************************************************
     * Method: getContent()
     *
     * @description
     *   Returns the content of this directory
     *******************************************************************************/
    virtual LinkedList* getContent() {
        return (&dir_content);
    }

    /*****************************************************************************
     * Method: seek(int4 seek_value)
     *
     * @description
     *
     *******************************************************************************/
    virtual ErrorT      seek(int4 seek_value) {
          /* avoid negative positions */
        if (this->position + seek_value < 0)
            this->position = 0;
        else
            this->position += seek_value;

        if (this->position > this->num_entries)
            this->position = num_entries;
        return (cOk );
    }

    /*****************************************************************************
     * Method: readBytes(char *bytes, unint4 &length)
     *
     * @description
     *  Returns the contents information of this directory as encoded string
     *******************************************************************************/
    virtual ErrorT      readBytes(char *bytes, unint4 &length);

    /*****************************************************************************
     * Method: createFile(char* p_name, unint4 flags)
     *
     * @description
     *   Creates a new file inside this directory.
     *   Virtual to allow specializations of directory to create the appropriate
     *   file objects.
     *******************************************************************************/
    virtual File*       createFile(char* p_name, unint4 flags) {
        return (0);
    }


    /*****************************************************************************
     * Method: createDirectory(char* p_name, unint4 flags)
     *
     * @description
     *   Creates a new directory inside this directory.
     *   Virtual to allow specializations of directory to create the appropriate
     *   Directory objects and update the filesystem if any.
     *******************************************************************************/
    virtual Directory*  createDirectory(char* p_name, unint4 flags);
};

/*
 * Overlay Directory class. Used to create directory overlayings.
 * Therefore an existing directory is referenced and overlayed on top
 * of another given base directory. The overlay directory entries
 * always take precedence over the base directory entries.
 */
class OverlayDirectory : public Directory {
private:
    /* Pointer to the directory we are overlaying or null if none */
    Directory* baseDir;

    /* The directory to be used as the overlay */
    Directory* overlayDir;

public:
    OverlayDirectory(const char* name, Directory* baseDir, Directory* overlay);

    /*****************************************************************************
     * Method: get(const char* name, unint1 name_len = 0)
     *
     * @description
     *   gets the resource with name 'name' inside the overlay directory.
     *   if not found it tries to get the resource with name inside the
     *   base directory which was overlayed.
     *******************************************************************************/
    Resource*   get(const char* name, unint1 name_len = 0);

    /*****************************************************************************
     * Method: createFile(char* p_name, unint4 flags)
     *
     * @description
     *  Creates a new file inside the overlay directory. The
     *  base directory stays untouched.
     *******************************************************************************/
    File*       createFile(char* p_name, unint4 flags) {
          return (this->overlayDir->createFile(p_name, flags));
    }

    /*****************************************************************************
     * Method: createDirectory(char* p_name, unint4 flags)
     *
     * @description
     *  Creates a new Directory inside the overlay directory. The
     *  base directory stays untouched.
     *******************************************************************************/
    Directory*  createDirectory(char* p_name, unint4 flags) {
          return (this->overlayDir->createDirectory(p_name, flags));
    }

    /*****************************************************************************
     * Method: readBytes(char *bytes, unint4 &length)
     *
     * @description
     *
     *******************************************************************************/
    ErrorT      readBytes(char *bytes, unint4 &length);

    /*****************************************************************************
     * Method: resetPosition()
     *
     * @description
     *  Resets the entry to read position of both overlay and directory.
     *******************************************************************************/
    ErrorT      resetPosition() {
            overlayDir->resetPosition();
            if (baseDir != 0)
                baseDir->resetPosition();
            return (cOk);
    }

    /*****************************************************************************
     * Method: seek(int4 seek_value)
     *
     * @description
     *  Seeks to the given entry by offset.
     *******************************************************************************/
    ErrorT      seek(int4 seek_value);

    /*****************************************************************************
     * Method: getNumEntries()
     *
     * @description
     *  Returns the number of directory entries of the overlayed directory.
     *******************************************************************************/
    unint2      getNumEntries();

    /*****************************************************************************
     * Method: getContent()
     *
     * @description
     *
     *******************************************************************************/
    LinkedList* getContent();

    /*****************************************************************************
     * Method: remove(Resource *res)
     *
     * @description
     *  Removes the given resource. First tries to remove from overlay.
     *
     *******************************************************************************/
    ErrorT      remove(Resource *res) {
        if (overlayDir->getContent()->getItem(res)) {
            return (overlayDir->remove(res));
        } else {
            if (baseDir != 0)
                return (baseDir->remove(res));
        }
        return (cError);
    }

    /*****************************************************************************
     * Method: add(Resource* res)
     *
     * @description
     *  Adds the given resource to the overlay
     *
     *******************************************************************************/
    ErrorT      add(Resource* res) {
        return (overlayDir->add(res));
    }
};

#endif /*DIRECTORY_H_*/
