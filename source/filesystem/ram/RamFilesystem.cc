/*
 * Ramdisk.cc
 *
 *  Created on: 22.02.2014
 *      Copyright & Author: dbaldin
 */

#include "RamFilesystem.hh"

#include "inc/types.hh"
#include "kernel/Kernel.hh"
#include "inc/Alignment.hh"

extern Kernel* theOS;

#define RAMDISK_BLOCK_SIZE 4096
#define NUM_BLOCKS_PER_SUPERBLOCK 128


RamFilesystem::RamFilesystem(T_RamFilesystem_Init* init) : FileSystemBase("ramdisk"), myMutex("RamFsMutex") {
    /* get Ramdisk location and size from SCL configuration */
    unint4 start    = init->StartAddress;
    unint4 end      = init->StartAddress + init->Size - 1;

    /* calculate number of blocks. drop insufficient sized last block  */
    unint4 numblocks        = ((end - start) / RAMDISK_BLOCK_SIZE);
    /* calculate size of block chain region needed for this number of blocks */
    unint4 blockChainSize   = (sizeof(unint4*)) * numblocks;
    /* calculate number of super blocks */
    superBlocks             = (numblocks / NUM_BLOCKS_PER_SUPERBLOCK) +1;
    /* calculate size of superblock chain */
    unint4 superBlockSize   = superBlocks * sizeof(struct SuperBlock);

    /* substract the blockchain and superblockchain size */
    numblocks              -= ((blockChainSize / RAMDISK_BLOCK_SIZE) + 1);
    numblocks              -= ((superBlockSize / RAMDISK_BLOCK_SIZE) + 1);

    blockChainSize          = (sizeof(unint4*)) * numblocks;
    blockChainEntries       = numblocks;

    superBlocks             = (numblocks / NUM_BLOCKS_PER_SUPERBLOCK) +1;
    superBlockSize          = superBlocks * sizeof(struct SuperBlock);

    superBlockChain         = (struct SuperBlock*) start;
    blockChain              = reinterpret_cast<unint4*>(alignCeil(reinterpret_cast<char*>(start + superBlockSize), RAMDISK_BLOCK_SIZE));

    firstBlock              = ((unint4) blockChain) + blockChainSize;
    firstBlock              = (unint4) alignCeil(reinterpret_cast<char*>(firstBlock), RAMDISK_BLOCK_SIZE);

    if (theOS->getRamManager() != 0) {
        /* Mark the ram disk area as used so it can not be used to allocate tasks a.s.o*/
        theOS->getRamManager()->markAsUsed(start, end, 0);
    }

    for (unint4 i = 0; i < blockChainEntries; i++) {
        blockChain[i] = (unint4) -1;  // mark as free
    }

    for (unint4 i = 0; i < superBlocks; i++) {
        superBlockChain[i].freeBlocks    = NUM_BLOCKS_PER_SUPERBLOCK;
        superBlockChain[i].nextFreeBlock = 0;
        superBlockChain[i].numBlocks     = NUM_BLOCKS_PER_SUPERBLOCK;
        superBlockChain[i].reserved      = 0;
    }


    /* set filesystem base variables */
    numBlocks  = blockChainEntries;
    freeBlocks = numBlocks;
    blockSize  = RAMDISK_BLOCK_SIZE;

    /* update number of blocks in last superblock */
    superBlockChain[superBlocks-1].numBlocks  = numBlocks - ((superBlocks-1) *  NUM_BLOCKS_PER_SUPERBLOCK);
    superBlockChain[superBlocks-1].freeBlocks = superBlockChain[superBlocks-1].numBlocks;

    LOG(FILESYSTEM, WARN, "Ramdisk: Blocks          0x%08x-0x%08x", firstBlock, firstBlock + (numBlocks * RAMDISK_BLOCK_SIZE));
    LOG(FILESYSTEM, WARN, "Ramdisk: SuperBlockChain 0x%08x-0x%08x", superBlockChain, ((unint4) superBlockChain) + superBlockSize);
    LOG(FILESYSTEM, WARN, "Ramdisk: BlockChain      0x%08x-0x%08x", blockChain, ((unint4) blockChain) + blockChainSize);
    LOG(FILESYSTEM, WARN, "Ramdisk: #Blocks         %u", numBlocks);
    LOG(FILESYSTEM, WARN, "Ramdisk: SuperBlocks     %u", superBlocks);
    LOG(FILESYSTEM, WARN, "Ramdisk: LastSB#blks     %u", superBlockChain[superBlocks-1].numBlocks);

    // create the ramdisk mount directory
    /* TODO: use SCL to get path and name */
    RamFilesystemDirectory* dir = new RamFilesystemDirectory(this, "ramdisk");
    theOS->getFileManager()->getDirectory("/mnt")->add(dir);
}

