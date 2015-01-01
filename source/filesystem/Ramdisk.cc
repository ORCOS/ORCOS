/*
 * Ramdisk.cc
 *
 *  Created on: 22.02.2014
 *      Copyright & Author: dbaldin
 */

#include "Ramdisk.hh"
#include "inc/types.hh"
#include "kernel/Kernel.hh"
#include "inc/Alignment.hh"

extern Kernel* theOS;

#define BLOCK_SIZE 4096

Ramdisk::Ramdisk(T_Ramdisk_Init* init) {
    /* get Ramdisk location and size from SCL configuration */
    unint4 start    = init->StartAddress;
    unint4 end      = init->StartAddress + init->Size - 1;

    blockChain = reinterpret_cast<unint4*>(start);

    unint4 numblocks        = ((end - start) / BLOCK_SIZE);
    unint4 blockChainSize   = (sizeof(unint4*)) * numblocks;
    numblocks              -= (blockChainSize / BLOCK_SIZE);
    blockChainSize          = (sizeof(unint4*)) * numblocks;

    blockChainEntries = numblocks;

    firstBlock = start + (numblocks * sizeof(unint4*));
    firstBlock = (unint4) alignCeil(reinterpret_cast<char*>(firstBlock), BLOCK_SIZE);

    if (theOS->getRamManager() != 0) {
        /* Mark the ram disk area as used so it can not be used to allocate tasks a.s.o*/
        theOS->getRamManager()->markAsUsed(start, end, 0);
    }

    for (unint4 i = 0; i < blockChainEntries; i++)
        blockChain[i] = (unint4) -1;  // mark as free

    // create the ramdisk mount directory
    /* TODO: use SCL to get path and name */
    RamdiskDirectory* dir = new RamdiskDirectory(this, "ramdisk");
    theOS->getFileManager()->getDirectory("/mnt")->add(dir);
}

/*****************************************************************************
 * Method: Ramdisk::allocateBlock(unint4 prev)
 *
 * @description
 *   Allocates a ramdisk block using the block number not
 *   the physical address
 *******************************************************************************/
unint4 Ramdisk::allocateBlock(unint4 prev) {
    for (unint4 i = 0; i < blockChainEntries; i++)
        if (blockChain[i] == (unint4) -1) {  // -1 == free
            blockChain[i] = 0;               //  0 == End of Chain
            if (prev != (unint4) -1)
                blockChain[prev] = i;

            return (i);
        }
    return (-1);
}

/*****************************************************************************
 * Method: Ramdisk::freeBlock(unint4 blockNum)
 *
 * @description
 *   Fress a block
 *******************************************************************************/
int Ramdisk::freeBlock(unint4 blockNum) {
    if (blockNum != (unint4) -1) {
        unint4 nextBlock = blockChain[blockNum];
        blockChain[blockNum] = -1;

        while (nextBlock != 0 && nextBlock != (unint4) -1) {
            blockNum = blockChain[nextBlock];
            blockChain[nextBlock] = (unint4) -1;
            nextBlock = blockNum;
        }
    }

    return (cOk );
}

/*****************************************************************************
 * Method: Ramdisk::getNextBlock(unint4 currentBlock, bool allocate)
 *
 * @description
 *   Returns the next block of the block list and allocates it if allocate is set
 *******************************************************************************/
unint4 Ramdisk::getNextBlock(unint4 currentBlock, bool allocate) {
    if (currentBlock != (unint4) -1) {
        unint4 nextBlock = blockChain[currentBlock];

        if ((nextBlock == 0 || nextBlock == (unint4) -1) && allocate) {
            return (allocateBlock(currentBlock));
        } else {
            return (nextBlock);
        }
    } else {
        return (-1);
    }
}

Ramdisk::~Ramdisk() {
}


/*****************************************************************************
 * Method: RamdiskDirectory::remove(Resource *res)
 *
 * @description
 *  Tries to delete the resource from the directory
 * @params
 *  res         Resource to delete
 * @returns
 *  int         Error Code
 *******************************************************************************/
ErrorT RamdiskDirectory::remove(Resource* res) {
    int error = Directory::remove(res);

    if (isError(error)) {
        LOG(FILESYSTEM, ERROR, "RamdiskDirectory::remove(): Error removing resource %s", res->getName());
        return (error);
    }

    if (res->getType() & cFile) {
        RamdiskFile* f = static_cast<RamdiskFile*>(res);
        myRamDisk->freeBlock(f->myBlockNumber);
    }

    delete res;

    return (cOk );
}

/*****************************************************************************
 * Method: RamdiskDirectory::createFile(char* name, unint4 flags)
 *
 * @description
 *  Creates a new File inside the Ramdisk Directory
 * @params
 *
 * @returns
 *  File*       The file created or nul
 *******************************************************************************/
File* RamdiskDirectory::createFile(char* name, unint4 flags) {
    /* avoid creating duplicate file names */
    if (get(name, strlen(name))) {
        LOG(FILESYSTEM, ERROR, "RamdiskDirectory::createFile(): Filename already exists '%s'", name);
        return (0);
    }

    /* search a free entry */
    unint4 block = this->myRamDisk->allocateBlock(-1);
    if (block == (unint4) -1) {
        LOG(FILESYSTEM, ERROR, "RamdiskDirectory::createFile(): No free block");
        return (0);
    }

    char* fileName = reinterpret_cast<char*>(theOS->getMemoryManager()->alloc(strlen(name) + 1));
    strcpy(fileName, name);

    RamdiskFile* f = new RamdiskFile(this->myRamDisk, block, fileName, flags);
    this->add(f);
    return (f);
}


