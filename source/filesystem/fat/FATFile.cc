/*
 * FATFile.cc
 *
 *  Created on: 01.11.2015
 *      Author: Daniel
 */


#include "FATFileSystem.hh"
#include "inc/memtools.hh"
#include "inc/stringtools.hh"
#include "kernel/Kernel.hh"

extern Kernel* theOS;

extern Mutex* FATAccess;

static char buffer[512];


FATFile::FATFile(char*              p_name,
                 FATDirectory*      parent,
                 FAT32_DirEntry*    p_entry,
                 FATFileSystem *    p_fs,
                 unint4             directory_sector,
                 unint2             directory_entry_number,
                 unint4             longname_entry_sector,
                 int2               longname_entry_number) :
        File(p_name, p_entry->DIR_FileSize, p_entry->DIR_Attr) {
    this->clusterStart              = ((((unint4)p_entry->DIR_FstClusHI) << 16) | p_entry->DIR_FstClusLO);
    this->currentCluster            = clusterStart;
    this->currentSector             = p_fs->ClusterToSector(clusterStart);
    this->myFS                      = p_fs;
    this->directory_sector          = directory_sector;
    this->directory_entry_number    = directory_entry_number;
    this->longname_entry_sector     = longname_entry_sector;
    this->longname_entry_number     = longname_entry_number;
    this->parent                    = parent;
    LOG(FILESYSTEM, TRACE, "FATFile() cluster: %u, sector: %u", clusterStart, currentSector);
}

FATFile::~FATFile() {
    // check if anybody is holding a handle to this..
}

/*****************************************************************************
 * Method: FATFile::readBytes(char* bytes, unint4& length)
 *
 * @description
 *   Tries to read the given number of bytes from the FAT File.
 *   length is updated to the actual number of bytes read.
 *******************************************************************************/
ErrorT FATFile::readBytes(char* bytes, unint4& length) {
    LOG(FILESYSTEM, DEBUG, "FATFile::readBytes length: %d", length);
    unint4 sector_pos;
    unint4 sector_read_len;
    unint4 pos = 0;

    unint4 readlength = length;
    /* be sure we are not reading over the end of the file */
    if ((this->filesize - this->position) < readlength)
        readlength = (this->filesize - this->position);

    if (readlength == 0) {
        return (cEOF);
    }

    unint4 sector_size = this->myFS->myPartition->getSectorSize();
    bool sector_changed = false;

    FATAccess->acquire();

    while (readlength > 0) {
        // position inside our current sector
        //sector_pos         = this->position % sector_size;
        /* sector size must be a multiple of 2 */
        sector_pos = this->position & (sector_size - 1);

        /* set read length to remaining bytes in this sector */
        sector_read_len = sector_size - sector_pos;

        /* check if we read less than the remaining bytes in this sector */
        if (readlength < sector_read_len) {
            sector_read_len = readlength;
        } else {
            sector_changed = true;
        }

        LOG(FILESYSTEM, DEBUG, "FATFile::readBytes reading Sector: %u, cluster: %u", currentSector, currentCluster);
        /* read the sector */
        int error = myFS->myPartition->readSectors(currentSector, buffer, 1);
        if (error < 0) {
            FATAccess->release();
            return (error);
        }

        /* copy the desired bytes in this sector into the buffer */
        memcpy(&bytes[pos], &buffer[sector_pos], sector_read_len);
        /* increase position by bytes read */
        this->position += sector_read_len;
        pos += sector_read_len;

        /* check if we reached the sector boundary */
        if (sector_changed) {
            currentSector  = myFS->getNextSector(currentSector, currentCluster);
            if (currentSector == EOC) {
                /* end of file reached */
                length = pos;
                FATAccess->release();
                LOG(FILESYSTEM, WARN, "FATFile::readBytes EOF: %u, cluster: %u, filepos: %u, filesize %u", currentSector, currentCluster, position, filesize);
                return (cEOF );
            }
            currentCluster = this->myFS->SectorToCluster(currentSector);
        }

        sector_changed = false;
        /* decrease total amount to read by read amount of bytes */
        readlength -= sector_read_len;
    }

    FATAccess->release();
    LOG(FILESYSTEM, DEBUG, "FATFile::readBytes Bytes read: %d", length);

    length = pos;
    return (cOk );
}