/*****************************************************************************
 * Method: Ramdisk::allocateBlock(unint4 prev)
 *
 * @description
 *   Allocates a Ramdisk block using the block number not
 *   the physical address
 *******************************************************************************/
unint4 RamFilesystem::allocateBlock(unint4 prev) {
    myMutex.acquire();
    for (unint4 superBlock = 0; superBlock < superBlocks; superBlock++) {
        if (superBlockChain[superBlock].freeBlocks > 0) {
            /* calculate blockChain Entry of superBlock */
            unint4 blockChainEntry = superBlock * NUM_BLOCKS_PER_SUPERBLOCK;

            // check if next free block is known
            if (superBlockChain[superBlock].nextFreeBlock != 255) {
                /* free block known.. perfect .. use the block */
                blockChainEntry += superBlockChain[superBlock].nextFreeBlock;
                superBlockChain[superBlock].nextFreeBlock = 255;
            } else {
                /* search inside the blockchain to find the free block */
                int numBlocks = superBlockChain[superBlock].numBlocks;
                for (int i = 0; i < numBlocks; i++) {
                    if (blockChain[blockChainEntry + i] == (unint4) -1) {
                        blockChainEntry = blockChainEntry + i;
                        break;
                    }
                }
            }
            superBlockChain[superBlock].freeBlocks--;

            blockChain[blockChainEntry] = 0;               //  0 == End of Chain
            if (prev != (unint4) -1)
               blockChain[prev] = blockChainEntry;

           freeBlocks--;
           myMutex.release();
           return (blockChainEntry);
        }
    }

    myMutex.release();
    return (-1);
}

/*****************************************************************************
 * Method: Ramdisk::freeBlock(unint4 blockNum)
 *
 * @description
 *   Frees a block chain starting at block #blocknum
 *******************************************************************************/
int RamFilesystem::freeBlock(unint4 blockNum) {
    myMutex.acquire();

    if (blockNum != (unint4) -1) {
        unint4 nextBlock     = blockChain[blockNum];
        unint4 superBlock    = blockNum / NUM_BLOCKS_PER_SUPERBLOCK;
        blockChain[blockNum] = -1;
        superBlockChain[superBlock].freeBlocks++;
        superBlockChain[superBlock].nextFreeBlock = blockNum & (NUM_BLOCKS_PER_SUPERBLOCK - 1);
        freeBlocks++;

        while (nextBlock != 0 && nextBlock != (unint4) -1) {
            superBlock            = nextBlock / NUM_BLOCKS_PER_SUPERBLOCK;
            superBlockChain[superBlock].freeBlocks++;
            superBlockChain[superBlock].nextFreeBlock = nextBlock & (NUM_BLOCKS_PER_SUPERBLOCK - 1);

            blockNum              = blockChain[nextBlock];
            blockChain[nextBlock] = (unint4) -1;
            nextBlock             = blockNum;
            freeBlocks++;
        }
    }

    myMutex.release();
    return (cOk );
}

/*****************************************************************************
 * Method: Ramdisk::getNextBlock(unint4 currentBlock, bool allocate)
 *
 * @description
 *   Returns the next block of the block list and allocates it if allocate is set
 *******************************************************************************/
unint4 RamFilesystem::getNextBlock(unint4 currentBlock, bool allocate) {
    if (currentBlock != (unint4) -1) {
        unint4 nextBlock = blockChain[currentBlock];

        if ((nextBlock == 0 || nextBlock == (unint4) -1) && allocate) {
            nextBlock = allocateBlock(currentBlock);
            LOG(FILESYSTEM, TRACE, "Ramdisk: allocated Block  %u", nextBlock);
            return (nextBlock);
        } else {
            return (nextBlock);
        }
    } else {
        return (-1);
    }
}

