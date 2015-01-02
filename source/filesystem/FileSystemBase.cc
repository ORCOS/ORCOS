/*
 * FileSystemBase.cc
 *
 *  Created on: 19.06.2013
 *      Copyright & Author: dbaldin
 */

#include <filesystem/SysFs.hh>
#include "FileSystemBase.hh"

FileSystemBase::FileSystemBase(Partition* p_myPartition) {
    this->myPartition = p_myPartition;
    /* we are invalid by default
       only if superclass knows for sure this is a valid file system
       this may be set to true */
    this->isValid = false;
    numBlocks = 0;
    blockSize = 512;
    freeBlocks = 0;

    /* create sys fs entries */
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
}

FileSystemBase::~FileSystemBase() {
}

