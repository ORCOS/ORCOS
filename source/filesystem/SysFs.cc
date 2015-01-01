/*
 * KernelVariableFile.cc
 *
 *  Created on: 23.09.2014
 *     Copyright & Author: Daniel
 */

#include <filesystem/SysFs.hh>
#include "kernel/Kernel.hh"

#if SYSFS_SUPPORT

extern Kernel* theOS;

KernelVariable::KernelVariable(char* name, int size, void* addr, int mode, SysFs_t type, sysfs_write_handler_t onWriteHandler) :
        File(name, size, type, cKernelVariable) {
    this->address = addr;
    this->mode = mode;
    this->onWriteHandler = onWriteHandler;
}

/*****************************************************************************
 * Method: KernelVariable::exportVariable(Directory* parent,
 *                                        char* name,
 *                                        void* address,
 *                                         int size,
 *                                         int mode,
 *                                         SysFs_t type)
 *
 * @description
 *
 * @params
 *
 *******************************************************************************/
void KernelVariable::exportVariable(Directory* parent, char* name, void* address, int size, int mode, SysFs_t type, sysfs_write_handler_t writeCallback) {
    if (parent == 0)
        return;

    parent->add(new KernelVariable(name, size, address, mode, type, writeCallback));
}

/*****************************************************************************
 * Method: KernelVariable::init()
 *
 * @description
 *
 * @params
 *
 *******************************************************************************/
void KernelVariable::init() {
    Directory* sysfs = new Directory("sys");
    /* register sys under root entry */
    theOS->getFileManager()->registerResource(sysfs);
}

/*****************************************************************************
 * Method: KernelVariable::getEntry(char* name, bool create)
 *
 * @description
 *
 * @params
 *
 *******************************************************************************/
Directory* KernelVariable::getEntry(char* name, bool create) {
    if (name == 0) {
        return (0);
    }

    Directory* sysfs = theOS->getFileManager()->getDirectory("/sys");
    if (!sysfs)
        return (0);

    Resource* entry = sysfs->get(name, strlen(name));
    if (entry) {
        if (entry->getType() & cDirectory) {
            Directory* dir = static_cast<Directory*>(entry);
            return (dir);
        } else {
            return (0);
        }
    } else if (create) {
        Directory* dir = new Directory(name);
        sysfs->add(dir);
        return (dir);
    }

    return (0);
}

KernelVariable::~KernelVariable() {
}

/*****************************************************************************
 * Method: KernelVariable::readBytes(char *bytes, unint4 &length)
 *
 * @description
 *
 * @params
 *
 *******************************************************************************/
ErrorT KernelVariable::readBytes(char *bytes, unint4 &length) {
    if (mode & READ) {
        /* read the memory address */

        if (length > this->filesize)
            length = this->filesize;

        for (unint i = 0; i < this->filesize; i++) {
            bytes[i] = (reinterpret_cast<char*>(this->address))[i];
        }

        return (cOk);
    } else {
        return (cError);
    }
}

/*****************************************************************************
 * Method: KernelVariable::writeBytes(const char *bytes, unint4 length)
 *
 * @description
 *
 * @params
 *
 *******************************************************************************/
ErrorT KernelVariable::writeBytes(const char *bytes, unint4 length) {
    if (mode & WRITE) {
        if (length < this->filesize) {
            return (cInvalidArgument );
        }

        length = this->filesize;

        if (onWriteHandler == 0) {

            for (unint i = 0; i < this->filesize; i++) {
                (reinterpret_cast<char*>(this->address))[i] = bytes[i];
            }

            return (cOk );
        } else {
            return (onWriteHandler(bytes, length));
        }
    } else {
        return (cError );
    }
}

#endif
