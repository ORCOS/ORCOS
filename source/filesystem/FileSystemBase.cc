/*
 * FileSystemBase.cc
 *
 *  Created on: 19.06.2013
 *      Copyright & Author: dbaldin
 */

#include <filesystem/SysFs.hh>
#include "FileSystemBase.hh"
#include <kernel/Kernel.hh>

extern Kernel* theOS;

FileSystemBase::FileSystemBase(Partition* p_myPartition) : Resource(cPartition, true, p_myPartition->getName()) {
    this->myPartition = p_myPartition;
    /* we are invalid by default
       only if superclass knows for sure this is a valid file system
       this may be set to true */
    this->isValid = false;
    numBlocks  = 0;
    blockSize  = 512;
    freeBlocks = 0;
    rootDir    = 0; /* to be initialized by superclass */

    /* create sys fs entries */
#if SYSFS_SUPPORT
    Directory* dir = KernelVariable::getEntry("fs", true);
    if (dir) {
        char* idstr = new char[32];
        sprintf(idstr, "%s", myPartition->getName());
        sysFsDir = new Directory(idstr);
        dir->add(sysFsDir);

        SYSFS_ADD_RO_UINT(sysFsDir, numBlocks);
        SYSFS_ADD_RO_UINT(sysFsDir, blockSize);
        SYSFS_ADD_RO_UINT(sysFsDir, freeBlocks);
    }
#endif
}


FileSystemBase::FileSystemBase(const char* name) : Resource(cPartition, true, name)  {
    this->myPartition = 0;
    this->isValid = true;
    numBlocks = 0;
    blockSize = 512;
    freeBlocks = 0;
    rootDir    = 0; /* to be initialized by superclass */

    /* create sys fs entries */
#if SYSFS_SUPPORT
    Directory* dir = KernelVariable::getEntry("fs", true);
    if (dir) {
        sysFsDir = new Directory(name);
        dir->add(sysFsDir);

        SYSFS_ADD_RO_UINT(sysFsDir, numBlocks);
        SYSFS_ADD_RO_UINT(sysFsDir, blockSize);
        SYSFS_ADD_RO_UINT(sysFsDir, freeBlocks);
    }
#endif
}

FileSystemBase::~FileSystemBase() {
#if SYSFS_SUPPORT
    Directory* dir = KernelVariable::getEntry("fs", true);
    if (dir) {
        dir->remove(sysFsDir);
        sysFsDir->invalidate();
        theOS->getMemoryManager()->scheduleDeletion(sysFsDir);
    }
#endif
    this->isValid = false;
    numBlocks  = 0;
    blockSize  = 512;
    freeBlocks = 0;
    rootDir    = 0;
    this->myPartition = 0;
}

