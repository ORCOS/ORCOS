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
#include "SCLConfig.hh"


/*!
 * \brief A simple filemanager that implements a very basic unix like filesystem
 *
 * \ingroup filesystem
 *
 * The filesystem is directory based and can thus be extended by any directory
 * and resource. This implementation also stores references to some predefined
 * directorys  "dev/" and "dev/comm/" for faster access to these directorys.
 */
class SimpleFileManager {
private:
    Directory rootDir;

    //! the root directory
    Directory* root;

public:
    SimpleFileManager();
    ~SimpleFileManager();


    /*****************************************************************************
     * Method: registerResource(Resource* res)
     *
     * @description
     *  Registration method for existing resources (e.g devices)
     * @params
     *  res         Resource to register
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT registerResource(Resource* res);

    /*****************************************************************************
     * Method: unregisterResource(Resource* res)
     *
     * @description
     *  Unregistration method for existing resources (e.g devices)
     * @params
     *  res         Resource to unregister
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT unregisterResource(Resource* res);


    /*****************************************************************************
     * Method: overlayDirectory(Directory* overlay, const char* path)
     *
     * @description
     *  Tries to overlay the directory passed as overlay
     *  on top of the directory specified by path.
     *  If path references the root directory the root
     *  directory will be overlayed.
     *  Otherwise the resource specified by path is searched and overlayed
     *  if it is an directory. If the resource specified by the path does not
     *  exists a new directory is created with no overlay.
     * @params
     *  overlay         The directory which overlaying another directory
     *  path            Path to the directory to be overlayed
     *
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT overlayDirectory(Directory* overlay, const char* path);


    /*****************************************************************************
     * Method: getResource(const char* pathname, Directory* &parentDir)
     *
     * @description
     *  Returns the resource by the full pathname
     * @params
     *  path        Path to the resource
     * @returns
     *  Resource*   The resource if found or null
     *  parentDir   Parent directory of the resource found
     *******************************************************************************/
    Resource* getResource(const char* pathname, Directory* &parentDir);


    /*****************************************************************************
     * Method: getResource(const char* pathname)
     *
     * @description
     *  Returns the resource by the full pathname
     * @params
     *  path        Path to the resource
     * @returns
     *  Resource*   The resource if found or null
     *******************************************************************************/
    Resource* getResource(const char* pathname) {
        Directory* dir;
        return (getResource(pathname, dir));
   }


    /*****************************************************************************
     * Method: getResourceByNameandType(const char* pathname,
     *                                  ResourceType type,
     *                                  Directory* &parentDir)
     *
     * @description
     *  Returns the resource by the full pathname and filters by type including the parent directory
     *
     * @params
     *  path        Path to the resource
     *  type        Type of the resource
     *
     * @returns
     *  Resource*   The resource if found or null
     *  parentDir   Parent directory of the resource found
     *******************************************************************************/
    Resource* getResourceByNameandType(const char* pathname, ResourceType type, Directory* &parentDir, Directory* rootDir = 0);


    Resource* getResourceByNameandType(const char* pathname, ResourceType type) {
        Directory* dir;
        return (getResourceByNameandType(pathname, type, dir));
    }

    /*****************************************************************************
     * Method: getDirectory(const char* dir, Directory* &parentDir)
     *
     * @description
     *  Returns the directory object if found.
     * @params
     *
     * @returns
     *  Directory* The directory if found
     *******************************************************************************/
    Directory* getDirectory(const char* dir, Directory* &parentDir);

    Directory* getDirectory(const char* dir) {
        Directory* parentDir;
        return (getDirectory(dir, parentDir));
    }
};

#endif /*SIMPLEFILEMANAGER_H_*/