/*****************************************************************************
 * Method: FATFile::writeBytes(const char* bytes, unint4 length)
 *
 * @description
 *   Writes the given number of bytes to the fat file. Directly
 *   writs the data to the partition. Data is directly
 *   transferred to the device.
 *******************************************************************************/
ErrorT FATFile::writeBytes(const char* bytes, unint4 length) {
    LOG(FILESYSTEM, DEBUG, "FATFile::writeBytes length: %d", length);

    unint4 sector_pos;
    unint4 sector_write_len;
    unint4 sector_size  = this->myFS->myPartition->getSectorSize();
    bool sector_changed = false;
    unint4 pos          = 0;  /* position inside the bytes array */
    bool new_sector     = false;
    int error;

    FATAccess->acquire();
    /* keep writing while we have bytes to write */

    while (length > 0) {
        /* check if the next write operation is overwriting something or appending */
        sector_pos          = this->position & (sector_size - 1);   /* position inside the sector */
        sector_write_len    = sector_size - sector_pos;             /* the length we are writing inside this sector */

        /* check if we read less than the remaining bytes in this sector */
        if (length < sector_write_len) {
            sector_write_len = length;
        } else {
            sector_changed = true;
        }
        LOG(FILESYSTEM, DEBUG, "FATFile::writeBytes sector: %u, cluster: %u, length: %u", currentSector, currentCluster, sector_write_len);

        /* read the sector if we are overwriting something */
        if (!new_sector) {
            error = myFS->myPartition->readSectors(currentSector, buffer, 1);
            if (error < 0) {
                LOG(FILESYSTEM, ERROR, "FATFile::writeBytes readSectors failed: sector: %u, cluster: %u, write_length: %u", currentSector, currentCluster, sector_write_len);
                FATAccess->release();
                return (error);
            }
        }

        /* copy the desired bytes in this sector into the buffer */
        memcpy(&buffer[sector_pos], &bytes[pos], sector_write_len);

        error = myFS->myPartition->writeSectors(currentSector, buffer, 1);
        if (error < 0) {
            LOG(FILESYSTEM, ERROR, "FATFile::writeBytes writeSectors failed: sector: %u, cluster: %u, write_length: %u", currentSector, currentCluster, sector_write_len);
            FATAccess->release();
            return (error);
        }

        /* increase position by bytes read */
        this->position += sector_write_len;
        pos += sector_write_len;

        /* check if we reached the sector boundary */
        if (sector_changed) {
            /* append if neccessary */
            currentSector  = myFS->getNextSector(currentSector, currentCluster, true);
            if (currentSector == EOC) {
                LOG(FILESYSTEM, ERROR, "FATFile::writeBytes getNext sector failed: sector: %u, cluster: %u, write_length: %u", currentSector, currentCluster, sector_write_len);
                /* no more memory.. */
                FATAccess->release();
                return (cDeviceMemoryExhausted );
            }
            currentCluster = this->myFS->SectorToCluster(currentSector);
            /* if we reach a new sector and the position > filesize
             * we can omit reading the contents of that sector */
            new_sector = position > filesize;
        }

        sector_changed = false;
        /* decrease total amount to read by read amount of bytes */
        length -= sector_write_len;
    }

    /* directly update the filesize! */
    if (this->position > this->filesize) {
        /* we appended something... update the entries filesize */
        error = myFS->myPartition->readSectors(this->directory_sector, buffer, 1);
        if (error < 0) {
            LOG(FILESYSTEM, ERROR, "FATFile::writeBytes Error reading directory entry: sector: %u", this->directory_sector);
            FATAccess->release();
            return (error);
        }

        this->filesize = this->position;

        FAT32_DirEntry* entries = reinterpret_cast<FAT32_DirEntry*>(buffer);
        entries[this->directory_entry_number].DIR_FileSize = this->filesize;

        error = myFS->myPartition->writeSectors(this->directory_sector, buffer, 1);
        if (error < 0) {
            LOG(FILESYSTEM, ERROR, "FATFile::writeBytes Error writing directory entry: sector: %u", this->directory_sector);
            FATAccess->release();
            return (error);
        }
    }

    FATAccess->release();
    return (cOk );
}

/*****************************************************************************
 * Method: FATFile::resetPosition()
 *
 * @description
 *   Resets the file position to the beginning of the FAT File.
 *******************************************************************************/
