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

#include "db/LinkedListDatabase.hh"
#include "filesystem/Resource.hh"
#include "hal/CharacterDeviceDriver.hh"
#include "filesystem/File.hh"
/*!
 * \brief Directory in filesystem.
 *
 * \ingroup filesystem
 *
 * The directory is a node of a tree and can thus have multiple directorys
 * or leaf nodes (resources other than a directory) as child.
 */
class Directory : public CharacterDeviceDriver {
protected:
    /*!
     * \brief The content of the directory. May contain other directory objects.
     */
    LinkedListDatabase dir_content;

    //! the amount of entries (max 256)
    unint1 num_entries;
public:
    Directory( const char* name );

    virtual ~Directory();

    //! Adds another resource to this directory
    virtual ErrorT add( Resource* res );

    //! Tries to remove the Resource from the directory
    virtual ErrorT remove(Resource *res);

    //! gets the resource with name 'name'. Maybe null if nonexistent
    virtual Resource* get( const char* name );

    //! Returns the amount of entries in this directory
    virtual int getNumEntries() {
        return (num_entries);
    }

    //! Returns the content of this directory
    virtual LinkedListDatabase* getContent() {
        return (&dir_content);
    }

    //! Returns the contents information of this directory as encoded string
    virtual ErrorT readBytes( char *bytes, unint4 &length );

    /* Creates a new file inside the directory.
     * Virtual to allow specializations of directory to create the appropriate
     * file objects.
     */
    virtual File* createFile(char* p_name, unint4 flags) {return 0;};
};

#endif /*DIRECTORY_H_*/
