/*
 * PartitionManager.hh
 *
 *  Created on: 19.06.2013
 *      Author: dbaldin
 */

#ifndef PARTITIONMANAGER_HH_
#define PARTITIONMANAGER_HH_
#include "PartitionManager.hh"
#include "hal/BlockDeviceDriver.hh"
#include "filesystem/Partition.hh"
#include "filesystem/Directory.hh"



class PartitionManager : Directory {


public:
	PartitionManager();

	void 	registerBlockDevice(BlockDeviceDriver *bdev);

	void 	unregisterBlockDevice(BlockDeviceDriver *bdev);

	ErrorT handleEFIPartitionTable(BlockDeviceDriver* bdev);

	/*!
	 * Tries to read a DOS MBR from the block device.
	 *
	 * returns: cOk on success
	 */
	ErrorT 	tryDOSMBR(BlockDeviceDriver *bdev);

	virtual ~PartitionManager();
};

#endif /* PARTITIONMANAGER_HH_ */