ErrorT FATFile::resetPosition() {
    this->currentCluster = clusterStart;
    this->currentSector = myFS->ClusterToSector(clusterStart);
    this->position = 0;
    return (cOk);
}


/*****************************************************************************
 * Method: FATFile::rename(char* newName)
 *
 * @description
 *
 *******************************************************************************/
ErrorT FATFile::rename(char* newName) {
    unint4 fileEntrySector = -1;
    unint2 fileSectorEntry = -1;

    unint4 longNameEntrySector = -1;
    int2   longNameSectorEntry = -1;
    FAT32_DirEntry* fsrootdir_entry;

    LOG(FILESYSTEM, WARN, "FATFile::rename() Removing old entries!");

    /* remove old name entries inside the directory table */
    parent->removeFileEntries(this, false);

    LOG(FILESYSTEM, WARN, "FATFile::rename() allocating new entries!");

    /* allocate new directory entries for the new name */
    ErrorT ret = parent->allocateEntry(newName, ATTR_ARCHIVE, fsrootdir_entry, fileEntrySector, fileSectorEntry, longNameEntrySector, longNameSectorEntry, clusterStart);
    if (isError(ret)) {
        LOG(FILESYSTEM, ERROR, "FATFile::rename() Error allocating new entries!");
        return (ret);
    }

    /* store reference to the new directory entries */
    this->directory_sector          = fileEntrySector;
    this->directory_entry_number    = fileSectorEntry;
    this->longname_entry_sector     = longNameEntrySector;
    this->longname_entry_number     = longNameSectorEntry;

    /* be sure data is written to device */
    this->myFS->myPartition->flushCache();

    /* finally call base method */
    File::rename(newName);
    return (cOk);
}

char fill_buffer[512];

/*****************************************************************************
 * Method: FATFile::seek(int4 seek_value)
 *
 * @description
 *
 *******************************************************************************/
ErrorT FATFile::seek(int4 seek_value) {
    if (seek_value == 0) {
        return (cOk);
    }

    FATAccess->acquire();

    unint4 new_position = this->position + seek_value;
    if (seek_value < 0 && new_position > this->position) {
        /* underflow */
        new_position = 0;
    }
    if (seek_value > 0 && new_position < this->position) {
        /* overflow */
        new_position = -1;
    }
    unint4 sector_size  = this->myFS->myPartition->getSectorSize();

    if (new_position < this->position) {
        /* start iterating cluster chain from start until new_position is reached..*/
        this->position          = 0;
        this->currentCluster    = this->clusterStart;
        this->currentSector     = this->myFS->ClusterToSector(this->clusterStart);
        seek_value              = new_position;
    } else {
        if (new_position > this->filesize) {
            /* first seek to the end */
            this->seek(this->filesize - this->position);
            /* now fill up remaining bytes with zeros */
            unint4 fill_length = new_position - this->filesize;
            memset(fill_buffer, 0, 512);

            while (fill_length > 0) {
                if (fill_length > 512) {
                    this->writeBytes(fill_buffer, 512);
                    fill_length -= 512;
                } else {
                    this->writeBytes(fill_buffer, fill_length);
                    fill_length = 0;
                }
            }

            return (cOk);
        }
    }
    /* seek_value is now always positive */

    bool sector_changed = false;
    /* start from current position and seek to position < filesize */
    while (seek_value > 0) {
        unint4 sector_pos       = this->position & (sector_size - 1);  // position inside the sector
        unint4 sector_seek_len  = sector_size - sector_pos;       // the length we will seek inside this sector

        if ((unint4)seek_value < sector_seek_len) {
            sector_seek_len = seek_value;
        } else {
            sector_changed = true;
        }

        seek_value -= sector_seek_len;
        this->position += sector_seek_len;

        /* check if we reached the sector boundary */
        if (sector_changed) {
          currentSector = myFS->getNextSector(currentSector, currentCluster, false);
          if (currentSector == EOC) {
              /* no more memory .. ? bug inside chain as we are still < filesize! */
              FATAccess->release();
              return (cError);
          }
          currentCluster = myFS->SectorToCluster(currentSector);
        }

        sector_changed = false;
    }

    FATAccess->release();
    return (cOk);
}
