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

#include "SimpleFileManager.hh"
#include "kernel/Kernel.hh"
#include <stringtools.hh>
#include "inc/error.hh"

extern Kernel* theOS;

SimpleFileManager::SimpleFileManager() :
        rootDir("root") {
    root = &rootDir;

    /* create dir dev/ */
    Directory* devDir = new Directory("dev");
    /* create dir dev/comm */
    Directory* commDir = new Directory("comm");

    devDir->add(commDir);
    rootDir.add(devDir);

#if not MINIMAL_FILESYSTEM
    Directory* memDir = new Directory("mem");
    Directory* userDir = new Directory("usr");
    Directory* mntDir = new Directory("mnt");

    rootDir.add(userDir);
    rootDir.add(mntDir);
    rootDir.add(memDir);
#endif
}

SimpleFileManager::~SimpleFileManager() {
}

/*****************************************************************************
 * Method: SimpleFileManager::registerResource(Resource* res)
 *
 * @description
 *  Registration method for existing resources (e.g devices)
 * @params
 *  res         Resource to register
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT SimpleFileManager::registerResource(Resource* res) {
    if (res != 0) {
        Directory* parentDir;
        Directory* targetDir = 0;

        if ((res->getType() & (cGenericDevice | cStreamDevice | cBlockDevice)) != 0) {
            /* register this resource under dev/ */
            targetDir = getDirectory("dev", parentDir);
        } else if (res->getType() & cCommDevice) {
            /* register under dev/comm */
            targetDir = getDirectory("dev/comm", parentDir);
        } else if (res->getType() == cDirectory) {
            targetDir = &rootDir;
        } else if (res->getType() == cSharedMem) {
#if not MINIMAL_FILESYSTEM
            targetDir = getDirectory("mem", parentDir);
#endif
        }
        if (targetDir != 0) {
            return (targetDir->add(res));
        } else {
            return (cInvalidResourceType );
        }
    }

    return (cNullPointerProvided );
}

/*****************************************************************************
 * Method: SimpleFileManager::unregisterResource(Resource* res)
 *
 * @description
 *  Unregistration method for existing resources (e.g devices)
 * @params
 *  res         Resource to unregister
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT SimpleFileManager::unregisterResource(Resource* res) {
    if (res != 0) {
        Directory* parentDir;
        Directory* targetDir = 0;

        if ((res->getType() & (cGenericDevice | cStreamDevice | cBlockDevice)) != 0) {
            /* resource under dev/ */
            targetDir = getDirectory("dev", parentDir);
        } else if (res->getType() == cCommDevice) {
            /* register under dev/comm */
            targetDir = getDirectory("dev/comm", parentDir);
        }  else if (res->getType() == cSharedMem) {
#if not MINIMAL_FILESYSTEM
            targetDir = getDirectory("mem", parentDir);
#endif
        }

        if (targetDir != 0) {
            return (targetDir->remove(res));
        } else {
            return (cInvalidResourceType );
        }
    }

    return (cNullPointerProvided );
}

