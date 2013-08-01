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

#ifndef FILEMANAGER_H_
#define FILEMANAGER_H_

#include "inc/const.hh"
#include "inc/types.hh"
#include "filesystem/Resource.hh"

/*!
 * \brief Base filemanager class defining interface for filemanager implementations
 *
 * \ingroup filesystem
 *
 * All filesystems in ORCOS need to implement the methods (or at least some of these methods).
 * This class is just an interface class and implementations do not need to inherit this class
 * since due to SCL all references to the filesystem will be of the specific implementation class.
 *
 */
class Filemanager {
public:
    Filemanager();
    ~Filemanager();

    /*!
     * \brief Registration method for existing resources (e.g devices)
     */
    ErrorT registerResource( Resource* res ) {
        return cNotImplemented;
    }

    /*!
     * \brief creates a new resource by full path name (e.g new file /dir1/file1.txt)
     */
    Resource* createResource( const char* pathname, int1 flags ) {
        return 0;
    }

    /*!
     * \brief Returns an existing resource by full path name e.g /dev/serial0
     */
    Resource* getResource( const char* pathname ) {
        return 0;
    }

    /*!
     * \brief Removes a resource from the file system.
     *
     * Depending on the flags different operations can be performed.
     */
    ErrorT removeResource( char* pathname, int1 flags ) {
        return cNotImplemented;
    }
};

#endif /*FILEMANAGER_H_*/