Directory* RamdiskDirectory::createDirectory(char* name, unint4 flags) {
    if (get(name, strlen(name))) {
       LOG(FILESYSTEM, ERROR, "RamdiskDirectory::createDirectory(): Directory already exists '%s'", name);
       return (0);
    }

    char* dirName = reinterpret_cast<char*>(theOS->getMemoryManager()->alloc(strlen(name) + 1));
    strcpy(dirName, name);

    RamdiskDirectory* f = new RamdiskDirectory(this->myRamDisk, dirName);
    this->add(f);
    return (f);
}

/*****************************************************************************
 * Method: RamdiskFile::readBytes(char *bytes, unint4 &length)
 *
 * @description
 *  Reads up to length number of bytes from the file into the
 *  array. Length is updated to the actual number of bytes read.
 *******************************************************************************/
ErrorT RamdiskFile::readBytes(char* bytes, unint4& length) {
    unint4 sector_pos;
    unint4 sector_read_len;
    unint4 pos = 0;

    unint4 readlength = length;
    // be sure we are not reading over the end of the file
    if ((this->filesize - this->readPos) < readlength)
        readlength = (this->filesize - this->readPos);

    char* buffer;
    bool sector_changed = false;

    while (readlength > 0) {
        // position inside our current sector
        // sector_pos         = this->position % sector_size;
        // sector size must be a multiple of 2
        sector_pos = this->readPos & (BLOCK_SIZE - 1);

        // set readlength to remaining bytes in this sector
        sector_read_len = BLOCK_SIZE - sector_pos;

        // check if we read less than the remaining bytes in this sector
        if (readlength < sector_read_len) {
            sector_read_len = readlength;
        } else {
            sector_changed = true;
        }

        buffer = reinterpret_cast<char*>((currentBlock * BLOCK_SIZE) + myRamDisk->getFirstBlockAddress());
        // copy the desired bytes in this sector into the buffer
        memcpy(&bytes[pos], &buffer[sector_pos], sector_read_len);

        // increase position by bytes read
        this->readPos += sector_read_len;
        pos += sector_read_len;

        // check if we reached the sector boundary
        if (sector_changed) {
            currentBlock = myRamDisk->getNextBlock(currentBlock, false);
            if (currentBlock == (unint4) -1) {
                // end of file reached
                length = pos;
                return (cOk );
            }
        }

        sector_changed = false;
        // decrease total amount to read by read amount of bytes
        readlength -= sector_read_len;
    }

    length = pos;
    return (cOk );
}

/*****************************************************************************
 * Method: RamdiskFile::writeBytes(const char *bytes, unint4 length)
 *
 * @description
 *  Writes up to length number of bytes to the file at its current position
 *  from the given array. Length is updated to the actual number of bytes written.
 *******************************************************************************/
ErrorT RamdiskFile::writeBytes(const char* bytes, unint4 length) {
    unint4 sector_pos;
    unint4 sector_write_len;
    bool sector_changed = false;
    unint4 pos = 0;  // position inside the bytes array

    char* buffer;

    // keep writing while we have bytes to write
    while (length > 0) {
        // check if the next write operation is overwriting something or appending
        sector_pos = this->position & (BLOCK_SIZE - 1);  // position inside the sector
        sector_write_len = BLOCK_SIZE - sector_pos;      // the length we are writing inside this sector

        // check if we read less than the remaining bytes in this sector
        if (length < sector_write_len) {
            sector_write_len = length;
        } else {
            sector_changed = true;
        }

        buffer = reinterpret_cast<char*>((currentBlock * BLOCK_SIZE) + myRamDisk->getFirstBlockAddress());

        // copy the desired bytes in this sector into the buffer
        memcpy(&buffer[sector_pos], &bytes[pos], sector_write_len);

        // increase position by bytes read
        this->position += sector_write_len;
        pos += sector_write_len;

        // check if we reached the sector boundary
        if (sector_changed) {
            // append if neccessary
            currentBlock = myRamDisk->getNextBlock(currentBlock, true);
            if (currentBlock == (unint4) -1) {
                // no more memory..
                return (cDeviceMemoryExhausted );
            }
        }

        sector_changed = false;
        // decrease total amount to read by read amount of bytes
        length -= sector_write_len;
    }

    if (this->position > this->filesize) {
        this->filesize = this->position;
    }

    return (cOk );
}

RamdiskFile::RamdiskFile(Ramdisk* myRamdisk, unint4 blockNum, char* name, int flags) :
        File(name, 0, flags) {
    this->myRamDisk     = myRamdisk;
    this->myBlockNumber = blockNum;
    currentBlock        = myBlockNumber;
    readPos = 0;
}

/*****************************************************************************
 * Method: RamdiskFile::resetPosition()
 *
 * @description
 *   Resets the file position to its beginning.
 *******************************************************************************/
ErrorT RamdiskFile::resetPosition() {
    //File::resetPosition();
    readPos = 0;
    currentBlock = myBlockNumber;
    return (cOk );
}