RamFilesystem::~RamFilesystem() {
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
ErrorT RamFilesystemDirectory::remove(Resource* res) {
    int error = Directory::remove(res);

    if (isError(error)) {
        LOG(FILESYSTEM, ERROR, "RamdiskDirectory::remove(): Error removing resource %s", res->getName());
        return (error);
    }

    theOS->getMemoryManager()->scheduleDeletion(res);

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
File* RamFilesystemDirectory::createFile(char* name, unint4 flags) {
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

    RamFilesystemFile* f = new RamFilesystemFile(this->myRamDisk, block, name, flags);
    this->add(f);
    return (f);
}


/*****************************************************************************
 * Method: RamdiskDirectory::createDirectory(char* name, unint4 flags)
 *
 * @description
 *   Creates a new Ramdisk Directory inside the given Ramdisk directory.
 *
 * @returns
 *  Directory*  Pointer to the directory or null if an error occured.
 *******************************************************************************/
Directory* RamFilesystemDirectory::createDirectory(char* name, unint4 flags) {
    if (get(name, strlen(name))) {
       LOG(FILESYSTEM, ERROR, "RamdiskDirectory::createDirectory(): Directory already exists '%s'", name);
       return (0);
    }

    RamFilesystemDirectory* f = new RamFilesystemDirectory(this->myRamDisk, name);
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
ErrorT RamFilesystemFile::readBytes(char* bytes, unint4& length) {
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
        sector_pos = this->readPos & (RAMDISK_BLOCK_SIZE - 1);

        // set readlength to remaining bytes in this sector
        sector_read_len = RAMDISK_BLOCK_SIZE - sector_pos;

        // check if we read less than the remaining bytes in this sector
        if (readlength < sector_read_len) {
            sector_read_len = readlength;
        } else {
            sector_changed = true;
        }

        buffer = reinterpret_cast<char*>((currentBlock * RAMDISK_BLOCK_SIZE) + myRamDisk->getFirstBlockAddress());
        // copy the desired bytes in this sector into the buffer
        memcpy(&bytes[pos], &buffer[sector_pos], sector_read_len);

        // increase position by bytes read
        this->readPos += sector_read_len;
        pos += sector_read_len;

        // check if we reached the sector boundary
        if (sector_changed) {
            unint4 nextBlock = myRamDisk->getNextBlock(currentBlock, false);
            if (nextBlock == (unint4) -1) {
                // end of file reached
                length = pos;
                return (cOk );
            }
            currentBlock = nextBlock;
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
ErrorT RamFilesystemFile::writeBytes(const char* bytes, unint4 length) {
    unint4 sector_pos;
    unint4 sector_write_len;
    bool sector_changed = false;
    unint4 pos = 0;  // position inside the bytes array

    char* buffer;

    // keep writing while we have bytes to write
    while (length > 0) {
        // check if the next write operation is overwriting something or appending
        sector_pos = this->position & (RAMDISK_BLOCK_SIZE - 1);  // position inside the sector
        sector_write_len = RAMDISK_BLOCK_SIZE - sector_pos;      // the length we are writing inside this sector

        // check if we write less than the remaining bytes in this sector
        if (length < sector_write_len) {
            sector_write_len = length;
        } else {
            sector_changed = true;
        }

        buffer = reinterpret_cast<char*>((currentBlock * RAMDISK_BLOCK_SIZE) + myRamDisk->getFirstBlockAddress());

        // copy the desired bytes in this sector into the buffer
        memcpy(&buffer[sector_pos], &bytes[pos], sector_write_len);

        // increase position by bytes read
        this->position += sector_write_len;
        pos += sector_write_len;

        // check if we reached the sector boundary
        if (sector_changed) {
            // append if neccessary
            unint4 nextBlock = myRamDisk->getNextBlock(currentBlock, true);
            if (nextBlock == (unint4) -1) {
                // no more memory..
                return (cDeviceMemoryExhausted);
            }
            currentBlock = nextBlock;
        }

        sector_changed = false;
        // decrease total amount to read by read amount of bytes
        length -= sector_write_len;
    }

    if (this->position > this->filesize) {
        this->filesize = this->position;
    }

    return (cOk);
}

RamFilesystemFile::RamFilesystemFile(RamFilesystem* myRamdisk, unint4 blockNum, char* name, int flags) :
        File(name, 0, flags) {
    this->myRamDisk     = myRamdisk;
    this->myBlockNumber = blockNum;
    currentBlock        = myBlockNumber;
    readPos = 0;
}


RamFilesystemFile::~RamFilesystemFile() {
    /* we are deleted.. free the blocks this file occupied */
    myRamDisk->freeBlock(this->myBlockNumber);
}

/*****************************************************************************
 * Method: RamdiskFile::resetPosition()
 *
 * @description
 *   Resets the file position to its beginning.
 *******************************************************************************/
ErrorT RamFilesystemFile::resetPosition() {
    //File::resetPosition();
    readPos        = 0;
    this->position = 0;
    currentBlock = myBlockNumber;
    return (cOk);
}