/*****************************************************************************
 * Method: SimpleFileManager::getResourceByNameandType(const char* pathname,
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
Resource* SimpleFileManager::getResourceByNameandType(const char* pathname, ResourceType typefilter, Directory* &parentDir) {
    /* parse the directory tree for a resource matching the pathname */
    ASSERT(pathname);

    register Resource* res = root;
    register Directory* dir;
    parentDir = 0;

    /*
     * The following code parses the pathname string on substrings
     * by the character '/'. No copy operation is done. It is assumed
     * that the pathname is accessible in data memory.
     */

    char* token = const_cast<char*>(pathname);
    int tokenlen;
    while (token[0] == '/')
        token++;
    tokenlen = strpos2(token, '/');

    /* if not found root dir name assumed */
    if (tokenlen < 0)
        tokenlen = 0;

    /* check for root dir match */
    if (tokenlen == 0) {
        /* pathname == "/"; */
        if ((res->getType() & typefilter) != 0) {
            LOG(FILESYSTEM, TRACE, "Filesystem: Resource %s found", pathname);
            return (res);
        } else {
            LOG(FILESYSTEM, ERROR, "Filesystem: Resource %s found but invalid type!", pathname);
            LOG(FILESYSTEM, ERROR, "Filesystem: Resource has type %d!", res->getType());
            return (0);
        }
    }

    while (token != 0) {
        /* if we got here res is another directory and we haven't finished searching */
        dir = static_cast<Directory*>(res);
        res = dir->get(token, tokenlen);
        if (res != 0) {
            parentDir = dir;
            /* the directory contains the next token
             * check if this is the last token */
            token += tokenlen;
            while (token[0] == '/') {
                token++;
            }
            tokenlen = strpos2(token, '/');

            if (tokenlen == 0) {
                /* end of pathname
                 * check whether the resource we found is of correct type */
                if ((res->getType() & typefilter) != 0) {
                    LOG(FILESYSTEM, TRACE, "Filesystem: Resource %s found", pathname);
                    return (res);
                } else {
                    LOG(FILESYSTEM, ERROR, "Filesystem: Resource %s found but invalid type!", pathname);
                    LOG(FILESYSTEM, ERROR, "Filesystem: Resource has type %d!", res->getType());
                    return (0);
                }
            } else {
                /* check if the res is not an directory
                 * if so there is an error */
                if (res->getType() != cDirectory) {
                    LOG(FILESYSTEM, ERROR, "Filesystem: Wrong subname %s! Directory expected!", token);
                    return (0);
                }
            }

        } else {
            /* path/file not found */
            LOG(FILESYSTEM, DEBUG, "Filesystem: Directory or File not found %s!", token);
            return (0);
        }
    }

    /* if we got here the pathname was invalid */
    return (0);
}

/*****************************************************************************
 * Method: SimpleFileManager::getResource(const char* pathname)
 *
 * @description
 *  Returns the resource by the full pathname
 * @params
 *  path        Path to the resource
 * @returns
 *  Resource*   The resource if found or null
 *******************************************************************************/
Resource* SimpleFileManager::getResource(const char* pathname, Directory* &parentDir) {
    LOG(FILESYSTEM, DEBUG, "Filesystem: getResource(%s)", pathname);

    /* get the resource by the pathname and set the filter to any type except a directory! */
    return (this->getResourceByNameandType(pathname, cAnyResource, parentDir));
}

/*****************************************************************************
 * Method: SimpleFileManager::getDirectory(const char* dir, Directory* &parentDir)
 *
 * @description
 *  Returns the directory object if found.
 * @params
 *
 * @returns
 *  Directory* The directory if found
 *******************************************************************************/
Directory* SimpleFileManager::getDirectory(const char* dir, Directory* &parentDir) {
    LOG(FILESYSTEM, DEBUG, "Filesystem: getDirectory(%s)", dir);
    return (static_cast<Directory*>(this->getResourceByNameandType(dir, cDirectory, parentDir)));
}

/*****************************************************************************
 * Method: SimpleFileManager::overlayDirectory(Directory* overlay, char* name, const char* path)
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
ErrorT SimpleFileManager::overlayDirectory(Directory* overlay, const char* path) {
    if (overlay == 0 || path == 0)
        return (cNullPointerProvided );

    Directory* parentDir;
    Directory* baseDir = getDirectory(path, parentDir);

    if (baseDir != 0) {
        /* Only one overlay at a time permitted */
        if (baseDir->getType() & cOverlay)
            return (cError );

        if (baseDir == root) {
            /* overlaying root */
            OverlayDirectory* ovDir = new OverlayDirectory("ROOT_OVERLAY", &rootDir, overlay);
            this->root = ovDir;
            LOG(FILESYSTEM, WARN, "SimpleFileManager::overlayDirectory overlaying root");
            return (cOk);
        } else {
            if (parentDir == 0) {
                /* BUG */
                LOG(FILESYSTEM, ERROR, "SimpleFileManager::overlayDirectory parentDir == 0 <BUG> for %s", path);
                return (cError);
            }

            /* overlaying the given directory  */
            OverlayDirectory* ovDir = new OverlayDirectory(baseDir->getName(), baseDir, overlay);
            /* replace directory entry with overlay */
            parentDir->remove(baseDir);
            parentDir->add(ovDir);

            return (cOk);
        }
    } else {
        return (cInvalidPath);
    }
}
