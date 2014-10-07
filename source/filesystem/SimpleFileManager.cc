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

//be able to log
extern Kernel* theOS;

SimpleFileManager::SimpleFileManager() :
        rootDir("root")

{
    // create dir dev/
    Directory* devDir = new Directory("dev");
    // create dir dev/comm
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

ErrorT SimpleFileManager::registerResource(Resource* res) {

    if (res != 0)
    {
        if ((res->getType() & (cGenericDevice | cStreamDevice | cBlockDevice))
                != 0)
        {
            // register this resource under dev/
            getDirectory("dev")->add(res);
            return (cOk );
        }
        else if (res->getType() & cCommDevice)
        {
            // register under dev/comm
            getDirectory("dev/comm")->add(res);
            return (cOk );
        }
        else if (res->getType() == cDirectory)
        {
            rootDir.add(res);
            return (cOk );
        }
#if not MINIMAL_FILESYSTEM
        else if (res->getType() == cSharedMem)
        {
            getDirectory("mem")->add(res);
            return (cOk );
        }
#endif
        return (cInvalidResourceType );
    }
    else
        return (cNullPointerProvided );

}

ErrorT SimpleFileManager::unregisterResource(Resource* res) {
    if (res != 0)
    {
        if ((res->getType() & (cGenericDevice | cStreamDevice | cBlockDevice))
                != 0)
        {
            // resource under dev/
            getDirectory("dev")->remove(res);
            return (cOk );
        }
        else if (res->getType() == cCommDevice)
        {
            // register under dev/comm
            getDirectory("dev/comm")->remove(res);
            return (cOk );
        }
#if not MINIMAL_FILESYSTEM
        else if (res->getType() == cSharedMem)
        {
            getDirectory("mem")->remove(res);
            return (cOk );
        }
#endif
        return (cInvalidResourceType );
    }
    else
        return (cNullPointerProvided );
}

Resource*
SimpleFileManager::getResourceByNameandType(const char* pathname, ResourceType typefilter) {

    // parse the directory tree for a resource matching the pathname
    ASSERT(pathname);

    register Resource* res = &rootDir;
    register Directory* dir;

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

    // if not found root dir name assumned
    if (tokenlen < 0)
        tokenlen = 0;

    // check for root dir match
    if (tokenlen == 0)
    {
        // pathname == "/";
        if ((res->getType() & typefilter) != 0)
        {
            LOG(FILESYSTEM, DEBUG, "Filesystem: Resource %s found",pathname);
            return (res);
        }
        else
        {
            LOG(FILESYSTEM, ERROR, "Filesystem: Resource %s found but invalid type!",pathname);
            LOG(FILESYSTEM, ERROR, "Filesystem: Resource has type %d!",res->getType());
            return (0);
        }
    }

    while (token != 0)
    {
        // if we got here res is another directory and we haven't finished searching
        dir = static_cast<Directory*>(res);
        res = dir->get(token, tokenlen);
        if (res != 0)
        {
            // the directory contains the next token
            // check if this is the last token
            token += tokenlen;
            while (token[0] == '/')
                token++;
            tokenlen = strpos2(token, '/');

            if (tokenlen == 0)
            {
                // end of pathname
                // check whether the resource we found is of correct type
                if ((res->getType() & typefilter) != 0)
                {
                    LOG(FILESYSTEM, DEBUG, "Filesystem: Resource %s found",pathname);
                    return (res);
                }
                else
                {
                    LOG(FILESYSTEM, ERROR, "Filesystem: Resource %s found but invalid type!",pathname);
                    LOG(FILESYSTEM, ERROR, "Filesystem: Resource has type %d!",res->getType());
                    return (0);
                }
            }
            else
            {
                // check if the res is not an directory
                // if so there is an error
                if (res->getType() != cDirectory)
                {
                    LOG(FILESYSTEM, ERROR, "Filesystem: Wrong subname %s! Directory expected!",token);
                    return (0);
                }
            }

        }
        else
        {
            // path/file not found
            LOG(FILESYSTEM, DEBUG, "Filesystem: Directory or File not found %s!",token);
            return (0);
        }
    }

    // if we got here the pathname was invalid
    return (0);
}

Resource* SimpleFileManager::getResource(const char* pathname) {

    LOG(FILESYSTEM, DEBUG, "Filesystem: getResource(%s)",pathname);

    // get the resource by the pathname and set the filter to any type except a directory!
    return (this->getResourceByNameandType(pathname, cAnyResource));
}

Directory*
SimpleFileManager::getDirectory(const char* dir) {
    LOG(FILESYSTEM, DEBUG,"Filesystem: getDirectory(%s)",dir);
    return ((Directory*) this->getResourceByNameandType(dir, cDirectory));
}
