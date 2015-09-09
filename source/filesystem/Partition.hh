/*
 * Partition.hh
 *
 *  Created on: 19.06.2013
 *      Copyright & Author: dbaldin
 */

#ifndef PARTITION_HH_
#define PARTITION_HH_

#include "filesystem/Resource.hh"
#include "hal/BlockDeviceDriver.hh"
#include "filesystem/FileSystemBase.hh"

class FileSystemBase;

#define SECTOR_READ  0
#define SECTOR_WRITE 1

/* The block size can be tuned for better performance */
#define BLOCK_SIZE 4096*2

/*
 * Base Class of all partition handler.
 *
 */
class Partition: public Resource {
    friend class PartitionManager;

protected:
    BlockDeviceDriver* myBlockDevice;

    // Logical Block Start address
    unint4 lba_start;

    // The number of Sectors this partition contains
    unint4 sectors;

    // The partition number
    unint4 partition_number;

    /* we support only one Filesystem per partition (are there any other cases anyway?) */
    FileSystemBase* mountedFileSystem;

    /* The number of sectors per block */
    unint4 sectorsPerBlock;

    /*****************************************************************************
     * Method: accessSector(unint4 sector_num, char* buffer, int operation)
     *
     * @description
     *  Performs SECTOR_READ or SECTOR_WRITE operation on a given sector using the data inside the buffer.
     *  On SECTOR_WRITE write the data inside the buffer into the sector.
     *  On SECTOR_READ reads the sector into the buffer.
     *******************************************************************************/
    ErrorT accessSector(unint4 sector_num, char* buffer, int operation);

public:
    /*****************************************************************************
     * Method: initialize()
     *
     * @description
     *  Static initialization of the Partition Buffer Cache.
     *******************************************************************************/
    static void initialize();

    /*****************************************************************************
     * Method: flushCache()
     *
     * @description
     *  Flushes all buffered sectors to the block devices.
     *******************************************************************************/
    void flushCache();

    Partition(BlockDeviceDriver *p_bdev, char* p_name, unint4 i_lba_start, unint4 i_sectors, unint4 i_partition_num) :
            Resource(cPartition, true, p_name) {
        this->myBlockDevice     = p_bdev;
        this->lba_start         = i_lba_start;
        this->sectors           = i_sectors;
        this->partition_number  = i_partition_num;
        this->mountedFileSystem = 0;
        this->sectorsPerBlock   = BLOCK_SIZE / this->myBlockDevice->sector_size;
    }


    Partition(BlockDeviceDriver *p_bdev, char* p_name) :
            Resource(cPartition, true, p_name) {
        this->myBlockDevice     = p_bdev;
        this->lba_start         = 0;
        this->sectors           = 0;
        this->partition_number  = 0;
        this->mountedFileSystem = 0;
        this->sectorsPerBlock   = BLOCK_SIZE / this->myBlockDevice->sector_size;
    }


    virtual ~Partition();

    /*****************************************************************************
    * Method: invalidate()
    *
    * @description
    *   Invalidation of the partition. Done when scheduled for deletion.
    *   Also invalidates the filesystem mounted contained inside this partition
    *   if any.
    *******************************************************************************/
    void invalidate();

    /*****************************************************************************
     * Method: setMountedFileSystem(FileSystemBase* fs)
     *
     * @description
     * On successfull mount the filesystem needs to call this method
     * to register the mounted filesystem at its partition.
     *******************************************************************************/
    void setMountedFileSystem(FileSystemBase* fs) {
        this->mountedFileSystem = fs;
    }

    /*****************************************************************************
     * Method: readSectors(unint4 sector_start, char* buffer, unint4 num_sectors)
     *
     * @description
     *  Tries to read the given sector of this partition into the buffer.
     *******************************************************************************/
    ErrorT readSectors(unint4 sector_start, char* buffer, unint4 num_sectors);

    /*****************************************************************************
     * Method: writeSectors(unint4 sector_start, char* buffer, unint4 num_sectors)
     *
     * @description
     *  Tries to write the given sector of this partition into the buffer
     *******************************************************************************/
    ErrorT writeSectors(unint4 sector_start, char* buffer, unint4 num_sectors);

    /*****************************************************************************
     * Method: clearBytes(unint4 sector_start, unint4 offset, unint4 length)
     *
     * @description
     *   Clears 'length' bytes starting in 'sector_start' at byte offset 'offset'
     *   Bytes are overwritten with 0.
     *******************************************************************************/
    ErrorT clearBytes(unint4 sector_start, unint4 offset, unint4 length);

    /*****************************************************************************
     * Method: getStartLBA()
     *
     * @description
     *   Returns the Starting Logical Block Address of this partition.
     *******************************************************************************/
    unint4 getStartLBA() {
        return (lba_start);
    }


    /*****************************************************************************
     * Method: getNumSectors()
     *
     * @description
     *   Returns the number of sectors this partition contains.
     *******************************************************************************/
    unint4 getNumSectors() {
        return (sectors);
    }

    /*****************************************************************************
     * Method: getSectorSize()
     *
     * @description
     *   Returns the bytes per sector value.
     *******************************************************************************/
    unint4 getSectorSize() {
        return (this->myBlockDevice->sector_size);
    }

    /*****************************************************************************
     * Method: getPartitionNumber()
     *
     * @description
     *  Returns the partition number.
     *******************************************************************************/
    unint4 getPartitionNumber() {
        return (partition_number);
    }
};

#endif /* PARTITION_HH_ */
