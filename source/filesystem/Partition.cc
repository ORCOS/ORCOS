/*
 * Partition.cc
 *
 *  Created on: 18.02.2014
 *     Copyright & Author: dbaldin
 */

#include "Partition.hh"
#include "inc/memtools.hh"
#include "kernel/Kernel.hh"
#include "filesystem/FileSystemBase.hh"

extern Kernel* theOS;

/* Buffer for read operation to the partition. 1024 bytes for maximum sector size. */
static char buffer[1024];

/* This structure defines the global partition cache */
typedef struct {
    char        buffer[BLOCK_SIZE];
    int         block_start;
    int         modified;
    Partition*  pPartition;
} tPartitionBlockCache;

tPartitionBlockCache CachedBlocks[5] __attribute__((aligned(4)));
/* List of Pointers to hte tPartitionBlockCache Elements. Sorted in reverse order of last access */
LinkedList*          BlockCacheList;

Partition::~Partition() {
    if (this->mountedFileSystem != 0) {
       delete this->mountedFileSystem;

       LinkedListItem* litem = BlockCacheList->getHead();
       tPartitionBlockCache* blockCache = 0;
       while (litem) {
          blockCache = reinterpret_cast<tPartitionBlockCache*>(litem->getData());
          if (blockCache->pPartition == this) {
              blockCache->pPartition  = 0;
              blockCache->block_start = -1;
              blockCache->modified    = 0;
              break;
          }
          litem = litem->getSucc();
      }
    }
}

/*****************************************************************************
* Method: Partition::invalidate()
*
* @description
*   Invalidation of the partition. Done when scheduled for deletion.
*   Also invalidates the filesystem mounted contained inside this partition
*   if any.
*******************************************************************************/
void Partition::invalidate() {
    if (this->mountedFileSystem) {
        this->mountedFileSystem->invalidate();
    }
}

/*****************************************************************************
* Method: Partition::initialize()
*
* @description
*
*******************************************************************************/
void Partition::initialize() {
    BlockCacheList = new LinkedList();
    for (int i = 0; i < 5; i++) {
        CachedBlocks[i].block_start  = -1;
        CachedBlocks[i].pPartition   = 0;
        CachedBlocks[i].modified     = 0;
        BlockCacheList->addTail(reinterpret_cast<ListItem*>(&CachedBlocks[i]));
    }
}


ErrorT Partition::accessSector(unint4 sector_num, char* buffer, int operation) {
    /* try to find sector in cached block list */
    LinkedListItem* litem = BlockCacheList->getHead();
    tPartitionBlockCache* blockCache = 0;
    int block_num = sector_num & ~(sectorsPerBlock-1);

    while (litem) {
        blockCache = reinterpret_cast<tPartitionBlockCache*>(litem->getData());
        if (blockCache->pPartition == this && blockCache->block_start == block_num) {
            break;
        }
        litem = litem->getSucc();
    }

    if (!litem) {
        /* not cached .. drop least recently used cached block and read new block */
        litem = BlockCacheList->getTail();
        blockCache = reinterpret_cast<tPartitionBlockCache*>(litem->getData());

        /* write back block */
        if (blockCache->pPartition != 0 && blockCache->modified) {
            ErrorT error = blockCache->pPartition->myBlockDevice->writeBlock(blockCache->pPartition->lba_start + blockCache->block_start, blockCache->buffer, sectorsPerBlock);
            if (isError(error)) {
               return (error);
            }
        }

        /* now cache new block */
        blockCache->pPartition  = this;
        blockCache->block_start = block_num;
        blockCache->modified    = 0;
        ErrorT error = myBlockDevice->readBlock(this->lba_start + blockCache->block_start, blockCache->buffer, sectorsPerBlock);
        if (isError(error)) {
            return (error);
        }
    }

    /* found in cache */
    int offset = sector_num & (sectorsPerBlock-1);
    if (operation == SECTOR_READ) {
        memcpyl(buffer, &blockCache->buffer[getSectorSize()*offset], getSectorSize());
    } else {
        blockCache->modified = 1;
        memcpyl(&blockCache->buffer[getSectorSize()*offset], buffer, getSectorSize());
    }
    /* put block at head */
    litem->remove();
    BlockCacheList->addHead(litem);
    return (cOk);
}

void Partition::flushCache() {
    LinkedListItem* litem = BlockCacheList->getHead();

     while (litem) {
         tPartitionBlockCache* blockCache;
         blockCache = reinterpret_cast<tPartitionBlockCache*>(litem->getData());
         if (blockCache->pPartition == this && blockCache->modified) {
             blockCache->modified = 0;
             /* write back this block */
             blockCache->pPartition->myBlockDevice->writeBlock(blockCache->pPartition->lba_start + blockCache->block_start, blockCache->buffer, sectorsPerBlock);
         }
         litem = litem->getSucc();
     }
}


/*****************************************************************************
 * Method: Partition::readSectors(unint4 sector_start, char* buffer, unint4 num_sectors)
 *
 * @description
 *  Tries to read the given sector of this partition into the buffer.
 *******************************************************************************/
ErrorT Partition::readSectors(unint4 sector_start, char* buffer, unint4 num_sectors) {
    if (sector_start + num_sectors < sectors) {
        ErrorT ret = cOk;
        for (unint4 i = 0; i < num_sectors; i++) {
            ret = accessSector(sector_start + i, &buffer[i*getSectorSize()], SECTOR_READ);
            if (isError(ret)) return (ret);
        }
        return (ret);
    }
    else {
        return (cError);
    }
}


/*****************************************************************************
 * Method: Partition::clearBytes(unint4 sector_start, unint4 offset, unint4 length)
 *
 * @description
 *
 *******************************************************************************/
ErrorT Partition::clearBytes(unint4 sector_start, unint4 offset, unint4 length) {
    unint4 sector = sector_start;

    /* check for supported sector sizes*/
    if (getSectorSize() > 1024) {
        return (cError);
    }

    if (offset > getSectorSize()) {
        /* shift to new sector and update offset */
        sector += getSectorSize() / offset;
        offset = offset % getSectorSize();
    }

    /* check for valid sector access*/
    if (sector < sectors) {
        int bytesCleared    = 0;

        /* clear all bytes until length is == 0 or sector reached end of partition */
        while (length > 0 && sector < sectors) {
            unint4 len  = length;

            /* shrink len to fit current sector */
            if (offset + length > getSectorSize()) {
                len = getSectorSize() - offset;
            }
            if (offset > 0 || len < getSectorSize()) {
                myBlockDevice->readBlock(this->lba_start + sector, buffer, 1);
            }
            memset(buffer + offset, 0, len);
            myBlockDevice->writeBlock(this->lba_start + sector, buffer, 1);

            length       -= len;
            bytesCleared += len;
            sector++;
            offset        = 0;
        }

        return (bytesCleared);

    } else return (cError);
}

/*****************************************************************************
 * Method: Partition::writeSectors(unint4 sector_start, char* buffer, unint4 num_sectors)
 *
 * @description
 *
 *******************************************************************************/
ErrorT Partition::writeSectors(unint4 sector_start, char* buffer, unint4 num_sectors) {
    if (sector_start + num_sectors < sectors) {
         ErrorT ret = cOk;
         for (unint4 i = 0; i < num_sectors; i++) {
             ret = accessSector(sector_start + i, &buffer[i*getSectorSize()], SECTOR_WRITE);
             if (isError(ret)) return (ret);
         }
         return (ret);
    } else {
         return (cError);
    }
}
