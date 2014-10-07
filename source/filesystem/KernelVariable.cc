/*
 * KernelVariableFile.cc
 *
 *  Created on: 23.09.2014
 *      Author: Daniel
 */

#include <filesystem/KernelVariable.hh>
#include "kernel/Kernel.hh"

#if SYSFS_SUPPORT

extern Kernel* theOS;

KernelVariable::KernelVariable(char* name, int size, void* addr, int mode, SysFs_t type) : File(name,size,type,cKernelVariable) {

    this->address = addr;
    this->mode = mode;

}

void KernelVariable::exportVariable(Directory* parent, char* name, void* address, int size, int mode, SysFs_t type) {
    if (parent == 0) return;
     parent->add(new KernelVariable(name,size,address,mode,type));
}
/*
void KernelVariable::exportVariable(Directory* parent, char* name, int* address, int size, int mode) {
    if (parent == 0) return;
    parent->add(new KernelVariable(name,size,address,mode,SYSFS_SIGNED_INTEGER));
}
void KernelVariable::exportVariable(Directory* parent, char* name, unsigned int* address, int size, int mode) {
    if (parent == 0) return;
    parent->add(new KernelVariable(name,size,address,mode,SYSFS_UNSIGNED_INTEGER));
}
void KernelVariable::exportVariable(Directory* parent, char* name, char* address, int size, int mode) {
    if (parent == 0) return;
    parent->add(new KernelVariable(name,size,address,mode,SYSFS_STRING));
}*/
/*void KernelVariable::exportVariable(Directory* parent, char* name, void* address, int size, int mode) {
    if (parent == 0) return;
    parent->add(new KernelVariable(name,size,address,mode,SYSFS_UNSIGNED_INTEGER));
}*/

void KernelVariable::init() {
    Directory* sysfs = new Directory("sys");
    /* register sys under root entry */
    theOS->getFileManager()->registerResource(sysfs);
}

Directory* KernelVariable::getEntry(char* name, bool create) {
    if (name == 0) {
        return (0);
    }

    Directory* sysfs  = theOS->getFileManager()->getDirectory("/sys");
    if (!sysfs)
        return (0);

    Resource* entry = sysfs->get(name,strlen(name));
    if (entry) {
        if (entry->getType() & cDirectory) {
            Directory* dir = (Directory*) (entry);
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

ErrorT KernelVariable::readBytes(char *bytes, unint4 &length) {

    if (mode & READ) {
        /* read the memory address */

        if (length > this->filesize)
            length = this->filesize;

        for (unint i = 0; i < this->filesize; i++) {
            bytes[i] = ((char*) this->address)[i];
        }

        return (cOk);

    } else {
        return (cError);
    }

}


ErrorT KernelVariable::writeBytes(const char *bytes, unint4 length) {
   if (mode & WRITE) {
       if (length < this->filesize) {
           return (cInvalidArgument);
       }

       length = this->filesize;

     for (unint i = 0; i < this->filesize; i++) {
         ((char*) this->address)[i] = bytes[i];
     }

     return (cOk);
   } else
       return (cError);

}

#endif
