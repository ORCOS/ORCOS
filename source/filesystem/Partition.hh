/*
 * Partition.hh
 *
 *  Created on: 19.06.2013
 *      Author: dbaldin
 */

#ifndef PARTITION_HH_
#define PARTITION_HH_

#include "filesystem/Resource.hh"
#include "hal/BlockDeviceDriver.hh"
#include "filesystem/FileSystemBase.hh"

class FileSystemBase;

/*
 * Base Class of all partition handler.
 *
 */
class Partition : public Resource {

	friend class PartitionManager;

protected:
	BlockDeviceDriver* myBlockDevice;

	// Logical Block Start address
	unint4 lba_start;

	// The number of Sectors this partition contains
	unint4 sectors;

	// The partition number
	unint4 partition_number;

	// we support only one Filesystem per partition (are there any other cases anyway?)
	FileSystemBase* mountedFileSystem;

public:

	Partition(BlockDeviceDriver *bdev, char* name, unint4 lba_start, unint4 sectors, unint4 partition_number) : Resource(cPartition,true,name) {
			this->myBlockDevice 	= bdev;
			this->lba_start 		= lba_start;
			this->sectors 			= sectors;
			this->partition_number 	= partition_number;
			this->mountedFileSystem = 0;
		};

	Partition(BlockDeviceDriver *bdev, char* name) : Resource(cPartition,true,name) {
		this->myBlockDevice 	= bdev;
		this->lba_start 		= 0;
		this->sectors 			= 0;
		this->partition_number 	= 0;
		this->mountedFileSystem = 0;
	};

	virtual ~Partition() {};

	/*!
	 * On successfull mount the filesystem needs to call this method
	 * to register the mounted filesystem at its partition.
	 */
	void setMountedFileSystem(FileSystemBase* fs) {
		this->mountedFileSystem = fs;
	}

	/*!
	 * Tries to read the given sector of this partition into the buffer.
	 */
	ErrorT readSectors(unint4 sector_start, char* buffer, unint4 num_sectors) {
		if (sector_start + num_sectors < sectors)
			return myBlockDevice->readBlock(this->lba_start + sector_start,buffer, num_sectors);
		else return cError;
	}

	ErrorT writeSectors(unint4 sector_start, char* buffer, unint4 num_sectors) {
		if (sector_start + num_sectors < sectors)
			return myBlockDevice->writeBlock(this->lba_start + sector_start,buffer, num_sectors);
		else return cError;
	}

	/*!
	 * Returns the Starting Logical Block Address of this partition.
	 */
	unint4 getStartLBA() 		{ return lba_start; }

	/*!
	 * Returns the number of sectors this partition contains.
	 */
	unint4 getNumSectors() 		{ return sectors; }

	/*!
	 * Returns the bytes per sector value.
	 */
	unint4 getSectorSize() 		{ return this->myBlockDevice->sector_size; }

	/*!
	 * Returns the partition number.
	 */
	unint4 getPartitionNumber() { return partition_number; }

};

#endif /* PARTITION_HH_ */