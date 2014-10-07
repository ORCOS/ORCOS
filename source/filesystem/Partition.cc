/*
 * Partition.cc
 *
 *  Created on: 18.02.2014
 *      Author: dbaldin
 */

#include "Partition.hh"
#include "inc/memtools.hh"

/*Enable disable global parition write-through cache.*/
#define PARTITION_CACHE_ENABLE 0

#if PARTITION_CACHE_ENABLE
static char cache[1024];
static unint4 cached_sector;
static Partition* cachedPartition;
#endif

/*!
 * Tries to read the given sector of this partition into the buffer.
 */
ErrorT Partition::readSectors(unint4 sector_start, unint1* buffer, unint4 num_sectors) {
    if (sector_start + num_sectors < sectors)
    {

#if PARTITION_CACHE_ENABLE
        if (num_sectors * this->getSectorSize() <= 1024 && cachedPartition == this && cached_sector == this->lba_start + sector_start)
        {
            memcpy(buffer,cache,num_sectors * this->getSectorSize());
            return (cOk);
        }
#endif

        return (myBlockDevice->readBlock(this->lba_start + sector_start, buffer, num_sectors));
    }
    else
        return (cError );
}

static unint1 buffer[1024];

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
    if (sector < sectors)
    {
        int bytesCleared    = 0;
        int offset          = offset;

        /* clear all bytes until length is == 0 or sector reached end of partition */
        while (length > 0 && sector < sectors) {
            int len  = length;

            /* shrink len to fit current sector */
            if (offset + length > getSectorSize()) {
                len = getSectorSize() - offset;
            }

            myBlockDevice->readBlock(this->lba_start + sector, buffer, 1);
            memset(buffer + offset,0,len);
            myBlockDevice->writeBlock(this->lba_start + sector, buffer, 1);

            length -= len;
            bytesCleared += len;
            sector++;
            offset = 0;
        }

        return (bytesCleared);

    } else return (cError);


}

ErrorT Partition::writeSectors(unint4 sector_start, unint1* buffer, unint4 num_sectors) {
    if (sector_start + num_sectors < sectors)
    {

#if PARTITION_CACHE_ENABLE
        if (num_sectors * this->getSectorSize() <= 1024)
        {
            cachedPartition = this;
            cached_sector = this->lba_start + sector_start;
            memcpy(cache,buffer,num_sectors * this->getSectorSize());
        }
#endif

        return (myBlockDevice->writeBlock(this->lba_start + sector_start, buffer, num_sectors));
    }
    else
        return (cError );
}
