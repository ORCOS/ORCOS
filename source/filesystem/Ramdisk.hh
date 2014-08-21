/*
 * Ramdisk.hh
 *
 *  Created on: 22.02.2014
 *      Author: dbaldin
 */

#ifndef RAMDISK_HH_
#define RAMDISK_HH_

#include "filesystem/Directory.hh"
#include "filesystem/File.hh"

class Ramdisk {
private:

    /* The global block chain */
    unint4* blockChain;

    /* Number of entries inside the blockchain */
    unint4 blockChainEntries;

    /* Address of the first block allocatable */
    unint4 firstBlock;

public:
    Ramdisk(T_Ramdisk_Init* init);

    unint4 allocateBlock(unint4 prev);

    int freeBlock(unint4 blockNum);

    unint4 getNextBlock(unint4 currentBlock, bool allocate);

    unint4 getFirstBlockAddress() {
        return (firstBlock);
    }

    virtual ~Ramdisk();
};

class RamdiskDirectory: public Directory {

private:
    Ramdisk* myRamDisk;

public:

    RamdiskDirectory(Ramdisk* myRamDisk, char* name) : Directory(name) {
        this->myRamDisk = myRamDisk;
    }

    //! Tries to delete the resource from the directory
    ErrorT remove(Resource *res);

    /* Creates a new File inside the Ramdisk. */
    File* createFile(char* name, unint4 flags);

};

class RamdiskFile: public File {
    friend class RamdiskDirectory;
private:
    Ramdisk* myRamDisk;

    unint4 myBlockNumber;

    unint4 currentBlock;

public:
    RamdiskFile(Ramdisk* myRamdisk, unint4 blockNum, char* name, int flags);

    ErrorT readBytes(char *bytes, unint4 &length);

    ErrorT writeBytes(const char *bytes, unint4 length);

    ErrorT resetPosition();
};

#endif /* RAMDISK_HH_ */
