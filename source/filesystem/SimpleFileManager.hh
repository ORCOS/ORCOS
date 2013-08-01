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

#ifndef SIMPLEFILEMANAGER_H_
#define SIMPLEFILEMANAGER_H_

#include "Directory.hh"
#include "Filemanager.hh"
#include "SCLConfig.hh"

#if HAS_PROCFS_ENABLED
#include "filesystem/SimpleProcfs.hh"
#endif

/*!
 * \brief A simple filemanager that implements a very basic unix like filesystem
 *
 * \ingroup filesystem
 *
 * This filemanager only supports accessing generic devices and
 * character devices as well as communication devices. Files are not supported.
 *
 * The filesystem is directory based and can thus be extended by any directory
 * and resource. This implementation also stores references to some predefined
 * directorys  "dev/" and "dev/comm/" for faster access to these directorys.
 * There also exists a "proc/" directory for debugging support if configured through SCL.
 */
class SimpleFileManager /*: public Filemanager  //uncommented since saves ~ 70 bytes*/
{

private:
    //! the root directory
    Directory rootDir;

public:
    SimpleFileManager();
    ~SimpleFileManager();

    /*!
     * \brief Registration method for existing resources (e.g devices)
     *
     * This implementation of a Filemanager only supports the registration
     * of devices and sockets. Files on secondary storage or somewhere else are
     * not supported here.
     */
    ErrorT registerResource( Resource* res );

    /*!
     * \brief Resource creation is not imlpemented in this filemanager.
     */
    Resource* createResource( const char* pathname, int1 flags ) {
        return 0;
    }
    ;

    /*!
     * \brief Removing resources is not implemented in this filemanger.
     */
    ErrorT removeResource( char* pathname, int1 flags ) {
        return cNotImplemented;
    }
    ;

    /*!
     * \brief Returns the resource by the full pathname
     */
    Resource* getResource( const char* pathname );

    /*!
     * \brief Returns the resource by the full pathname and filters by type
     */
    Resource* getResourceByNameandType( const char* pathname, ResourceType type );


    /*!
     * \brief Returns the directory object if found.
     */
    Directory* getDirectory( const char* dir );

};

#endif /*SIMPLEFILEMANAGER_H_*/
