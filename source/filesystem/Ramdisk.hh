/*
 * Ramdisk.hh
 *
 *  Created on: 22.02.2014
 *     Copyright & Author: dbaldin
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
    /*****************************************************************************
     * Method: Ramdisk(T_Ramdisk_Init* init)
     *
     * @description
     *  Creates a new ramdisk with the given init paramters.
     *******************************************************************************/
    explicit Ramdisk(T_Ramdisk_Init* init);

    virtual ~Ramdisk();

    /*****************************************************************************
     * Method: allocateBlock(unint4 prev)
     *
     * @description
     *   Allocates a ramdisk block using the block number not
     *   the physical address
     *******************************************************************************/
    unint4 allocateBlock(unint4 prev);

    /*****************************************************************************
     * Method: freeBlock(unint4 blockNum)
     *
     * @description
     *   Fress a block
     *******************************************************************************/
    int freeBlock(unint4 blockNum);

    /*****************************************************************************
     * Method: getNextBlock(unint4 currentBlock, bool allocate)
     *
     * @description
     *   Returns the next block of the block list and allocates it if allocate is set
     *******************************************************************************/
    unint4 getNextBlock(unint4 currentBlock, bool allocate);

    /*****************************************************************************
     * Method: getFirstBlockAddress()
     *
     * @description
     *   Returns the physical address of the first ramdisk block
     *******************************************************************************/
    unint4 getFirstBlockAddress() {
        return (firstBlock);
    }
};

class RamdiskDirectory: public Directory {
private:
    Ramdisk* myRamDisk;

public:
    RamdiskDirectory(Ramdisk* myRamDisk, char* name) : Directory(name) {
        this->myRamDisk = myRamDisk;
    }

    /*****************************************************************************
     * Method: remove(Resource *res)
     *
     * @description
     *  Tries to delete the resource from the directory
     * @params
     *  res         Resource to delete
     * @returns
     *  int         Error Code
     *******************************************************************************/
    ErrorT remove(Resource *res);

    /*****************************************************************************
     * Method: createFile(char* name, unint4 flags)
     *
     * @description
     *  Creates a new File inside the Ramdisk Directory
     * @params
     *
     * @returns
     *  File*       The file created or null on error
     *******************************************************************************/
    File* createFile(char* name, unint4 flags);

    /*****************************************************************************
     * Method: createDirectory(char* name, unint4 flags)
     *
     * @description
     *  Creates a new Ramdisk Directory inside the Ramdisk Directory
     * @params
     *
     * @returns
     *  Directory*       The Directory created or null on error
     *******************************************************************************/
    Directory* createDirectory(char* name, unint4 flags);
};

class RamdiskFile: public File {
    friend class RamdiskDirectory;
private:
    Ramdisk* myRamDisk;

    unint4   myBlockNumber;

    unint4   currentBlock;

    unint4   readPos;

public:
    RamdiskFile(Ramdisk* myRamdisk, unint4 blockNum, char* name, int flags);

    /*****************************************************************************
     * Method: readBytes(char *bytes, unint4 &length)
     *
     * @description
     *  Reads up to length number of bytes from the file into the
     *  array. Length is updated to the actual number of bytes read.
     *******************************************************************************/
    ErrorT readBytes(char *bytes, unint4 &length);

    /*****************************************************************************
     * Method: writeBytes(const char *bytes, unint4 length)
     *
     * @description
     *  Writes up to length number of bytes to the file at its current position
     *  from the given array. Length is updated to the actual number of bytes written.
     *******************************************************************************/
    ErrorT writeBytes(const char *bytes, unint4 length);

    /*****************************************************************************
     * Method: resetPosition()
     *
     * @description
     *   Resets the file position to its beginning.
     *******************************************************************************/
    ErrorT resetPosition();
};

#endif /* RAMDISK_HH_ */
