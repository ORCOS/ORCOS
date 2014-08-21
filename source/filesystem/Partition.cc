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
